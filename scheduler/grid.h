#pragma once
// What a kWh costs on the grid in each quarter-hour: a plan from plans/, and the grid: settings of
// config.yaml over it (format in docs/grid-fees.md). Plain C++17, with nothing from ESPHome, like charger.h.

#include "calendar.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace esphome::scheduler {

// The grid settings as written (format in docs/grid-fees.md): a plan from plans/, or the grid: block of
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

// Reads grid settings in the YAML of plans: two-space indents, `key: value` or `key:` lines, and comments on
// lines of their own, without quotes, flow style or anchors, ending with a line break. Returns what's wrong, or "".
inline std::string read_grid(const std::string &text, GridText &grid) {
  if (!text.empty() && text.back() != '\n')
    return "the text ends inside a line, as if cut off";
  std::string section;
  size_t start = 0;
  for (int number = 1; start < text.size(); number++) {
    const size_t end = text.find('\n', start);
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
      grid.calendar.push_back({key, {}});
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
// A day's rates are letters from 'a', so a grid has 26 at most. Prices are per kWh and below MAX_PRICE. grid.py
// checks the same before the board gets the settings.
constexpr size_t MAX_RATES = 26;
constexpr float MAX_PRICE = 1e6f;

// The grid of a plan's text and config.yaml's grid: settings, both as read_grid() reads them: your calendar and clock
// replace the plan's, and your exceptions and rates replace or add to its own, one key at a time. The plan's prices are
// in `currency`. Returns what's wrong, or "".
inline std::string make_grid(const std::string &plan_text, const std::string &own_text, const std::string &currency,
                             Grid &grid) {
  GridText plan, own;
  if (const std::string error = read_grid(plan_text, plan) + read_grid(own_text, own); !error.empty())
    return error;
  if (!plan.rates.empty() && plan.currency != currency)
    return concat({"the plan's prices are in ", plan.currency, ", not ", currency});
  GridText all = plan;
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
    if (end != price.c_str() + price.size() || !(fee >= 0.0f && fee < MAX_PRICE))
      return concat({"rate ", name, ": ", price, " isn't a price per kWh"});
    if (std::find(names.begin(), names.end(), name) != names.end() || names.size() == MAX_RATES)
      return concat({"rate ", name, ": a rate is there twice, or there are more than 26"});
    names.push_back(name);
    made.fee.push_back(fee);
  }
  std::vector<bool> used(names.size());
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
      if (on.empty())
        return concat({"calendar: ", months, ": ", days, " isn't a day or a range like mon-fri"});
      std::string rates;
      if (const std::string error = day_rates(line, names, rates, used); !error.empty())
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
    if (const std::string error = day_rates(line, names, rates, used); !error.empty())
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

// Whether to download the plan at `now`: daily after a try that brought a plan the board can use, the one in use or a
// new one, and hourly after any other. Not before the clock is set: 0 is never past a try.
inline bool plan_due(int64_t now, int64_t tried_at, bool usable) {
  return now - tried_at >= (usable ? DAY_SECONDS : 3600);
}

}  // namespace esphome::scheduler
