#pragma once
// Charges the Tesla in the cheapest quarter-hours before Ready by.
//
// A plan takes the cheapest slots, priced at Nord Pool's spot price plus VAT and the grid fee, that
// bring the battery to the car's charge limit, plus a buffer slot. Every 30 s ChargingComponent
// (charging_component.h) passes the car's state to Controller::tick(), which re-plans when needed and
// returns what to do.
//
// Plain C++17 plus ArduinoJson, with nothing from ESPHome, so it can be unit-tested on a computer.

#include <ArduinoJson.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numeric>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace esphome::charging {

// A slot is one quarter-hour, Nord Pool's price period.
constexpr int64_t SLOT_SECONDS = 15 * 60;
constexpr int64_t DAY_SECONDS = 24 * 3600;
// The default winter offset from UTC; the YAML sets the board's own from its clock's time zone.
constexpr int32_t VILNIUS_STANDARD_OFFSET = 2 * 3600;

// ---------------------------------------------------------------------------
// Calendar helpers (UTC, proleptic Gregorian; no libc time zone support needed)
// ---------------------------------------------------------------------------

// a / b rounded down, for b > 0.
inline int64_t floor_div(int64_t a, int64_t b) { return a / b - (a % b < 0); }

// 0 = Sunday; day 0, 1970-01-01, was a Thursday.
inline int weekday(int64_t day) { return static_cast<int>((day % 7 + 11) % 7); }

inline int64_t floor_to_slot(int64_t t) { return floor_div(t, SLOT_SECONDS) * SLOT_SECONDS; }

// Howard Hinnant's days_from_civil / civil_from_days.
inline int64_t days_from_civil(int64_t y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int64_t era = floor_div(y, 400);
  const auto yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + static_cast<int64_t>(doe) - 719468;
}

struct CivilDate {
  int64_t year;
  unsigned month;
  unsigned day;
};

inline CivilDate civil_from_days(int64_t z) {
  z += 719468;
  const int64_t era = floor_div(z, 146097);
  const auto doe = static_cast<unsigned>(z - era * 146097);
  const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const unsigned mp = (5 * doy + 2) / 153;
  const unsigned d = doy - (153 * mp + 2) / 5 + 1;
  const unsigned m = mp < 10 ? mp + 3 : mp - 9;
  return {static_cast<int64_t>(yoe) + era * 400 + (m <= 2), m, d};
}

// Day number (days since 1970-01-01) of the last Sunday of a 31-day month.
inline int64_t last_sunday_of(int64_t year, unsigned month) {
  const int64_t last_day = days_from_civil(year, month, 31);
  return last_day - weekday(last_day);
}

// EU summer time: last Sunday of March 01:00 UTC to last Sunday of October 01:00 UTC.
inline bool eu_summer_time(int64_t utc) {
  const int64_t year = civil_from_days(floor_div(utc, DAY_SECONDS)).year;
  const int64_t start = last_sunday_of(year, 3) * DAY_SECONDS + 3600;
  const int64_t end = last_sunday_of(year, 10) * DAY_SECONDS + 3600;
  return utc >= start && utc < end;
}

inline int32_t eu_offset(int64_t utc, int32_t standard_offset) {
  return standard_offset + (eu_summer_time(utc) ? 3600 : 0);
}

// The local calendar day (days since 1970-01-01) of a moment.
inline int64_t local_day_of(int64_t utc, int32_t standard_offset) {
  return floor_div(utc + eu_offset(utc, standard_offset), DAY_SECONDS);
}

inline std::string format_hhmm(int64_t utc, int32_t standard_offset) {
  const int64_t local = utc + eu_offset(utc, standard_offset);
  const int64_t minute_of_day = floor_div(local, 60) - floor_div(local, DAY_SECONDS) * 1440;
  char buf[8];
  std::snprintf(buf, sizeof(buf), "%02d:%02d", static_cast<int>(minute_of_day / 60),
                static_cast<int>(minute_of_day % 60));
  return buf;
}

// The moment (UTC) that is `minutes` after local midnight on local calendar day `local_day`. A time the clocks
// skip in spring moves an hour on; one they repeat in autumn is the first.
inline int64_t local_to_utc(int64_t local_day, int minutes, int32_t standard_offset) {
  const int64_t local = local_day * DAY_SECONDS + static_cast<int64_t>(minutes) * 60;
  return local - eu_offset(local - standard_offset - 3600, standard_offset);
}

// The next moment (UTC) that is `minutes` after local midnight, strictly after `now`.
inline int64_t next_local_time(int64_t now, int minutes, int32_t standard_offset) {
  const int64_t today = local_day_of(now, standard_offset);
  const int64_t utc = local_to_utc(today, minutes, standard_offset);
  return utc > now ? utc : local_to_utc(today + 1, minutes, standard_offset);
}

// "Mon 00:00" in local time.
inline std::string format_day_hhmm(int64_t utc, int32_t standard_offset) {
  static constexpr const char *WEEKDAYS[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  return std::string(WEEKDAYS[weekday(local_day_of(utc, standard_offset))]) + " " + format_hhmm(utc, standard_offset);
}

// "23:00" for a moment less than 24 clock hours after `now` (the next 23:00 is unambiguous), else "Mon 00:00".
inline std::string format_when(int64_t utc, int64_t now, int32_t standard_offset) {
  return utc + eu_offset(utc, standard_offset) < now + eu_offset(now, standard_offset) + DAY_SECONDS
             ? format_hhmm(utc, standard_offset)
             : format_day_hhmm(utc, standard_offset);
}

// ---------------------------------------------------------------------------
// Grid: what a kWh costs in each quarter-hour
// ---------------------------------------------------------------------------

// Easter Sunday (anonymous Gregorian algorithm) as a day number.
inline int64_t easter_sunday(int64_t year) {
  const int64_t a = year % 19, b = year / 100, c = year % 100, d = b / 4, e = b % 4;
  const int64_t f = (b + 8) / 25, g = (b - f + 1) / 3, h = (19 * a + b - d - g + 15) % 30;
  const int64_t i = c / 4, k = c % 4, l = (32 + 2 * e + 2 * i - h - k) % 7, m = (a + 11 * h + 22 * l) / 451;
  const int64_t month = (h + l - 7 * m + 114) / 31, day = (h + l - 7 * m + 114) % 31 + 1;
  return days_from_civil(year, static_cast<unsigned>(month), static_cast<unsigned>(day));
}

// The VAT and grid fees from config.yaml (format in docs/grid-fees.md; the build turns them into one of these): VAT
// on the spot price, and a grid fee per zone. Every hour of a workday, a weekend day and a public
// holiday belongs to a zone, named by a letter. Zone hours are local time, or winter time all year
// with clock: winter. Part of the year can have other hours (winter_from to winter_to, as month * 100
// + day, a range that may cross New Year); an empty winter string keeps the year-round one. Default:
// spot prices only.
struct Grid {
  float vat = 0.0f;
  bool winter_clock = false;
  std::string workday;  // one zone letter per hour; empty means no grid fees
  std::string weekend;
  std::string holiday;
  std::vector<float> fee;            // EUR/kWh incl. VAT, per zone letter a-z
  std::vector<uint16_t> holidays;    // public holidays on fixed dates, as month * 100 + day
  std::vector<int8_t> after_easter;  // public holidays around Easter, in days after Easter Sunday
  uint16_t winter_from = 0;
  uint16_t winter_to = 0;
  std::string winter_workday;
  std::string winter_weekend;
  std::string winter_holiday;
};

inline bool public_holiday(int64_t local_day, const Grid &grid) {
  const CivilDate date = civil_from_days(local_day);
  for (uint16_t holiday : grid.holidays)
    if (holiday == date.month * 100 + date.day)
      return true;
  const int64_t easter = easter_sunday(date.year);
  for (int8_t days : grid.after_easter)
    if (local_day == easter + days)
      return true;
  return false;
}

inline float grid_fee(int64_t utc, const Grid &grid, int32_t standard_offset) {
  if (grid.workday.empty())
    return 0.0f;
  const int64_t local = utc + (grid.winter_clock ? standard_offset : eu_offset(utc, standard_offset));
  const int64_t local_day = floor_div(local, DAY_SECONDS);
  const int64_t hour = (local - local_day * DAY_SECONDS) / 3600;
  const int day_of_week = weekday(local_day);
  const bool holiday = public_holiday(local_day, grid);
  const bool weekend = day_of_week == 0 || day_of_week == 6;
  const CivilDate date = civil_from_days(local_day);
  const auto month_day = static_cast<uint16_t>(date.month * 100 + date.day);
  const bool winter = grid.winter_from <= grid.winter_to ? grid.winter_from <= month_day && month_day <= grid.winter_to
                                                         : month_day >= grid.winter_from || month_day <= grid.winter_to;
  const std::string &special = holiday ? grid.winter_holiday : weekend ? grid.winter_weekend : grid.winter_workday;
  const std::string &hours = winter && !special.empty() ? special
                             : holiday                  ? grid.holiday
                             : weekend                  ? grid.weekend
                                                        : grid.workday;
  return grid.fee[hours[hour] - 'a'];
}

// The price of a kWh bought in the quarter-hour starting at `slot_start`, leaving out charges that
// are the same in every quarter-hour (the supplier's margin, public service obligations).
inline float total_price(float spot_price, int64_t slot_start, const Grid &grid, int32_t standard_offset) {
  return spot_price * (1.0f + grid.vat) + grid_fee(slot_start, grid, standard_offset);
}

// ---------------------------------------------------------------------------
// Nord Pool prices
// ---------------------------------------------------------------------------

// Nord Pool delivery days run midnight to midnight CET.
constexpr int32_t CET_STANDARD_OFFSET = 3600;

// Parses "2025-10-01T22:00:00Z", as Nord Pool sends it.
inline std::optional<int64_t> parse_iso8601(const char *s) {
  int year, month, day, hour, minute, second, consumed;
  if (s == nullptr ||
      std::sscanf(s, "%4d-%2d-%2dT%2d:%2d:%2d%n", &year, &month, &day, &hour, &minute, &second, &consumed) != 6 ||
      (s[consumed] != 'Z' && s[consumed] != '\0'))
    return std::nullopt;
  if (month < 1 || month > 12 || day < 1 || day > 31)
    return std::nullopt;
  return days_from_civil(year, month, day) * DAY_SECONDS + hour * 3600 + minute * 60 + second;
}

// Nord Pool's DayAheadPrices URL for the CET delivery day `day_offset` days after `now`.
inline std::string nord_pool_url(const char *area, const char *currency, int64_t now, int day_offset) {
  const CivilDate date = civil_from_days(local_day_of(now, CET_STANDARD_OFFSET) + day_offset);
  char buf[192];
  std::snprintf(buf, sizeof(buf),
                "https://dataportal-api.nordpoolgroup.com/api/DayAheadPrices"
                "?market=DayAhead&date=%04d-%02u-%02u&deliveryArea=%s&currency=%s",
                static_cast<int>(date.year), date.month, date.day, area, currency);
  return buf;
}

// End (UTC) of the CET delivery day containing `now`, and of the one after it.
inline int64_t end_of_delivery_day(int64_t now) {
  return local_to_utc(local_day_of(now, CET_STANDARD_OFFSET) + 1, 0, CET_STANDARD_OFFSET);
}
inline int64_t end_of_next_delivery_day(int64_t now) {
  return local_to_utc(local_day_of(now, CET_STANDARD_OFFSET) + 2, 0, CET_STANDARD_OFFSET);
}

class PriceTable {
  // The first slot starting at or after `t`, in the table or a const one.
  template <typename Slots>
  static auto find_(Slots &slots, int64_t t) {
    return std::lower_bound(slots.begin(), slots.end(), t, [](const auto &a, int64_t v) { return a.first < v; });
  }

 public:
  // Stores the quarter-hour prices (EUR/kWh) from a Nord Pool DayAheadPrices response.
  // Returns how many quarter-hours were stored, or -1 if the JSON could not be parsed.
  int add_nord_pool(const char *json, size_t length, const char *area) {
    JsonDocument filter;
    filter["multiAreaEntries"][0]["deliveryStart"] = true;
    filter["multiAreaEntries"][0]["deliveryEnd"] = true;
    filter["multiAreaEntries"][0]["entryPerArea"][area] = true;
    JsonDocument doc;
    if (deserializeJson(doc, json, length, DeserializationOption::Filter(filter)) != DeserializationError::Ok)
      return -1;
    int stored = 0;
    for (JsonObject entry : doc["multiAreaEntries"].as<JsonArray>()) {
      JsonVariant value = entry["entryPerArea"][area];
      const auto start = parse_iso8601(entry["deliveryStart"].as<const char *>());
      const auto end = parse_iso8601(entry["deliveryEnd"].as<const char *>());
      if (value.isNull() || !start || !end)
        continue;
      // Hourly entries (before Nord Pool's October 2025 switch to 15-minute prices) become four slots.
      for (int64_t t = *start; t < *end; t += SLOT_SECONDS, ++stored)
        set(t, value.as<float>() / 1000.0f);
    }
    return stored;
  }

  void set(int64_t slot_start, float price) {
    auto it = find_(slots_, slot_start);
    if (it != slots_.end() && it->first == slot_start)
      it->second = price;
    else
      slots_.insert(it, {slot_start, price});
  }

  std::optional<float> get(int64_t slot_start) const {
    auto it = find_(slots_, slot_start);
    if (it == slots_.end() || it->first != slot_start)
      return std::nullopt;
    return it->second;
  }

  // End of the published prices that run on without a gap from the quarter-hour of `now`; that
  // quarter-hour's start when it has no price.
  int64_t known_until(int64_t now) const {
    int64_t t = floor_to_slot(now);
    for (auto it = find_(slots_, t); it != slots_.end() && it->first == t; ++it)
      t += SLOT_SECONDS;
    return t;
  }

  void drop_before(int64_t t) { slots_.erase(slots_.begin(), find_(slots_, t)); }

 private:
  std::vector<std::pair<int64_t, float>> slots_;
};

// ---------------------------------------------------------------------------
// Planner
// ---------------------------------------------------------------------------

constexpr float EFFICIENCY = 0.9f;  // share of the grid energy that reaches the battery
constexpr int BUFFER_SLOTS = 1;     // one slot more than needed, in case charging runs slow

// Indices of the `count` cheapest slots, in time order; of equal prices, the earlier slot.
inline std::vector<int> cheapest_slots(const std::vector<float> &prices, int count) {
  std::vector<int> chosen(prices.size());
  std::iota(chosen.begin(), chosen.end(), 0);
  std::stable_sort(chosen.begin(), chosen.end(), [&prices](int a, int b) { return prices[a] < prices[b]; });
  chosen.resize(std::min(chosen.size(), static_cast<size_t>(std::max(count, 0))));
  std::sort(chosen.begin(), chosen.end());
  return chosen;
}

struct Settings {
  int ready_by_minutes = 7 * 60;  // local time
  int64_t ready_by_once = 0;      // UTC; a one-off deadline used instead of the daily time while it's ahead
  float capacity_kwh = 75.0f;
  float charge_kw = 11.0f;
  int32_t standard_offset = VILNIUS_STANDARD_OFFSET;
  const char *currency = "EUR";  // of the Nord Pool prices and the grid fees
};

struct PlanRequest {
  int64_t now = 0;
  int64_t deadline = 0;
  float soc = NAN;
  float limit = NAN;
  Grid grid;
  Settings settings;  // for capacity_kwh, charge_kw and standard_offset
};

// A run of consecutive chosen slots.
struct Window {
  int64_t start;
  int64_t end;
  float energy_kwh = 0;   // expected from the grid
  float cost_eur = 0;     // what energy_kwh costs
  float avg_price = NAN;  // EUR/kWh: cost_eur / energy_kwh, or its slots' plain average when it takes no energy

  // Holds only the buffer: the car should be full before it.
  bool spare() const { return energy_kwh < 0.05f; }
};

struct Plan {
  bool valid = false;      // false when this quarter-hour has no price
  int needed_slots = 0;    // to reach the limit, plus the buffer
  int horizon_slots = 0;   // from this quarter-hour to the deadline
  int unpriced_slots = 0;  // the last ones before the deadline, whose prices aren't out yet
  std::vector<Window> windows;
  float avg_price = NAN;   // EUR/kWh at total prices (see total_price()) for the energy the car is expected to take
  float soc_at_end = NAN;  // the battery level after the windows, in %

  bool contains(int64_t t) const {
    return std::any_of(windows.begin(), windows.end(), [t](const Window &w) { return w.start <= t && t < w.end; });
  }
};

inline Plan make_plan(const PriceTable &prices, const PlanRequest &request) {
  Plan plan;
  const int64_t known_until = prices.known_until(request.now);
  if (known_until <= request.now)
    return plan;
  plan.valid = true;
  const int64_t priced_end = std::min(request.deadline, known_until);
  const int64_t first = floor_to_slot(request.now);
  std::vector<float> totals;  // total_price() of each slot from `first` to priced_end
  for (int64_t t = first; t + SLOT_SECONDS <= priced_end; t += SLOT_SECONDS)
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access): up to known_until(), every slot has a price
    totals.push_back(total_price(*prices.get(t), t, request.grid, request.settings.standard_offset));
  plan.unpriced_slots = static_cast<int>((request.deadline - priced_end) / SLOT_SECONDS);
  plan.horizon_slots = static_cast<int>(totals.size()) + plan.unpriced_slots;
  const float missing_kwh = (request.limit - request.soc) / 100.0f * request.settings.capacity_kwh;
  const float slot_kwh = request.settings.charge_kw * (SLOT_SECONDS / 3600.0f) * EFFICIENCY;
  plan.needed_slots = missing_kwh > 0.05f ? static_cast<int>(std::ceil(missing_kwh / slot_kwh)) + BUFFER_SLOTS : 0;
  // No guessing: while the quarter-hours without prices could still do all the charging, it waits for
  // their prices; else it buys only what they can't do.
  const int buy = plan.needed_slots - plan.unpriced_slots;
  // The car charges through the chosen slots in turn at charge_kw, from now in the current one, and
  // stops at the limit: the last slot is often only partly used, or spare (the buffer).
  float soc = request.soc, energy_kwh = 0, cost_eur = 0;
  for (int i : cheapest_slots(totals, buy)) {
    const int64_t start = first + i * SLOT_SECONDS;
    const float hours = static_cast<float>(start + SLOT_SECONDS - std::max(start, request.now)) / 3600.0f;
    const float room_kwh = std::max(0.0f, (request.limit - soc) / 100.0f * request.settings.capacity_kwh);
    const float stored_kwh = std::min(request.settings.charge_kw * hours * EFFICIENCY, room_kwh);
    soc += stored_kwh / request.settings.capacity_kwh * 100.0f;
    const float grid_kwh = stored_kwh / EFFICIENCY;
    energy_kwh += grid_kwh;
    cost_eur += grid_kwh * totals[i];
    if (plan.windows.empty() || plan.windows.back().end != start)
      plan.windows.push_back({start, start});
    Window &w = plan.windows.back();
    w.end = start + SLOT_SECONDS;
    w.energy_kwh += grid_kwh;
    w.cost_eur += grid_kwh * totals[i];
  }
  for (Window &w : plan.windows) {
    const auto begin = totals.begin() + (w.start - first) / SLOT_SECONDS;
    const int64_t count = (w.end - w.start) / SLOT_SECONDS;
    w.avg_price = w.energy_kwh > 0 ? w.cost_eur / w.energy_kwh
                                   : std::accumulate(begin, begin + count, 0.0f) / static_cast<float>(count);
  }
  plan.avg_price = cost_eur / energy_kwh;  // NaN (0 / 0) when the car takes nothing
  plan.soc_at_end = soc;
  return plan;
}

// The plan's windows for the web page: "<currency>;<start>,<end>,<price per kWh>[,spare];...", in UTC seconds with each
// window's price (see Window::avg_price), marked spare when the car shouldn't need it. For example
// "EUR;1790463600,1790466300,0.203;1790467200,1790468100,0.211,spare". Just the currency without windows.
inline std::string format_windows(const Plan &plan, const char *currency) {
  std::string text = currency;
  char part[64];
  for (const Window &w : plan.windows) {
    std::snprintf(part, sizeof(part), ";%lld,%lld,%.3f%s", static_cast<long long>(w.start),
                  static_cast<long long>(w.end), w.avg_price, w.spare() ? ",spare" : "");
    text += part;
  }
  return text;
}

// ---------------------------------------------------------------------------
// Controller
// ---------------------------------------------------------------------------

enum class Command { NONE, START_CHARGING, STOP_CHARGING, WAKE };

struct CarState {
  int64_t now = 0;              // UTC seconds; 0 until the clock is synced
  std::optional<bool> plugged;  // unknown until the car reports it (after a restart while it sleeps)
  std::string charging_state;   // "Charging", "Starting", "Stopped", "Complete", ...; empty or "Unknown" when unknown
  float soc = NAN;              // battery %
  float limit = NAN;            // the car's charge limit %
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
  std::string windows;  // format_windows() while the plan decides, else empty
  // "plan", "now" (charging regardless of price), "none" (no plan) or "wait" (no plan possible yet:
  // no clock, plug state, battery level or prices).
  std::string mode;
  std::optional<Notification> notification;  // once per plug-in, when its plan has settled
};

class Controller {
 public:
  PriceTable prices;

  // The buttons on the web page. Each acts at once, free of command_()'s limits.

  // Charge regardless of price until the car is unplugged.
  void charge_now() {
    hold_ = Hold::NOW;
    allow_command_();
  }
  // Follow the plan: cancels charge_now(), a start from the car or app, or stop_charging().
  void create_plan() {
    hold_ = Hold::PLAN;
    allow_command_();
  }
  // No plan and no charging until create_plan(), charge_now(), a start from the car or app, or the car is
  // unplugged.
  void stop_charging() {
    hold_ = Hold::NONE;
    allow_command_();
  }
  // What the buttons chose, for the board to keep across a restart (see Hold).
  int held_mode() const { return static_cast<int>(hold_); }
  void restore_mode(int mode) { hold_ = mode == 1 ? Hold::NOW : mode == 2 ? Hold::NONE : Hold::PLAN; }
  // Recompute the plan on the next tick (new prices or settings).
  void replan() { replan_ = true; }
  // Whether to download prices now, which counts as a try: every 5 minutes while there's no price for this
  // quarter-hour, else hourly until 12:45 CET and every 5 minutes from then, when Nord Pool publishes the
  // next day, until tomorrow's are in. Not before the clock is set: 0 is never past a try.
  bool fetch_prices_due(int64_t now) {
    const int64_t until = prices.known_until(now);
    const int64_t cet_minute = floor_div(now + eu_offset(now, CET_STANDARD_OFFSET), 60) % 1440;
    const int64_t wait = until > now && cet_minute < 12 * 60 + 45 ? 60 * 60 : 5 * 60;
    if (now - prices_tried_at_ < wait || until >= end_of_next_delivery_day(now))
      return false;
    prices_tried_at_ = now;
    return true;
  }
  const Plan &plan() const { return plan_; }
  // The grid fees from config.yaml; until they're set, plans use spot prices only.
  void set_grid(const Grid &grid) {
    grid_ = grid;
    replan_ = true;
  }

  Decision tick(const CarState &car, const Settings &settings) {
    Decision d = decide_(car, settings);
    notify_(car.now, settings, d);
    if (d.mode.empty())
      d.mode = hold_ == Hold::NOW ? "now" : hold_ == Hold::NONE ? "none" : "plan";
    return d;
  }

 private:
  // What the buttons (or a start from the car) chose. It holds until the car is unplugged; a real plug-in
  // also starts afresh, in case the board missed the unplug. The board persists it as an int: keep the values.
  enum class Hold { PLAN = 0, NOW = 1, NONE = 2 };
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
    prices.drop_before(floor_to_slot(now));  // plans start at this quarter-hour
    const bool charging = observe_(car, now);
    if (floor_to_slot(now) != planned_slot_ || replan_)
      update_plan_(now, settings);

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

    bool want_charge = true;  // as usual, without a plan
    if (hold_ == Hold::NOW) {
      d.status = "Charging now";
    } else if (hold_ == Hold::NONE) {
      want_charge = false;
      d.status = "No plan";
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
    } else if (!plan_.valid) {
      if (getting_prices_(now)) {
        d.status = "Getting prices";
        d.mode = "wait";
        return d;
      }
      d.status = "Charging (no prices)";
    } else {
      want_charge = in_plan_();
      // A plan made in a window's last 2 minutes: not worth a start that the next quarter-hour's plan
      // stops, which the command limit delays into that quarter-hour.
      if (!charging && !plan_.contains(planned_slot_ + SLOT_SECONDS) && planned_slot_ + SLOT_SECONDS - now < 2 * 60)
        want_charge = false;
      d.status = want_charge ? "Charging" : next_window_status_(now, settings.standard_offset);
      d.windows = format_windows(plan_, settings.currency);
    }
    const bool full = complete_ || soc_ >= limit_ - 0.5f;  // false while the battery level is unknown (NaN)
    if (want_charge && full && !charging) {
      d.status = "Charged";
      return d;
    }
    d.command = command_(want_charge, charging, now);
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
        replan_ = true;  // a new charge limit changes the plan right away
      if (car.limit > limit_ && complete_)
        limit_raised_at_ = now;  // a finished charge resumes by itself
      limit_ = car.limit;
    }
    if (battery_known_() != battery_was_known)
      replan_ = true;

    // With "Unknown", esphome-tesla-ble also reports the charger as unplugged: that's no unplug.
    const std::optional<bool> plugged = car.charging_state == "Unknown" ? std::nullopt : car.plugged;
    if (plugged.has_value() && *plugged != plugged_) {
      plugged_ = *plugged;
      replan_ = true;
      if (plugged_) {
        plugged_since_ = now;
        // The plug state first seen after a restart is not a plug-in: no message for it, and what the
        // buttons chose before the restart still holds. A real plug-in starts afresh.
        notify_pending_ = plug_state_seen_;
        if (plug_state_seen_)
          hold_ = Hold::PLAN;
      } else {
        hold_ = Hold::PLAN;
        battery_unknown_since_ = 0;
        notify_pending_ = false;
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
      if (started_by_car && plugged_ && (hold_ == Hold::NONE || !in_plan_()))
        hold_ = Hold::NOW;  // from the car or the Tesla app: leave it alone until unplugged
      charging_ = charging;
      complete_ = car.charging_state == "Complete";
    }
    return charging_;  // the last known state: "Unknown" says nothing about it
  }

  // Plans from the current quarter-hour; the plan stands until the next one or replan().
  void update_plan_(int64_t now, const Settings &settings) {
    // A car that says Complete is at its limit, whatever the level reads: it won't take a start.
    const PlanRequest request{now, deadline_(now, settings), complete_ ? limit_ : soc_, limit_, grid_, settings};
    plan_ = battery_known_() ? make_plan(prices, request) : Plan{};
    planned_slot_ = floor_to_slot(now);
    replan_ = false;
    commands_this_plan_ = 0;
  }

  // After a plug-in, one message once its plan has settled: the battery levels, deadline, average price
  // and window count, why nothing was bought, or else the status.
  void notify_(int64_t now, const Settings &settings, Decision &d) {
    if (!notify_pending_)
      return;
    if (hold_ == Hold::NONE) {  // you've already stopped it from the page
      notify_pending_ = false;
      return;
    }
    const bool settled = battery_known_() ? now - plugged_since_ >= 2 * 60 : now - battery_unknown_since_ >= WAKE_FOR;
    const bool waiting_for_prices = plan_.valid && plan_.needed_slots > 0 && plan_.unpriced_slots > 0;
    if (!settled || (!plan_.valid && getting_prices_(now)) || (waiting_for_prices && hold_ != Hold::NOW))
      return;
    notify_pending_ = false;
    Notification &n = d.notification.emplace();
    n.title = "Tesla charging";
    if (hold_ == Hold::NOW || !plan_.valid) {  // no plan without the battery level either
      n.message = d.status;
      return;
    }
    if (plan_.windows.empty()) {
      n.message = plan_.needed_slots == 0 ? "Not needed: battery at limit" : "No time left before Ready by";
      return;
    }
    int windows = 0;
    for (const Window &w : plan_.windows)
      windows += !w.spare();
    char text[128];
    std::snprintf(text, sizeof(text), "%.0f to %.0f%% by %s; avg %.3f %s/kWh over %d window(s)", soc_, limit_,
                  format_day_hhmm(deadline_(now, settings), settings.standard_offset).c_str(), plan_.avg_price,
                  settings.currency, windows);
    n.title = "Tesla charging plan created";
    n.message = text;
    if (plan_.soc_at_end < limit_ - 0.5f)
      n.message += "\nNot enough time to reach the limit";
  }

  // The board's own starts and stops: at most one every 2 minutes, and 3 per plan (each quarter-hour or re-plan).
  Command command_(bool want_charge, bool charging, int64_t now) {
    if (want_charge == charging || now - last_command_at_ < 2 * 60 || commands_this_plan_ >= 3)
      return Command::NONE;
    last_command_at_ = now;
    started_at_ = want_charge ? now : 0;
    ++commands_this_plan_;
    return want_charge ? Command::START_CHARGING : Command::STOP_CHARGING;
  }

  void allow_command_() {
    last_command_at_ = 0;
    commands_this_plan_ = 0;
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

  // The status while the plan doesn't charge; its windows are all ahead then.
  std::string next_window_status_(int64_t now, int32_t standard_offset) const {
    if (!plan_.windows.empty())
      return "Charges at " + format_when(plan_.windows.front().start, now, standard_offset);
    return plan_.needed_slots == 0 ? "Charged" : plan_.unpriced_slots > 0 ? "Waiting for prices" : "Waiting";
  }

  bool battery_known_() const { return !std::isnan(soc_) && !std::isnan(limit_); }

  // Whether the plan charges in the quarter-hour it was made for (always without prices or battery level).
  bool in_plan_() const { return !plan_.valid || plan_.contains(planned_slot_); }

  Plan plan_;
  Grid grid_;
  float soc_ = NAN;
  float limit_ = NAN;
  bool plugged_ = false;
  bool charging_ = false;
  bool complete_ = false;  // the car's own word, which it keeps a percent under the limit
  Hold hold_ = Hold::PLAN;
  bool replan_ = true;
  bool notify_pending_ = false;
  bool plug_state_seen_ = false;
  int64_t planned_slot_ = -1;
  int64_t plugged_since_ = 0;
  int64_t limit_raised_at_ = 0;
  int64_t battery_unknown_since_ = 0;
  int64_t plug_unknown_since_ = 0;
  int64_t first_tick_at_ = 0;  // about when the board started, once the clock is set
  int64_t prices_tried_at_ = 0;
  int64_t last_wake_at_ = 0;
  int64_t last_command_at_ = 0;
  int64_t started_at_ = 0;  // the board's last start, 0 after its stop; the buttons don't reset it
  int commands_this_plan_ = 0;
};

}  // namespace esphome::charging
