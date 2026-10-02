#pragma once
// Charges the Tesla in the cheapest quarter-hours before Ready by.
//
// A plan takes the cheapest slots, priced at Nord Pool's spot price plus VAT and the grid fee, that
// bring the battery to the car's charge limit, plus a buffer slot. Every 30 s ChargingComponent
// (charging_component.h) passes the car's state to Controller::tick(), which re-plans when needed, counts
// what charging cost and saved, and returns what to do.
//
// Plain C++17 plus ArduinoJson, with nothing from ESPHome, so it can be unit-tested on a computer.

#include <ArduinoJson.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
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

// The grid settings as written (format in docs/grid-fees.md): a price list from pricelists/, or the grid: block of
// config.yaml, which the build writes out the same way. Keys and lines stay text, in the order written.
struct GridText {
  std::string clock;
  std::string currency;
  std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> calendar;
  std::vector<std::pair<std::string, std::string>> exceptions;
  std::vector<std::pair<std::string, std::string>> rates;
};

// The pieces, in one string.
inline std::string concat(std::initializer_list<std::string_view> pieces) {
  std::string joined;
  for (const std::string_view piece : pieces)
    joined += piece;
  return joined;
}

// Reads grid settings in the YAML of price lists: two-space indents, `key: value` or `key:` lines, and comments on
// lines of their own, without quotes, flow style or anchors. Returns what's wrong, or "".
inline std::string read_grid(const std::string &text, GridText &grid) {
  std::string section;
  size_t start = 0;
  for (int number = 1; start < text.size(); number++) {
    const size_t end = std::min(text.find('\n', start), text.size());
    std::string line = text.substr(start, end - start);
    start = end + 1;
    line.erase(line.find_last_not_of(" \r") + 1);
    const size_t indent = line.find_first_not_of(' ');
    if (indent == std::string::npos || line[indent] == '#')
      continue;
    const size_t colon = line.find(':');
    if (colon == std::string::npos || colon == indent || (colon + 1 < line.size() && line[colon + 1] != ' '))
      return concat({"line ", std::to_string(number), " isn't a key and a value"});
    const std::string key = line.substr(indent, colon - indent);
    std::string value = line.substr(colon + 1);
    value.erase(0, value.find_first_not_of(' '));
    if (indent == 0 && value.empty() && (key == "calendar" || key == "exceptions" || key == "rates")) {
      section = key;
    } else if (indent == 0 && !value.empty() && (key == "clock" || key == "currency")) {
      section.clear();
      (key == "clock" ? grid.clock : grid.currency) = value;
    } else if (indent == 2 && value.empty() && section == "calendar") {
      grid.calendar.emplace_back(key, std::vector<std::pair<std::string, std::string>>{});
    } else if (indent == 4 && !value.empty() && section == "calendar" && !grid.calendar.empty()) {
      grid.calendar.back().second.emplace_back(key, value);
    } else if (indent == 2 && !value.empty() && (section == "exceptions" || section == "rates")) {
      (section == "rates" ? grid.rates : grid.exceptions).emplace_back(key, value);
    } else {
      return concat({"line ", std::to_string(number), " doesn't belong there: ", key});
    }
  }
  return "";
}

// The number in two digits of `s` at `at`, or -1.
inline int two_digits(const std::string &s, size_t at) {
  const auto digit = [&](size_t i) { return i < s.size() && s[i] >= '0' && s[i] <= '9'; };
  return digit(at) && digit(at + 1) ? (s[at] - '0') * 10 + s[at + 1] - '0' : -1;
}

// The indexes that `key` names among `names`: one name, or a range like "fri-mon" or "nov-mar", which may wrap.
// Empty when it's neither.
template <size_t N>
inline std::vector<size_t> named(const std::string &key, const char *const (&names)[N]) {
  const size_t dash = key.find('-');
  const auto index = [&](const std::string &name) {
    return static_cast<size_t>(std::find(names, names + N, name) - names);
  };
  const size_t first = index(key.substr(0, dash));
  const size_t last = dash == std::string::npos ? first : index(key.substr(dash + 1));
  std::vector<size_t> found;
  for (size_t i = first; first < N && last < N && (found.empty() || found.back() != last); i = (i + 1) % N)
    found.push_back(i);
  return found;
}

// A day's rates as a letter for each quarter-hour, 'a' for the first of `names`, from a line like
// "night 07:00 day 23:00 night": the rate from midnight, then each time it changes, on a quarter-hour, and the rate
// from then, onto `day`, which starts empty. Marks the rates used. Returns what's wrong, or "".
inline std::string day_rates(const std::string &line, const std::vector<std::string> &names, std::string &day,
                             std::vector<bool> &used) {
  std::vector<std::string> words;
  for (size_t start = 0, end; start < line.size(); start = end + 1) {
    end = std::min(line.find(' ', start), line.size());
    if (end > start)
      words.push_back(line.substr(start, end - start));
  }
  if (words.size() % 2 == 0)
    return concat({"\"", line, "\" isn't rates and times by turns, like night 07:00 day"});
  for (size_t i = 0; i < words.size(); i += 2) {
    const auto rate = static_cast<size_t>(std::find(names.begin(), names.end(), words[i]) - names.begin());
    if (rate == names.size())
      return concat({"rate ", words[i], " has no price"});
    used[rate] = true;
    size_t until = 96;
    if (i + 1 < words.size()) {
      const std::string &time = words[i + 1];
      const int hour = two_digits(time, 0), minute = two_digits(time, 3);
      const bool quarter =
          time.size() == 5 && time[2] == ':' && hour >= 0 && minute >= 0 && minute <= 45 && minute % 15 == 0;
      until = quarter ? static_cast<size_t>(hour) * 4 + static_cast<size_t>(minute) / 15 : 0;
      if (until <= day.size() || until >= 96)
        return concat({time, " isn't a later quarter-hour, like 07:00 or 22:15"});
    }
    day.append(until - day.size(), static_cast<char>('a' + rate));
  }
  return "";
}

// What a kWh costs on the grid: the VAT on market prices, and the grid's rates for each quarter-hour, from
// make_grid(). The times are local time, or winter time all year with clock: winter. Without a calendar, no grid
// fees: market prices only.
struct Grid {
  float vat = 0.0f;
  bool winter_clock = false;
  std::vector<std::array<std::string, 7>> weeks;        // the calendar: a day's rates by day of the week from Sunday
  std::array<uint8_t, 12> week_of_month{};              // which of the weeks each month has, from January
  std::vector<std::pair<int, std::string>> exceptions;  // month * 100 + day, and that day's rates
  std::vector<float> fee;                               // per kWh with VAT, by letter from 'a'
};

constexpr const char *DAY_NAMES[] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};
constexpr const char *MONTH_NAMES[] = {"jan", "feb", "mar", "apr", "may", "jun",
                                       "jul", "aug", "sep", "oct", "nov", "dec"};

// The grid of a price list's settings and config.yaml's own: your calendar and clock replace the list's, and your
// exceptions and rates replace or add to its own, one key at a time. The list's prices are in `currency`. Returns
// what's wrong, or "".
inline std::string make_grid(const GridText &list, const GridText &own, const std::string &currency, Grid &grid) {
  if (!list.rates.empty() && list.currency != currency)
    return concat({"the price list's prices are in ", list.currency, ", not ", currency});
  GridText all = list;
  if (!own.clock.empty())
    all.clock = own.clock;
  if (!own.calendar.empty())
    all.calendar = own.calendar;
  const auto set = [](auto &table, const auto &entry) {
    const auto found =
        std::find_if(table.begin(), table.end(), [&](const auto &other) { return other.first == entry.first; });
    if (found == table.end())
      table.push_back(entry);
    else
      found->second = entry.second;
  };
  for (const auto &entry : own.exceptions)
    set(all.exceptions, entry);
  for (const auto &entry : own.rates)
    set(all.rates, entry);

  Grid made;
  if (!all.clock.empty() && all.clock != "local" && all.clock != "winter")
    return concat({"clock is local or winter, not ", all.clock});
  made.winter_clock = all.clock == "winter";
  std::vector<std::string> names;
  for (const auto &[name, price] : all.rates) {
    char *end = nullptr;
    const float fee = std::strtof(price.c_str(), &end);
    if (end != price.c_str() + price.size() || !(fee >= 0.0f && fee < 1e6f))
      return concat({"rate ", name, ": ", price, " isn't a price per kWh"});
    if (std::find(names.begin(), names.end(), name) != names.end() || names.size() == 26)
      return concat({"rate ", name, ": a rate is there twice, or there are more than 26"});
    names.push_back(name);
    made.fee.push_back(fee);
  }
  std::vector<bool> used(names.size());
  std::string error;
  unsigned months_seen = 0;
  for (const auto &[months, week] : all.calendar) {
    const std::vector<size_t> in = named(months, MONTH_NAMES);
    if (in.empty())
      return concat({"calendar: ", months, " isn't a month or a range like nov-mar"});
    unsigned days_seen = 0;
    made.weeks.emplace_back();
    for (const size_t month : in) {
      if (months_seen & 1U << month)
        return concat({"calendar: ", months, " has a month that's in the calendar already"});
      months_seen |= 1U << month;
      made.week_of_month[month] = static_cast<uint8_t>(made.weeks.size() - 1);
    }
    for (const auto &[days, line] : week) {
      const std::vector<size_t> on = named(days, DAY_NAMES);
      std::string rates;
      if (on.empty())
        return concat({"calendar: ", months, ": ", days, " isn't a day or a range like mon-fri"});
      error = day_rates(line, names, rates, used);
      if (!error.empty())
        return concat({"calendar: ", months, ": ", days, ": ", error});
      for (const size_t day : on) {
        if (days_seen & 1U << day)
          return concat({"calendar: ", months, ": ", days, " has a day that's there already"});
        days_seen |= 1U << day;
        made.weeks.back()[day] = rates;
      }
    }
    if (days_seen != 0x7FU)
      return concat({"calendar: ", months, " needs every day of the week"});
  }
  if (months_seen != 0xFFFU)
    return "the calendar needs every month";
  for (const auto &[date, line] : all.exceptions) {
    static constexpr int LAST_DAY[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const int month = two_digits(date, 0), day = two_digits(date, 3);
    const int month_day = month * 100 + day;
    std::string rates;
    if (date.size() != 5 || date[2] != '-' || month < 1 || month > 12 || day < 1 || day > LAST_DAY[month - 1] ||
        std::any_of(made.exceptions.begin(), made.exceptions.end(),
                    [&](const auto &other) { return other.first == month_day; }))
      return concat({"exceptions: ", date, " isn't a date like 12-25, or it's there twice"});
    error = day_rates(line, names, rates, used);
    if (!error.empty())
      return concat({"exceptions: ", date, ": ", error});
    made.exceptions.emplace_back(month_day, rates);
  }
  for (const auto &[name, price] : own.rates)
    if (!used[std::find(names.begin(), names.end(), name) - names.begin()])
      return concat({"rate ", name, " isn't used on any day"});
  grid = made;
  return "";
}

// The grid fee of the quarter-hour at `utc`: the exception's rates on its date, else the calendar's for the month
// and the day of the week.
inline float grid_fee(int64_t utc, const Grid &grid, int32_t standard_offset) {
  if (grid.weeks.empty())
    return 0.0f;
  const int64_t local = utc + (grid.winter_clock ? standard_offset : eu_offset(utc, standard_offset));
  const int64_t local_day = floor_div(local, DAY_SECONDS);
  const CivilDate date = civil_from_days(local_day);
  const auto fee_of = [&](const std::string &rates) {
    return grid.fee[rates[(local - local_day * DAY_SECONDS) / SLOT_SECONDS] - 'a'];
  };
  for (const auto &[month_day, rates] : grid.exceptions)
    if (month_day == static_cast<int>(date.month * 100 + date.day))
      return fee_of(rates);
  return fee_of(grid.weeks[grid.week_of_month[date.month - 1]][weekday(local_day)]);
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

// Start (UTC) of the CET delivery day containing `now`, its end, and the end of the one after it.
inline int64_t start_of_delivery_day(int64_t now) {
  return local_to_utc(local_day_of(now, CET_STANDARD_OFFSET), 0, CET_STANDARD_OFFSET);
}
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

  // Calls f(slot_start, price) for each price from `from` to `to`.
  template <typename F>
  void for_each(int64_t from, int64_t to, F f) const {
    for (auto it = find_(slots_, from); it != slots_.end() && it->first < to; ++it)
      f(it->first, it->second);
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
    // Up to known_until(), every slot has a price.
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access,clang-analyzer-core.CallAndMessage)
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
// Savings
// ---------------------------------------------------------------------------

// The energy the car took from the grid on one local day, what it cost, and what it would have cost at the
// delivery day's average total price and charging at once (see Controller::count_()). Money in hundredths of the
// currency, rounded from the day's sum rather than from each tick.
struct SavingsDay {
  uint32_t wh = 0;
  int32_t paid = 0;
  int32_t average = 0;
  int32_t at_once = 0;

  void add(const SavingsDay &other) {
    wh += other.wh;
    paid += other.paid;
    average += other.average;
    at_once += other.at_once;
  }
};

constexpr int SAVINGS_DAYS = 365;

// What the board keeps in flash: the last 365 local days. ESPHome saves it byte for byte, and loads it only into a
// struct of the same size.
struct Savings {
  char currency[4] = {};          // of the money; another one, or none, starts the figures afresh
  int32_t day = 0;                // the last day in `days`, as a local day number
  SavingsDay days[SAVINGS_DAYS];  // by ring_index() of the local day number
};

inline int ring_index(int64_t day) { return static_cast<int>(day - floor_div(day, SAVINGS_DAYS) * SAVINGS_DAYS); }

// The savings for the web page: "<currency>;<last 30 days>;<last 365 days>", each period as
// "<Wh>,<paid>,<at the day's average>,<at once>" with the money in hundredths. For example
// "EUR;9200,103,147,190;9200,103,147,190".
inline std::string format_savings(const Savings &savings) {
  SavingsDay periods[2];
  for (int i = 0; i < SAVINGS_DAYS; ++i) {
    const SavingsDay &day = savings.days[ring_index(savings.day - i)];
    if (i < 30)
      periods[0].add(day);
    periods[1].add(day);
  }
  std::string text = savings.currency;
  char part[64];
  for (const SavingsDay &period : periods) {
    std::snprintf(part, sizeof(part), ";%u,%d,%d,%d", static_cast<unsigned>(period.wh), static_cast<int>(period.paid),
                  static_cast<int>(period.average), static_cast<int>(period.at_once));
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
  std::string windows;  // format_windows() while the plan decides, else empty
  // "plan", "now" (charging regardless of price), "none" (no plan) or "wait" (no plan possible yet:
  // no clock, plug state, battery level or prices).
  std::string mode;
  std::optional<Notification> notification;  // once per plug-in, when its plan has settled, and once more
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
  // The page's Reset savings: the figures start afresh on the next tick.
  void reset_savings() { savings = Savings{}; }
  // What the buttons chose, for the board to keep across a restart (see Hold).
  int held_mode() const { return static_cast<int>(hold_); }
  void restore_mode(int mode) { hold_ = mode == 1 ? Hold::NOW : mode == 2 ? Hold::NONE : Hold::PLAN; }
  // Recompute the plan on the next tick (new prices or settings).
  void replan() { replan_ = true; }
  // Whether to download prices now, which counts as a try: every 5 minutes while there's no price for this
  // quarter-hour, else hourly until 12:45 CET and every 5 minutes from then, when Nord Pool publishes the
  // next day, until tomorrow's are in. Never without market prices, and not before the clock is set: 0 is never past a
  // try.
  bool fetch_prices_due(int64_t now) {
    const int64_t until = prices.known_until(now);
    const int64_t cet_minute = floor_div(now + eu_offset(now, CET_STANDARD_OFFSET), 60) % 1440;
    const int64_t wait = until > now && cet_minute < 12 * 60 + 45 ? 60 * 60 : 5 * 60;
    if (!market_ || now - prices_tried_at_ < wait || until >= end_of_next_delivery_day(now))
      return false;
    prices_tried_at_ = now;
    return true;
  }
  const Plan &plan() const { return plan_; }
  // Without market prices to download, every quarter-hour's spot price is 0: the grid fees are the whole price.
  void without_market_prices() { market_ = false; }
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
    if (!market_)  // as far ahead as Nord Pool's prices go, so the page offers the same times for Ready by
      for (int64_t t = start_of_delivery_day(now); t < end_of_next_delivery_day(now); t += SLOT_SECONDS)
        prices.set(t, 0.0f);
    const bool charging = observe_(car, now);
    count_(car, now, settings, d);
    prices.drop_before(start_of_delivery_day(now));  // after count_(), which may need the day before
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
    bool fallback = false;    // at any price, for lack of prices or a battery level
    bool starting = false;    // the status is about the start, until the car charges
    if (hold_ == Hold::NOW) {
      d.status = "Charging now";
      starting = true;
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
      fallback = true;
    } else if (!plan_.valid) {
      if (getting_prices_(now)) {
        d.status = "Getting prices";
        d.mode = "wait";
        return d;
      }
      d.status = "Charging (no prices)";
      fallback = true;
    } else {
      want_charge = in_plan_();
      // A plan made in a window's last 2 minutes: not worth a start that the next quarter-hour's plan
      // stops, which the command limit delays into that quarter-hour.
      if (!charging && !plan_.contains(planned_slot_ + SLOT_SECONDS) && planned_slot_ + SLOT_SECONDS - now < 2 * 60)
        want_charge = false;
      d.status = want_charge ? "Charging" : next_window_status_(now, settings.standard_offset);
      d.windows = format_windows(plan_, settings.currency);
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
      d.status = commands_this_plan_ >= 3 && now - last_command_at_ >= 2 * 60 ? "Can't start charging" : "Starting";
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
      fallback_told_ = false;
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
      if (started_by_car && plugged_ && (hold_ == Hold::NONE || !in_plan_())) {
        notify_car_start_ = hold_ != Hold::NOW;  // once per hold
        hold_ = Hold::NOW;                       // from the car or the Tesla app: leave it alone until unplugged
      }
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

  // After a plug-in, one message once its plan has settled: the battery levels, deadline, average price
  // and window count, why nothing was bought, or else the status. A start from the car or the Tesla app
  // outside the plan sends one at once, as the board then charges at any price.
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

  // The status while the plan doesn't charge; its windows are all ahead then.
  std::string next_window_status_(int64_t now, int32_t standard_offset) const {
    if (!plan_.windows.empty())
      return "Charges at " + format_when(plan_.windows.front().start, now, standard_offset);
    return plan_.needed_slots == 0 ? "Charged" : plan_.unpriced_slots > 0 ? "Waiting for prices" : "Waiting";
  }

  bool battery_known_() const { return !std::isnan(soc_) && !std::isnan(limit_); }

  // At the limit by the car's word, or within half a percent; false while the battery level is unknown (NaN).
  bool full_() const { return complete_ || soc_ >= limit_ - 0.5f; }

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
  bool fallback_told_ = false;
  bool notify_car_start_ = false;
  bool plug_state_seen_ = false;
  bool market_ = true;  // prices come from Nord Pool (see without_market_prices())
  int64_t planned_slot_ = -1;
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
  int commands_this_plan_ = 0;

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

}  // namespace esphome::charging
