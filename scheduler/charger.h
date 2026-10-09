#pragma once
// Charges the Tesla in the cheapest quarter-hours before Ready by.
//
// Every 30 s SchedulerComponent (scheduler_component.h) passes the car's state to Controller::tick(), which reschedules
// when needed (schedule.h), counts what charging cost and saved (savings.h), and returns what to do.
//
// Plain C++17 with nothing from ESPHome, like the headers it includes, so it can be unit-tested on a computer.

#include "calendar.h"
#include "market.h"
#include "savings.h"
#include "schedule.h"
#include "tariff.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>

namespace esphome::scheduler {

enum class Command { NONE, START_CHARGING, STOP_CHARGING, WAKE };

struct CarState {
  int64_t now = 0;              // UTC seconds; 0 until the clock is synced
  std::optional<bool> plugged;  // unknown until the car reports it (after a restart while it sleeps)
  std::string charging_state;   // "Charging", "Starting", "Stopped", "Complete", ...; empty or "Unknown" when unknown
  float soc = NAN;              // battery %
  float limit = NAN;            // the car's charge limit %
  float power_kw = NAN;         // what the car draws from the charger
  bool port_open = false;       // the charge port flap; the car reports it even while asleep
  bool paired = true;           // the car has reported to the board's key since the settings named it

  bool charging_known() const { return !charging_state.empty() && charging_state != "Unknown"; }
};

// A one-off phone message, which SchedulerComponent sends through ntfy.
struct Notification {
  std::string title = "ESPHome Tesla BLE Scheduler";
  std::string message;
};

struct Decision {
  Command command = Command::NONE;
  std::string status;
  // "schedule", "now" (charging regardless of price), "none" (no schedule), "unplugged" (the board's plug state, which
  // ignores the Charger reading while the charging state is Unknown) or "wait" (no schedule possible yet: not paired,
  // or no clock, plug state, battery level or prices).
  std::string mode;
  std::string windows;                       // format_windows() while the schedule decides, else empty
  std::optional<Notification> notification;  // once per plug-in, when its schedule has settled, once more
                                             // when the board falls back to charging at any price or a
                                             // start from the car takes over, and when Ready by passes
                                             // with the car short of its limit
  std::string savings;                       // format_savings() once the key is paired and the clock set, else empty
  bool save_savings = false;                 // Controller::savings changed: time to write it to flash
  bool unlock_port = false;                  // the car just finished charging, and the settings ask to unlock its port
};

class Controller {
 public:
  PriceTable prices;
  // What charging cost and saved, for the board to keep in flash (see Decision::save_savings), as counter_ counts it.
  Savings savings;

  // The buttons on the web page. Each acts at once, free of command_()'s limits.

  // Follow the schedule: cancels charge_now(), a start from the car or app, or stop_charging().
  void create_schedule() { press_(Hold::SCHEDULE); }
  // Charge regardless of price until the car is unplugged.
  void charge_now() { press_(Hold::NOW); }
  // No schedule and no charging until create_schedule(), charge_now(), a start from the car or app, or the car is
  // unplugged.
  void stop_charging() { press_(Hold::NONE); }
  // Settings saved with other values (deletes_schedule() in settings.h): a schedule goes, as with stop_charging(),
  // while charging now stays.
  void delete_schedule() {
    if (hold_ == Hold::SCHEDULE)
      press_(Hold::NONE);
  }
  // The page's Reset savings: the figures start afresh on the next tick.
  void reset_savings() { savings = Savings{}; }
  // What the buttons chose, for the board to keep across a restart (see Hold).
  int held_mode() const { return static_cast<int>(hold_); }
  void restore_mode(int mode) { hold_ = mode == 1 ? Hold::NOW : mode == 2 ? Hold::NONE : Hold::SCHEDULE; }
  // Recompute the schedule on the next tick (new prices or Ready by).
  void reschedule() { reschedule_ = true; }
  // Whether to download prices now, which counts as a try: every 5 minutes while some of today's prices are missing,
  // or from 12:45 CET, when the next day's prices start to come out, until tomorrow's are in. Never without market
  // prices, and not before the clock is set: 0 is never past a try.
  bool fetch_prices_due(int64_t now) {
    const int64_t until = prices.known_until(now);
    const int64_t cet_minute = floor_div(now + eu_offset(now, CET_STANDARD_OFFSET), 60) % 1440;
    if (!market_ || now - prices_tried_at_ < 5 * 60 ||
        (until >= end_of_delivery_day(now) && cet_minute < 12 * 60 + 45) || until >= end_of_next_delivery_day(now))
      return false;
    prices_tried_at_ = now;
    return true;
  }
  const Schedule &schedule() const { return schedule_; }
  // Whether prices come from the market; without market prices to download, every quarter-hour's spot price is 0: the
  // tariff is the whole price. Settings set it both ways, as they apply in place while the board has no car.
  void set_market_prices(bool market) { market_ = market; }
  // The plan's fees, VAT and the supplier's margin (see Tariff); until they're set, schedules use spot prices only.
  void set_tariff(const Tariff &tariff) {
    tariff_ = tariff;
    reschedule_ = true;
  }

  Decision tick(const CarState &car, const Settings &settings) {
    Decision d = decide_(car, settings);
    notify_(car.now, settings, d);
    if (d.mode.empty())
      d.mode = hold_ == Hold::NOW ? "now" : hold_ == Hold::NONE ? "none" : "schedule";
    return d;
  }

 private:
  // What the buttons (or a start from the car) chose. It holds until the car is unplugged; a real plug-in
  // also starts afresh, in case the board missed the unplug. The board persists it as an int: keep the values.
  enum class Hold { SCHEDULE = 0, NOW = 1, NONE = 2 };
  // The car's stay, as it last reported it: unknown after a restart while it sleeps, as the car reports the plug state
  // only awake; then unplugged, or plugged in and not charging, charging, or charged, its own word (Complete), which it
  // keeps a percent under the limit.
  enum class Stay { UNKNOWN, UNPLUGGED, PLUGGED_IN, CHARGING, CHARGED };
  static constexpr int64_t WAKE_FOR = 30 * 60;     // how long it wakes the car to learn the plug state or battery level
  static constexpr int64_t COMMAND_GAP = 2 * 60;   // the least time between the board's own starts and stops
  static constexpr int COMMANDS_PER_SCHEDULE = 3;  // the most of them for one schedule

  Decision decide_(const CarState &car, const Settings &settings) {
    Decision d;
    const auto waiting = [&d](const char *status) {  // no schedule possible yet
      d.status = status;
      d.mode = "wait";
      return d;
    };
    if (!car.paired)  // the car tells a key it doesn't know nothing
      return waiting("Not paired");
    if (car.now == 0)
      return waiting("Starting up");
    const int64_t now = car.now;
    if (first_tick_at_ == 0)
      first_tick_at_ = now;
    if (!market_)  // as far ahead as market prices go, so the page offers the same times for Ready by
      for (int64_t t = start_of_delivery_day(now); t < end_of_next_delivery_day(now); t += SLOT_SECONDS)
        prices.set(t, 0.0f);
    const bool charging = observe_(car, now);
    d.save_savings = counter_.count(savings, now, car.charging_state == "Charging", car.power_kw,
                                    stay_ != Stay::UNKNOWN ? std::optional<bool>(plugged_() && !full_()) : std::nullopt,
                                    prices, tariff_, settings);
    d.savings = format_savings(savings);
    prices.drop_before(start_of_delivery_day(now));  // after counting, which may need the day before
    if (floor_to_slot(now) != scheduled_slot_ || reschedule_)
      update_schedule_(now, settings);

    if (stay_ == Stay::UNKNOWN) {
      // The plug state comes only from an awake car, so it's unknown after the board restarts while
      // the car sleeps. With the charge port flap open the car may be plugged in: wake it to find
      // out, every 10 minutes for half an hour.
      if (car.port_open && plug_unknown_since_ == 0)
        plug_unknown_since_ = now;
      const bool checking = car.port_open && now - plug_unknown_since_ < WAKE_FOR;
      if (checking && wake_due_(now))
        d.command = Command::WAKE;
      return waiting(checking ? "Checking the car" : "Waiting for car");
    }
    if (!plugged_()) {
      d.status = "Unplugged";
      d.mode = "unplugged";
      return d;
    }
    d.unlock_port = finished_ && settings.unlock_when_charged;

    bool want_charge = true;  // as usual, without a schedule
    bool fallback = false;    // at any price, for lack of prices or a battery level
    if (hold_ == Hold::NOW) {
      d.status = "Charging now";
    } else if (hold_ == Hold::NONE) {
      want_charge = false;
      d.status = "No schedule";
    } else if (!battery_known_()) {
      if (battery_unknown_since_ == 0)
        battery_unknown_since_ = now;
      if (now - battery_unknown_since_ < WAKE_FOR) {
        if (!charging && wake_due_(now))
          d.command = Command::WAKE;
        return waiting("Reading battery");
      }
      d.status = "Charging (battery unknown)";
      fallback = true;
    } else if (!schedule_.valid) {
      if (getting_prices_(now))
        return waiting("Getting prices");
      d.status = "Charging (no prices)";
      fallback = true;
    } else {
      want_charge = in_schedule_();
      // In a window's last 2 minutes, with the car not charging: not worth a start that the next quarter-hour's
      // schedule stops, which the command gap delays into that quarter-hour.
      if (!charging && !schedule_.contains(scheduled_slot_ + SLOT_SECONDS) &&
          scheduled_slot_ + SLOT_SECONDS - now < COMMAND_GAP)
        want_charge = false;
      d.status = want_charge ? "Charging" : next_window_status_(now, settings.standard_offset);
      d.windows = format_windows(schedule_, settings.currency);
    }
    if (want_charge && charged_()) {
      d.status = "Charged";
      return d;
    }
    if (fallback)  // only once the car really charges at any price: a full car waits for a higher limit
      notify_fallback_();
    if (car.charging_known() && car.charging_state != "No Power")  // "Unknown" says nothing: the request stands
      no_power_asked_ = false;
    // The charger withholds power (an OCPP box waiting for approval, its own schedule): one start, so the car
    // charges as soon as power comes, and another every 10 minutes in case its request lapsed. The board's
    // last start counts as that request too.
    if (want_charge && car.charging_state == "No Power") {
      d.status = "Charger has no power";
      if (now - asked_at_ >= 10 * 60) {
        asked_at_ = now;
        d.command = Command::START_CHARGING;
      }
      no_power_asked_ = true;
      return d;
    }
    if (want_charge && !fallback && !charging)  // the status is about the board's start, until the car charges
      d.status = commands_this_schedule_ >= COMMANDS_PER_SCHEDULE && now - last_command_at_ >= COMMAND_GAP
                     ? "Can't start charging"
                     : "Starting";
    d.command = command_(want_charge, charging || no_power_asked_, now);  // a stop ends the request the charger holds
    if (d.command == Command::STOP_CHARGING)
      no_power_asked_ = false;
    return d;
  }

  // Takes in what the car reports: battery, plug-ins and unplugs, and starts from the car or the Tesla app.
  // Returns whether the car charges.
  bool observe_(const CarState &car, int64_t now) {
    const bool battery_was_known = battery_known_(), was_charged = charged_();
    finished_ = false;
    if (!std::isnan(car.soc))
      soc_ = car.soc;
    if (!std::isnan(car.limit)) {
      if (car.limit != limit_)
        reschedule_ = true;  // a new charge limit changes the schedule right away
      if (car.limit > limit_ && stay_ == Stay::CHARGED)
        limit_raised_at_ = now;  // a finished charge resumes by itself
      limit_ = car.limit;
    }

    // The car's stay after this report, and what changed: a plug-in, an unplug, a start or the end of a charge.
    const Stay was = stay_, next = stay_after_(car);
    if ((next >= Stay::PLUGGED_IN) != (was >= Stay::PLUGGED_IN)) {
      reschedule_ = true;
      fallback_told_ = false;
      if (next >= Stay::PLUGGED_IN) {
        plugged_since_ = now;
        // The plug state first seen after a restart is not a plug-in: no message for it, and what the
        // buttons chose before the restart still holds. A real plug-in starts afresh.
        notify_pending_ = was != Stay::UNKNOWN;
        if (was != Stay::UNKNOWN)
          hold_ = Hold::SCHEDULE;
      } else {
        hold_ = Hold::SCHEDULE;
        battery_unknown_since_ = 0;
        notify_pending_ = false;
      }
    }
    // Charged by its level, short of Complete: a car that charges again is finishing, as after a charger's pause.
    const bool finishing = charged_() && was != Stay::CHARGED;
    stay_ = next;
    if (next == Stay::CHARGING && was != Stay::CHARGING) {
      const bool we_started_it = now - started_at_ < 5 * 60;
      // A Tesla starts by itself when plugged in, and when a higher limit resumes a finished charge (after
      // Stop charging, that's the app's start).
      const bool auto_start = now - plugged_since_ < 3 * 60 || (hold_ != Hold::NONE && now - limit_raised_at_ < 3 * 60);
      if (!we_started_it && !auto_start && !finishing && (hold_ == Hold::NONE || !in_schedule_())) {
        notify_car_start_ = hold_ != Hold::NOW;  // once per hold: from the car or the Tesla app
        hold_ = Hold::NOW;                       // leave it alone until unplugged
      }
    }
    finished_ = was == Stay::CHARGING && next == Stay::CHARGED;
    // A battery level at last, or a car charged or no longer, schedules from another battery level.
    if (battery_known_() != battery_was_known || charged_() != was_charged)
      reschedule_ = true;
    return charging_();  // the last known state: "Unknown" says nothing about it
  }

  // The car's stay after a report: its plug state while it has one (with "Unknown", esphome-tesla-ble reports the
  // charger as unplugged too, which is no unplug), and while it's plugged in, its charging state while that's known.
  Stay stay_after_(const CarState &car) const {
    Stay stay = stay_;
    if (car.charging_state != "Unknown" && car.plugged.has_value())
      stay = *car.plugged ? std::max(stay, Stay::PLUGGED_IN) : Stay::UNPLUGGED;
    if (stay >= Stay::PLUGGED_IN && car.charging_known())
      stay = car.charging_state == "Charging" || car.charging_state == "Starting" ? Stay::CHARGING
             : car.charging_state == "Complete"                                   ? Stay::CHARGED
                                                                                  : Stay::PLUGGED_IN;
    return stay;
  }

  // Schedules from the current quarter-hour; the schedule stands until the next one or reschedule().
  void update_schedule_(int64_t now, const Settings &settings) {
    // A charged car is at its limit, whatever the level reads: it won't take a start.
    const ScheduleRequest request{now, deadline_(now, settings), charged_() ? limit_ : soc_, limit_, tariff_, settings};
    schedule_ = battery_known_() ? make_schedule(prices, request) : Schedule{};
    scheduled_slot_ = floor_to_slot(now);
    reschedule_ = false;
    commands_this_schedule_ = 0;
  }

  // Charging at any price from now on, for lack of prices or a battery level: the phone hears it once per
  // plug state. When it comes first, the plug-in message says it.
  void notify_fallback_() {
    if (!fallback_told_)
      notify_pending_ = true;
    fallback_told_ = true;
  }

  // After a plug-in, one message once its schedule has settled: the battery levels, deadline, average price
  // and window count, why nothing was bought, or else the status. A start from the car or the Tesla app
  // outside the schedule sends one at once, as the board then charges at any price. So does Ready by passing
  // with the car short while the schedule decides, after the plug-in message: a night that went wrong.
  void notify_(int64_t now, const Settings &settings, Decision &d) {
    const bool deadline_passed = deadline_passed_(now, settings);
    if (notify_car_start_) {
      notify_car_start_ = false;
      notify_pending_ = false;  // a plug-in message still pending would only repeat this
      Notification &n = d.notification.emplace();
      n.message = "Started from the car or the Tesla app: charging";
      if (!std::isnan(limit_))
        n.message += " to " + std::to_string(std::lround(limit_)) + "%";
      n.message += " at any price until you unplug";
      return;
    }
    if (deadline_passed && plugged_() && hold_ == Hold::SCHEDULE && battery_known_() && !full_() && !notify_pending_) {
      Notification &n = d.notification.emplace();
      n.message = "Ready by passed at " + std::to_string(std::lround(soc_)) + "% of " +
                  std::to_string(std::lround(limit_)) + "%";
      return;
    }
    if (!notify_pending_)
      return;
    if (hold_ == Hold::NONE) {  // you've already stopped it from the page
      notify_pending_ = false;
      return;
    }
    const bool settled = battery_known_() ? now - plugged_since_ >= 2 * 60 : now - battery_unknown_since_ >= WAKE_FOR;
    const bool waiting_for_prices = schedule_.needed_slots > 0 && schedule_.unpriced_slots > 0;
    if (!settled || (!schedule_.valid && getting_prices_(now)) || (waiting_for_prices && hold_ != Hold::NOW))
      return;
    notify_pending_ = false;
    Notification &n = d.notification.emplace();
    if (hold_ == Hold::NOW || !schedule_.valid) {  // no schedule without the battery level either
      n.message = d.status;
      return;
    }
    if (schedule_.windows.empty()) {
      n.message = schedule_.needed_slots == 0 ? "Not needed: battery at limit" : "No time left before Ready by";
      return;
    }
    int windows = 0;
    for (const Window &w : schedule_.windows)
      windows += !w.spare();
    char text[128];
    std::snprintf(text, sizeof(text), "%.0f to %.0f%% by %s; avg %.3f %s/kWh over %d window(s)", soc_, limit_,
                  format_day_hhmm(deadline_(now, settings), settings.standard_offset).c_str(), schedule_.avg_price,
                  settings.currency, windows);
    n.title = "Tesla schedule created";
    n.message = text;
    if (schedule_.soc_at_end < limit_ - FULL_WITHIN)
      n.message += "\nNot enough time to reach the limit";
  }

  // The board's own starts and stops: at most one every COMMAND_GAP, and COMMANDS_PER_SCHEDULE for each schedule, which
  // is made anew each quarter-hour and on reschedule().
  Command command_(bool want_charge, bool charging, int64_t now) {
    if (want_charge == charging || now - last_command_at_ < COMMAND_GAP ||
        commands_this_schedule_ >= COMMANDS_PER_SCHEDULE)
      return Command::NONE;
    last_command_at_ = now;
    started_at_ = asked_at_ = want_charge ? now : 0;
    ++commands_this_schedule_;
    return want_charge ? Command::START_CHARGING : Command::STOP_CHARGING;
  }

  // What each button does: holds its choice, and lets the next command go out at once.
  void press_(Hold hold) {
    hold_ = hold;
    last_command_at_ = 0;
    commands_this_schedule_ = 0;
    asked_at_ = 0;  // a button asks again at once, also while the charger has no power
  }

  // At most one wake-up every 10 minutes.
  bool wake_due_(int64_t now) {
    if (now - last_wake_at_ < 10 * 60)
      return false;
    last_wake_at_ = now;
    return true;
  }

  // Right after a restart the prices are still downloading: give them 10 minutes before charging as
  // usual without them, so a restart doesn't start the car charging for a minute.
  bool getting_prices_(int64_t now) const { return now - first_tick_at_ < 10 * 60; }

  // The deadline in force: the one-off while it's ahead (at most a week out), else the next daily Ready by.
  int64_t deadline_(int64_t now, const Settings &settings) const {
    if (settings.ready_by_once > now)
      return std::min(settings.ready_by_once, now + 7 * DAY_SECONDS);
    return next_local_time(now, settings.ready_by_minutes, settings.standard_offset);
  }

  // Whether the deadline in force at the last tick has passed. One that only moved, as when Ready by changes, hasn't.
  bool deadline_passed_(int64_t now, const Settings &settings) {
    const int64_t last = deadline_at_;
    deadline_at_ = now != 0 ? deadline_(now, settings) : 0;
    return last != 0 && now >= last;
  }

  // The status while the schedule doesn't charge now: its next window that hasn't started.
  std::string next_window_status_(int64_t now, int32_t standard_offset) const {
    for (const Window &w : schedule_.windows)
      if (w.start > now)
        return "Charges at " + format_when(w.start, now, standard_offset);
    return schedule_.needed_slots == 0 ? "Charged" : schedule_.unpriced_slots > 0 ? "Waiting for prices" : "Waiting";
  }

  bool battery_known_() const { return !std::isnan(soc_) && !std::isnan(limit_); }

  // At the limit by the car's word, or within half a percent; without a battery level, only by the car's word.
  bool full_() const { return stay_ == Stay::CHARGED || soc_ >= limit_ - FULL_WITHIN; }

  // Full and not charging: the board starts no charge, and schedules none.
  bool charged_() const { return full_() && !charging_(); }
  bool plugged_() const { return stay_ >= Stay::PLUGGED_IN; }
  bool charging_() const { return stay_ == Stay::CHARGING; }

  // Whether the schedule charges in the quarter-hour it was made for (always without prices or battery level).
  bool in_schedule_() const { return !schedule_.valid || schedule_.contains(scheduled_slot_); }

  Schedule schedule_;
  Tariff tariff_;
  float soc_ = NAN;
  float limit_ = NAN;
  Stay stay_ = Stay::UNKNOWN;
  bool finished_ = false;  // this tick, the car went from charging to charged: it reached its limit
  Hold hold_ = Hold::SCHEDULE;
  bool reschedule_ = true;
  bool notify_pending_ = false;
  bool fallback_told_ = false;
  bool notify_car_start_ = false;
  bool market_ = true;  // prices come from the market (see set_market_prices())
  int64_t scheduled_slot_ = -1;
  int64_t plugged_since_ = 0;
  int64_t deadline_at_ = 0;  // the deadline in force at the last tick, 0 before the clock is set
  int64_t limit_raised_at_ = 0;
  int64_t battery_unknown_since_ = 0;
  int64_t plug_unknown_since_ = 0;
  int64_t first_tick_at_ = 0;  // about when the board started, once the clock is set
  int64_t prices_tried_at_ = 0;
  int64_t last_wake_at_ = 0;
  int64_t asked_at_ = 0;         // when the board last asked the car to charge; 0 after its stop or a button
  bool no_power_asked_ = false;  // the car was asked to charge while its charger had no power
  int64_t last_command_at_ = 0;
  int64_t started_at_ = 0;  // the board's last start, 0 after its stop; the buttons don't reset it
  int commands_this_schedule_ = 0;
  SavingsCounter counter_;  // counts into `savings`
};

}  // namespace esphome::scheduler
