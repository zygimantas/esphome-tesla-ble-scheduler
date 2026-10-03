#pragma once
// Charges the Tesla in the cheapest quarter-hours before Ready by.
//
// Every 30 s SchedulerComponent (scheduler_component.h) passes the car's state to Controller::tick(), which reschedules
// when needed (schedule.h), counts what charging cost and saved (savings.h), and returns what to do.
//
// Plain C++17 with nothing from ESPHome, like the headers it includes, so it can be unit-tested on a computer.

#include "calendar.h"
#include "grid.h"
#include "market.h"
#include "savings.h"
#include "schedule.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

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
};

// A one-off phone message; the YAML sends it through ntfy.
struct Notification {
  std::string title;
  std::string message;
};

struct Decision {
  Command command = Command::NONE;
  std::string status;
  std::string windows;  // format_windows() while the schedule decides, else empty
  // "schedule", "now" (charging regardless of price), "none" (no schedule) or "wait" (no schedule possible yet:
  // no clock, plug state, battery level or prices).
  std::string mode;
  std::optional<Notification> notification;  // once per plug-in, when its schedule has settled, and once more
                                             // when the board falls back to charging at any price or a
                                             // start from the car takes over
  std::string savings;                       // format_savings() once the clock is set, else empty
  bool save_savings = false;                 // Controller::savings changed: time to write it to flash
};

class Controller {
 public:
  PriceTable prices;
  // What charging cost and saved, for the board to keep in flash (see Decision::save_savings).
  Savings savings;

  // The buttons on the web page. Each acts at once, free of command_()'s limits.

  // Charge regardless of price until the car is unplugged.
  void charge_now() {
    hold_ = Hold::NOW;
    allow_command_();
  }
  // Follow the schedule: cancels charge_now(), a start from the car or app, or stop_charging().
  void create_schedule() {
    hold_ = Hold::SCHEDULE;
    allow_command_();
  }
  // No schedule and no charging until create_schedule(), charge_now(), a start from the car or app, or the car is
  // unplugged.
  void stop_charging() {
    hold_ = Hold::NONE;
    allow_command_();
  }
  // The page's Reset savings: the figures start afresh on the next tick.
  void reset_savings() { savings = Savings{}; }
  // What the buttons chose, for the board to keep across a restart (see Hold).
  int held_mode() const { return static_cast<int>(hold_); }
  void restore_mode(int mode) { hold_ = mode == 1 ? Hold::NOW : mode == 2 ? Hold::NONE : Hold::SCHEDULE; }
  // Recompute the schedule on the next tick (new prices or settings).
  void reschedule() { reschedule_ = true; }
  // Whether to download prices now, which counts as a try: every 5 minutes while there's no price for this
  // quarter-hour, else hourly until 12:45 CET and every 5 minutes from then, when Nord Pool publishes the next day,
  // until tomorrow's are in. Never without market prices, and not before the clock is set: 0 is never past a try.
  bool fetch_prices_due(int64_t now) {
    const int64_t until = prices.known_until(now);
    const int64_t cet_minute = floor_div(now + eu_offset(now, CET_STANDARD_OFFSET), 60) % 1440;
    const int64_t wait = until > now && cet_minute < 12 * 60 + 45 ? 60 * 60 : 5 * 60;
    if (!market_ || now - prices_tried_at_ < wait || until >= end_of_next_delivery_day(now))
      return false;
    prices_tried_at_ = now;
    return true;
  }
  const Schedule &schedule() const { return schedule_; }
  // Without market prices to download, every quarter-hour's spot price is 0: the grid fees are the whole price.
  void without_market_prices() { market_ = false; }
  // The grid's VAT and fees, from the price list and config.yaml; until they're set, schedules use spot prices only.
  void set_grid(const Grid &grid) {
    grid_ = grid;
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
  static constexpr int64_t WAKE_FOR = 30 * 60;  // how long it wakes the car to learn the plug state or battery level

  Decision decide_(const CarState &car, const Settings &settings) {
    Decision d;
    if (car.now == 0) {
      d.status = "Starting up";
      d.mode = "wait";
      return d;
    }
    const int64_t now = car.now;
    if (first_tick_at_ == 0)
      first_tick_at_ = now;
    if (!market_)  // as far ahead as Nord Pool's prices go, so the page offers the same times for Ready by
      for (int64_t t = start_of_delivery_day(now); t < end_of_next_delivery_day(now); t += SLOT_SECONDS)
        prices.set(t, 0.0f);
    const bool charging = observe_(car, now);
    count_(car, now, settings, d);
    prices.drop_before(start_of_delivery_day(now));  // after count_(), which may need the day before
    if (floor_to_slot(now) != scheduled_slot_ || reschedule_)
      update_schedule_(now, settings);

    if (!plug_state_seen_) {
      // The plug state comes only from an awake car, so it's unknown after the board restarts while
      // the car sleeps. With the charge port flap open the car may be plugged in: wake it to find
      // out, every 10 minutes for half an hour.
      if (car.port_open && plug_unknown_since_ == 0)
        plug_unknown_since_ = now;
      if (car.port_open && now - plug_unknown_since_ < WAKE_FOR) {
        d.status = "Checking the car";
        if (wake_due_(now))
          d.command = Command::WAKE;
      } else {
        d.status = "Waiting for car";
      }
      d.mode = "wait";
      return d;
    }
    if (!plugged_) {
      d.status = "Unplugged";
      return d;
    }

    bool want_charge = true;  // as usual, without a schedule
    bool fallback = false;    // at any price, for lack of prices or a battery level
    bool starting = false;    // the status is about the start, until the car charges
    if (hold_ == Hold::NOW) {
      d.status = "Charging now";
      starting = true;
    } else if (hold_ == Hold::NONE) {
      want_charge = false;
      d.status = "No schedule";
    } else if (!battery_known_()) {
      if (battery_unknown_since_ == 0)
        battery_unknown_since_ = now;
      if (now - battery_unknown_since_ < WAKE_FOR) {
        d.status = "Reading battery";
        d.mode = "wait";
        if (!charging && wake_due_(now))
          d.command = Command::WAKE;
        return d;
      }
      d.status = "Charging (battery unknown)";
      fallback = true;
    } else if (!schedule_.valid) {
      if (getting_prices_(now)) {
        d.status = "Getting prices";
        d.mode = "wait";
        return d;
      }
      d.status = "Charging (no prices)";
      fallback = true;
    } else {
      want_charge = in_schedule_();
      // A schedule made in a window's last 2 minutes: not worth a start that the next quarter-hour's schedule
      // stops, which the command limit delays into that quarter-hour.
      if (!charging && !schedule_.contains(scheduled_slot_ + SLOT_SECONDS) &&
          scheduled_slot_ + SLOT_SECONDS - now < 2 * 60)
        want_charge = false;
      d.status = want_charge ? "Charging" : next_window_status_(now, settings.standard_offset);
      d.windows = format_windows(schedule_, settings.currency);
      starting = want_charge;
    }
    if (want_charge && full_() && !charging) {
      d.status = "Charged";
      return d;
    }
    if (fallback)  // only once the car really charges at any price: a full car waits for a higher limit
      notify_fallback_();
    const bool reported = !car.charging_state.empty() && car.charging_state != "Unknown";
    if (reported && car.charging_state != "No Power")  // "Unknown" says nothing: the request stands
      no_power_asked_ = false;
    // The charger withholds power (an OCPP box waiting for approval, its own schedule): one start, so the car
    // charges as soon as power comes, and another every 10 minutes in case its request lapsed. The board's
    // last start counts as that request too.
    if (want_charge && !charging && car.charging_state == "No Power") {
      d.status = "Charger has no power";
      if (now - std::max(no_power_asked_at_, started_at_) >= 10 * 60) {
        no_power_asked_at_ = now;
        d.command = Command::START_CHARGING;
      }
      no_power_asked_ = true;
      return d;
    }
    if (starting && !charging)
      d.status = commands_this_schedule_ >= 3 && now - last_command_at_ >= 2 * 60 ? "Can't start charging" : "Starting";
    d.command = command_(want_charge, charging || no_power_asked_, now);  // a stop ends the request the charger holds
    if (d.command == Command::STOP_CHARGING)
      no_power_asked_ = false;
    return d;
  }

  // Takes in what the car reports: battery, plug-ins and unplugs, and starts from the car or the Tesla app.
  // Returns whether the car charges.
  bool observe_(const CarState &car, int64_t now) {
    const bool battery_was_known = battery_known_();
    if (!std::isnan(car.soc))
      soc_ = car.soc;
    if (!std::isnan(car.limit)) {
      if (car.limit != limit_)
        reschedule_ = true;  // a new charge limit changes the schedule right away
      if (car.limit > limit_ && complete_)
        limit_raised_at_ = now;  // a finished charge resumes by itself
      limit_ = car.limit;
    }
    if (battery_known_() != battery_was_known)
      reschedule_ = true;

    // With "Unknown", esphome-tesla-ble also reports the charger as unplugged: that's no unplug.
    const std::optional<bool> plugged = car.charging_state == "Unknown" ? std::nullopt : car.plugged;
    if (plugged.has_value() && *plugged != plugged_) {
      plugged_ = *plugged;
      reschedule_ = true;
      fallback_told_ = false;
      if (plugged_) {
        plugged_since_ = now;
        // The plug state first seen after a restart is not a plug-in: no message for it, and what the
        // buttons chose before the restart still holds. A real plug-in starts afresh.
        notify_pending_ = plug_state_seen_;
        if (plug_state_seen_)
          hold_ = Hold::SCHEDULE;
      } else {
        hold_ = Hold::SCHEDULE;
        battery_unknown_since_ = 0;
        notify_pending_ = false;
        notify_car_start_ = false;
      }
    }
    if (plugged.has_value())
      plug_state_seen_ = true;

    const bool charging_known = !car.charging_state.empty() && car.charging_state != "Unknown";
    const bool charging = car.charging_state == "Charging" || car.charging_state == "Starting";
    if (charging_known) {
      const bool we_started_it = now - started_at_ < 5 * 60;
      // A Tesla starts by itself when plugged in, and when a higher limit resumes a finished charge (after
      // Stop charging, that's the app's start).
      const bool auto_start = now - plugged_since_ < 3 * 60 || (hold_ != Hold::NONE && now - limit_raised_at_ < 3 * 60);
      const bool started_by_car = charging && !charging_ && !we_started_it && !auto_start;
      if (started_by_car && plugged_ && (hold_ == Hold::NONE || !in_schedule_())) {
        notify_car_start_ = hold_ != Hold::NOW;  // once per hold
        hold_ = Hold::NOW;                       // from the car or the Tesla app: leave it alone until unplugged
      }
      charging_ = charging;
      complete_ = car.charging_state == "Complete";
    }
    return charging_;  // the last known state: "Unknown" says nothing about it
  }

  // Schedules from the current quarter-hour; the schedule stands until the next one or reschedule().
  void update_schedule_(int64_t now, const Settings &settings) {
    // A car that says Complete is at its limit, whatever the level reads: it won't take a start.
    const ScheduleRequest request{now, deadline_(now, settings), complete_ ? limit_ : soc_, limit_, grid_, settings};
    schedule_ = battery_known_() ? make_schedule(prices, request) : Schedule{};
    scheduled_slot_ = floor_to_slot(now);
    reschedule_ = false;
    commands_this_schedule_ = 0;
  }

  // Counts the energy the car took since the last tick (its power times the time, at most a minute back) into
  // `savings`: at the total price of each quarter-hour, at the delivery day's average total price, and at the price
  // it would have had charging at once. Charging at once starts when the car needs charging, at a plug-in or when it's
  // no longer full, and replays the car's charging time from then. Until it starts (after a restart, when the car was
  // plugged in is unknown), past a day of it, or without its price, it counts as the day's average. A quarter-hour
  // without a price counts as the day's average throughout, or as nothing without that: it neither saves nor costs.
  void count_(const CarState &car, int64_t now, const Settings &settings, Decision &d) {
    const int64_t today = local_day_of(now, settings.standard_offset);
    if (std::strncmp(savings.currency, settings.currency, sizeof(savings.currency)) != 0) {
      savings = Savings{};
      std::snprintf(savings.currency, sizeof(savings.currency), "%s", settings.currency);
      savings.day = static_cast<int32_t>(today);
      counted_day_ = -1;
      savings_unsaved_ = true;
    }
    for (; savings.day < today; ++savings.day)  // a new day takes the place of the one a year before
      savings.days[ring_index(savings.day + 1)] = SavingsDay{};

    const bool needed = plugged_ && !full_();
    if (needed && !needed_) {
      at_once_prices_.assign(DAY_SECONDS / SLOT_SECONDS, NAN);
      at_once_from_ = now;
      at_once_charged_ = 0;
    }
    if (plug_state_seen_)
      needed_ = needed;
    for (size_t i = 0; i < at_once_prices_.size(); ++i) {  // as each price comes out, then kept
      const int64_t slot = floor_to_slot(at_once_from_) + static_cast<int64_t>(i) * SLOT_SECONDS;
      const std::optional<float> spot = std::isnan(at_once_prices_[i]) ? prices.get(slot) : std::nullopt;
      if (spot)
        at_once_prices_[i] = total_price(*spot, slot, grid_, settings.standard_offset);
    }

    const bool charging = car.charging_state == "Charging";
    for (int64_t from = std::max(counted_until_, now - 60); charging && car.power_kw > 0 && from < now;) {
      const int64_t slot = floor_to_slot(from);
      const int64_t to = std::min(now, slot + SLOT_SECONDS);
      const float kwh = car.power_kw * static_cast<float>(to - from) / 3600.0f;
      const float average = day_average_(slot, settings.standard_offset);
      const auto replayed =
          static_cast<size_t>((at_once_from_ - floor_to_slot(at_once_from_) + at_once_charged_) / SLOT_SECONDS);
      const float at_once = replayed < at_once_prices_.size() && !std::isnan(at_once_prices_[replayed])
                                ? at_once_prices_[replayed]
                                : average;
      at_once_charged_ += to - from;
      const int64_t day = local_day_of(slot, settings.standard_offset);
      if (const std::optional<float> spot = prices.get(slot)) {
        add_energy_(day, kwh, total_price(*spot, slot, grid_, settings.standard_offset), average, at_once);
      } else {
        const float neutral = std::isnan(average) ? 0.0f : average;
        add_energy_(day, kwh, neutral, neutral, neutral);
      }
      from = to;
    }
    counted_until_ = now;

    d.savings = format_savings(savings);
    // While the car charges, once a quarter-hour: a restart loses at most that.
    d.save_savings = savings_unsaved_ && (floor_to_slot(now) != savings_saved_slot_ || !charging);
    if (d.save_savings) {
      savings_unsaved_ = false;
      savings_saved_slot_ = floor_to_slot(now);
    }
  }

  // Adds `kwh` bought at `paid` per kWh, and its cost at the two other prices, to a local day. The day's figures add
  // up in floats, as the cents of each tick would round away.
  void add_energy_(int64_t day, float kwh, float paid, float average, float at_once) {
    SavingsDay &figures = savings.days[ring_index(day)];
    if (day != counted_day_) {
      counted_day_ = day;
      counted_ = {static_cast<float>(figures.wh) / 1000.0f, static_cast<float>(figures.paid) / 100.0f,
                  static_cast<float>(figures.average) / 100.0f, static_cast<float>(figures.at_once) / 100.0f};
    }
    counted_.kwh += kwh;
    counted_.paid += kwh * paid;
    counted_.average += kwh * average;
    counted_.at_once += kwh * at_once;
    figures.wh = static_cast<uint32_t>(std::lround(counted_.kwh * 1000.0f));
    figures.paid = static_cast<int32_t>(std::lround(counted_.paid * 100.0f));
    figures.average = static_cast<int32_t>(std::lround(counted_.average * 100.0f));
    figures.at_once = static_cast<int32_t>(std::lround(counted_.at_once * 100.0f));
    savings_unsaved_ = true;
  }

  // The average total price of the delivery day of `t`, over the quarter-hours with a price; NaN without one.
  float day_average_(int64_t t, int32_t standard_offset) const {
    float sum = 0;
    int count = 0;
    prices.for_each(start_of_delivery_day(t), end_of_delivery_day(t), [&](int64_t slot, float spot) {
      sum += total_price(spot, slot, grid_, standard_offset);
      ++count;
    });
    return sum / static_cast<float>(count);  // NaN (0 / 0) without prices
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
  // outside the schedule sends one at once, as the board then charges at any price.
  void notify_(int64_t now, const Settings &settings, Decision &d) {
    if (notify_car_start_) {
      notify_car_start_ = false;
      notify_pending_ = false;  // a plug-in message still pending would only repeat this
      Notification &n = d.notification.emplace();
      n.title = "Tesla charging";
      n.message = "Started from the car or the Tesla app: charging";
      if (!std::isnan(limit_))
        n.message += " to " + std::to_string(std::lround(limit_)) + "%";
      n.message += " at any price until you unplug";
      return;
    }
    if (!notify_pending_)
      return;
    if (hold_ == Hold::NONE) {  // you've already stopped it from the page
      notify_pending_ = false;
      return;
    }
    const bool settled = battery_known_() ? now - plugged_since_ >= 2 * 60 : now - battery_unknown_since_ >= WAKE_FOR;
    const bool waiting_for_prices = schedule_.valid && schedule_.needed_slots > 0 && schedule_.unpriced_slots > 0;
    if (!settled || (!schedule_.valid && getting_prices_(now)) || (waiting_for_prices && hold_ != Hold::NOW))
      return;
    notify_pending_ = false;
    Notification &n = d.notification.emplace();
    n.title = "Tesla charging";
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
    n.title = "Tesla charging schedule created";
    n.message = text;
    if (schedule_.soc_at_end < limit_ - 0.5f)
      n.message += "\nNot enough time to reach the limit";
  }

  // The board's own starts and stops: at most one every 2 minutes, and 3 per schedule, which is made anew each
  // quarter-hour and on reschedule().
  Command command_(bool want_charge, bool charging, int64_t now) {
    if (want_charge == charging || now - last_command_at_ < 2 * 60 || commands_this_schedule_ >= 3)
      return Command::NONE;
    last_command_at_ = now;
    started_at_ = want_charge ? now : 0;
    ++commands_this_schedule_;
    return want_charge ? Command::START_CHARGING : Command::STOP_CHARGING;
  }

  void allow_command_() {
    last_command_at_ = 0;
    commands_this_schedule_ = 0;
    no_power_asked_at_ = 0;  // a button asks again at once, also while the charger has no power
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

  // The status while the schedule doesn't charge; its windows are all ahead then.
  std::string next_window_status_(int64_t now, int32_t standard_offset) const {
    if (!schedule_.windows.empty())
      return "Charges at " + format_when(schedule_.windows.front().start, now, standard_offset);
    return schedule_.needed_slots == 0 ? "Charged" : schedule_.unpriced_slots > 0 ? "Waiting for prices" : "Waiting";
  }

  bool battery_known_() const { return !std::isnan(soc_) && !std::isnan(limit_); }

  // At the limit by the car's word, or within half a percent; false while the battery level is unknown (NaN).
  bool full_() const { return complete_ || soc_ >= limit_ - 0.5f; }

  // Whether the schedule charges in the quarter-hour it was made for (always without prices or battery level).
  bool in_schedule_() const { return !schedule_.valid || schedule_.contains(scheduled_slot_); }

  Schedule schedule_;
  Grid grid_;
  float soc_ = NAN;
  float limit_ = NAN;
  bool plugged_ = false;
  bool charging_ = false;
  bool complete_ = false;  // the car's own word, which it keeps a percent under the limit
  Hold hold_ = Hold::SCHEDULE;
  bool reschedule_ = true;
  bool notify_pending_ = false;
  bool fallback_told_ = false;
  bool notify_car_start_ = false;
  bool plug_state_seen_ = false;
  bool market_ = true;  // prices come from Nord Pool (see without_market_prices())
  int64_t scheduled_slot_ = -1;
  int64_t plugged_since_ = 0;
  int64_t limit_raised_at_ = 0;
  int64_t battery_unknown_since_ = 0;
  int64_t plug_unknown_since_ = 0;
  int64_t first_tick_at_ = 0;  // about when the board started, once the clock is set
  int64_t prices_tried_at_ = 0;
  int64_t last_wake_at_ = 0;
  int64_t no_power_asked_at_ = 0;
  bool no_power_asked_ = false;  // the car was asked to charge while its charger had no power
  int64_t last_command_at_ = 0;
  int64_t started_at_ = 0;  // the board's last start, 0 after its stop; the buttons don't reset it
  int commands_this_schedule_ = 0;

  // Savings (see count_()).
  struct Figures {
    float kwh, paid, average, at_once;
  };
  Figures counted_{};  // of counted_day_
  bool savings_unsaved_ = false;
  bool needed_ = true;  // plugged in and not full, at the last tick with a plug state
  int64_t counted_day_ = -1;
  int64_t counted_until_ = 0;
  int64_t savings_saved_slot_ = 0;
  // Charging at once: the total prices of a day of quarter-hours from at_once_from_, NaN until known, and how long the
  // car has charged since.
  std::vector<float> at_once_prices_;
  int64_t at_once_from_ = 0;
  int64_t at_once_charged_ = 0;
};

}  // namespace esphome::scheduler
