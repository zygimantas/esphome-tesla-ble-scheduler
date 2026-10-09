#pragma once
// The board's settings: the file its page writes, read and checked before the board takes it. Plain C++17, with
// nothing from ESPHome, like charger.h.

#include "market.h"
#include "tariff.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace esphome::scheduler {

// The most a settings file can have.
constexpr size_t MAX_SETTINGS_BYTES = 4096;

// The plans built in, by name like lt/eso-standartinis-4-zones, with their text.
using Plans = std::vector<std::pair<std::string_view, std::string_view>>;

// The time zones of the market countries, which the page writes, with their offset from UTC in winter, in hours: their
// clocks all change on the EU's dates, as calendar.h has them.
constexpr std::pair<const char *, int> TIME_ZONES[] = {
    {"Africa/Ceuta", 1},     {"Atlantic/Canary", 0},   {"Atlantic/Madeira", 0},  {"Europe/Amsterdam", 1},
    {"Europe/Berlin", 1},    {"Europe/Bratislava", 1}, {"Europe/Brussels", 1},   {"Europe/Bucharest", 2},
    {"Europe/Budapest", 1},  {"Europe/Busingen", 1},   {"Europe/Copenhagen", 1}, {"Europe/Dublin", 0},
    {"Europe/Helsinki", 2},  {"Europe/Lisbon", 0},     {"Europe/Ljubljana", 1},  {"Europe/Luxembourg", 1},
    {"Europe/Madrid", 1},    {"Europe/Mariehamn", 2},  {"Europe/Oslo", 1},       {"Europe/Paris", 1},
    {"Europe/Prague", 1},    {"Europe/Riga", 2},       {"Europe/Rome", 1},       {"Europe/Sofia", 2},
    {"Europe/Stockholm", 1}, {"Europe/Tallinn", 2},    {"Europe/Vienna", 1},     {"Europe/Vilnius", 2},
    {"Europe/Warsaw", 1},    {"Europe/Zagreb", 1},     {"Europe/Zurich", 1},
};

struct SettingsFile {
  std::string currency;        // of the market prices and the tariff
  const Area *area = nullptr;  // null without market prices
  float vat = 0.0f;            // on the market prices
  float margin = 0.0f;         // the supplier's, per kWh with VAT: on top of the market's, or its fixed part
  std::string ntfy_topic;      // empty: no phone messages
  std::string plan;            // a plan from the list, by its path; empty for a custom plan or none
  std::string_view plan_text;  // the plan as built in
  std::string custom_plan;     // a custom plan's text instead, as read_tariff() reads it: its lines count from 1
  float battery_kwh = 0.0f;
  float charging_kw = 0.0f;
  bool unlock_when_charged = false;  // unlock the charge port once the car finishes charging
  std::string vin;
  int32_t standard_offset = 0;  // the time zone's, in seconds
};

// `text` in capitals.
inline std::string upper(std::string text) {
  for (char &c : text)
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  return text;
}

// The settings file as written, before the checks: each setting by its place, the places the file has, grid: and
// market: too, and a custom plan's lines under grid: plan: as a plan's text.
struct SettingsText {
  std::string area, battery, charging, currency, fixed, margin, ntfy_topic, plan, unlock, vat, vin, zone, custom_plan;
  std::vector<std::string> seen;
};

// Whether the file has a line at `place`.
inline bool has(const SettingsText &written, std::string_view place) {
  return std::find(written.seen.begin(), written.seen.end(), place) != written.seen.end();
}

// Reads the file's lines into `written`: each setting at its place, once, and a custom plan's lines, indented by four
// spaces under grid: plan:, as a plan's text. Returns what's wrong, or "".
inline std::string read_lines(const std::string &text, SettingsText &written) {
  // Each setting by its place in the file, and where its value goes; grid: and market: head lines of their own.
  const std::pair<const char *, std::string *> places[] = {{"currency", &written.currency},
                                                           {"fixed_price", &written.fixed},
                                                           {"grid", nullptr},
                                                           {"grid: plan", &written.plan},
                                                           {"market", nullptr},
                                                           {"market: area", &written.area},
                                                           {"market: margin", &written.margin},
                                                           {"market: vat", &written.vat},
                                                           {"ntfy_topic", &written.ntfy_topic},
                                                           {"tesla_battery_kwh", &written.battery},
                                                           {"tesla_charging_kw", &written.charging},
                                                           {"tesla_unlock_when_charged", &written.unlock},
                                                           {"tesla_vin", &written.vin},
                                                           {"timezone", &written.zone}};
  std::string section;
  bool custom = false;  // in a custom plan's lines, under grid: plan:
  size_t start = 0;
  for (int number = 1; start < text.size(); number++) {
    const std::string line = next_line(text, start);
    const size_t indent = line.find_first_not_of(' ');
    const auto at = [&](std::string_view what) { return concat({"line ", std::to_string(number), " ", what}); };
    if (indent == std::string::npos || line[indent] == '#') {
      if (custom)
        written.custom_plan += "\n";
      continue;
    }
    // A custom plan's lines, which read_tariff() checks with the plan file's own line numbers.
    if (custom && indent >= 4) {
      written.custom_plan += line.substr(4) + "\n";
      continue;
    }
    std::string key, value;
    if (const std::string error = key_value(line, indent, key, value); !error.empty())
      return at(error);
    const bool heading = value.empty();  // a key with lines of its own below
    std::string place;
    if (indent == 0) {
      section = heading ? key : "";
      place = key;
    } else if (indent == 2 && ((section == "market" && !heading) || section == "grid")) {
      place = concat({section, ": ", key});
    } else {
      return at(concat({"doesn't belong there: ", key}));
    }
    const auto *found =
        std::find_if(std::begin(places), std::end(places), [&](const auto &known) { return place == known.first; });
    if (found == std::end(places))
      return at(concat({"has ", place, ", which isn't a setting"}));
    if (has(written, place))
      return at(concat({"has ", place, " again"}));
    written.seen.push_back(place);
    if (found->second != nullptr)
      *found->second = value;
    custom = place == "grid: plan" && heading;
  }
  return "";
}

// The market prices, where market: is there: their area, the VAT on them, and the supplier's margin, 0 when left out.
// Returns what's wrong, or "".
inline std::string check_market(const SettingsText &written, SettingsFile &read) {
  if (!has(written, "market"))
    return "";
  for (const Area &known : AREAS)
    if (upper(written.area) == known.name)
      read.area = &known;
  if (read.area == nullptr)
    return "market: area must be one the board knows, like LT or SE3: see Countries and plans";
  read.vat = number(written.vat);
  if (!(read.vat >= 0.0f && read.vat < 1.0f))
    return "market: vat must be the VAT as a fraction, like 0.21 for 21%";
  read.margin = written.margin.empty() ? 0.0f : number(written.margin);
  if (!(read.margin >= 0.0f && read.margin < MAX_PRICE))
    return "market: margin must be a price per kWh, like 0.012";
  return "";
}

// A fixed price's supplier part, without the grid fees, which the board adds to every quarter-hour like a margin.
// Returns what's wrong, or "".
inline std::string check_fixed_price(const SettingsText &written, SettingsFile &read) {
  if (written.fixed.empty())
    return "";
  if (read.area != nullptr)
    return "fixed_price goes without market:; on top of a market price, use market: margin";
  read.margin = number(written.fixed);
  if (!(read.margin >= 0.0f && read.margin < MAX_PRICE))
    return "fixed_price must be a price per kWh, like 0.15";
  return "";
}

// The currency: the file's, else the market's own, else euros; with a market, one its prices come in. Returns what's
// wrong, or "".
inline std::string check_currency(const SettingsText &written, SettingsFile &read) {
  const char *own = read.area != nullptr ? own_currency(*read.area) : "EUR";
  read.currency = upper(written.currency.empty() ? own : written.currency);
  if (read.currency.size() != 3 ||
      !std::all_of(read.currency.begin(), read.currency.end(), [](char c) { return c >= 'A' && c <= 'Z'; }))
    return "currency must be a currency's three-letter code, like EUR";
  if (read.area == nullptr)
    return "";
  const char *converted = converted_currency(*read.area);
  if (read.currency != own && (converted == nullptr || read.currency != converted))
    return concat({"currency: ", market_name(read.area->market), "'s prices for ", read.area->name, " come in ", own,
                   converted != nullptr ? concat({", or ", converted, " at the ECB's daily rate"}) : ""});
  return "";
}

// The ntfy topic the phone messages go to, where the file has one. Returns what's wrong, or "".
inline std::string check_topic(const SettingsText &written, SettingsFile &read) {
  read.ntfy_topic = written.ntfy_topic;
  if (read.ntfy_topic.size() > 64 || !std::all_of(read.ntfy_topic.begin(), read.ntfy_topic.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_';
      }))
    return "ntfy_topic must be the topic's name, not its address: up to 64 letters, digits, - and _";
  return "";
}

// The grid plan under grid: plan:, one built in, by name, or a custom plan's lines, as a tariff in the settings'
// currency. Without market: and grid:, the board has no prices, and the car charges as usual. A grid: with neither a
// plan nor a calendar under it, like one with the plan's name on its own line, would add nothing. Returns what's wrong,
// or "".
inline std::string check_plan(const SettingsText &written, const Plans &plans, SettingsFile &read) {
  read.plan = written.plan;
  read.custom_plan = written.custom_plan;
  if (!read.plan.empty()) {
    const auto found =
        std::find_if(plans.begin(), plans.end(), [&](const auto &known) { return known.first == read.plan; });
    if (found == plans.end())
      return concat({"grid: there's no plan ", read.plan});
    read.plan_text = found->second;
  }
  TariffText custom_text;
  if (read.plan_text.empty() && read_tariff(read.custom_plan, custom_text).empty() && custom_text.calendar.empty() &&
      has(written, "grid"))
    // nothing under grid: names no plan; a custom plan that doesn't read gets make_tariff()'s error
    return read.custom_plan.empty() ? "grid: there's no plan" : "custom plan: there's no calendar";
  Tariff tariff;
  if (const std::string error =
          make_tariff(read.plan.empty() ? read.custom_plan : std::string(read.plan_text), read.currency, tariff);
      !error.empty())
    return concat({read.plan.empty() ? "custom plan: " : "grid: ", error});
  return "";
}

// The car, all three or none yet, as the setup saves the prices before the car's step, and whether to unlock the charge
// port once it's charged. Wider than the page's 20 to 200 kWh and 1 to 22 kW, and closed, so a schedule's numbers stay
// in range. Returns what's wrong, or "".
inline std::string check_car(const SettingsText &written, SettingsFile &read) {
  read.vin = written.vin;
  if (!written.battery.empty() || !written.charging.empty() || !read.vin.empty()) {
    read.battery_kwh = number(written.battery);
    if (!(read.battery_kwh >= 10.0f && read.battery_kwh <= 1000.0f))
      return "tesla_battery_kwh must be the battery's size in kWh, like 75";
    read.charging_kw = number(written.charging);
    if (!(read.charging_kw >= 0.5f && read.charging_kw <= 100.0f))
      return "tesla_charging_kw must be the charging power in kW, like 11";
    if (read.vin.size() != 17 || !std::all_of(read.vin.begin(), read.vin.end(), [](char c) {
          return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z' && c != 'I' && c != 'O' && c != 'Q');
        }))
      return "tesla_vin must be the car's VIN: 17 capital letters and digits, none of them I, O or Q, on the car's "
             "screen under Controls, Software";
  }
  if (!written.unlock.empty() && written.unlock != "true")
    return "tesla_unlock_when_charged must be true, or left out";
  read.unlock_when_charged = !written.unlock.empty();
  return "";
}

// The time zone, one of TIME_ZONES, for its offset. Returns what's wrong, or "".
inline std::string check_time_zone(const SettingsText &written, SettingsFile &read) {
  const auto *found = std::find_if(std::begin(TIME_ZONES), std::end(TIME_ZONES),
                                   [&](const auto &known) { return written.zone == known.first; });
  if (found == std::end(TIME_ZONES))
    return "timezone must be one the board knows, like Europe/Vilnius";
  read.standard_offset = found->second * 3600;
  return "";
}

// Reads and checks the settings file, in the plans' YAML (tariff.h) as the page writes it, the last line break left out
// or not, without quotes or comments after a value. grid: plan: names a plan built in, or holds a custom plan: a
// plan's file (plans/README.md), each line indented by four spaces, whose line numbers count from the line after
// plan:, as in the file. The checks run in turn, each on what the ones before it read. Returns what's wrong, or "".
inline std::string read_settings(const std::string &text, const Plans &plans, SettingsFile &settings) {
  if (text.size() > MAX_SETTINGS_BYTES)
    return "the file is longer than 4 kB";
  SettingsText written;
  SettingsFile read;
  std::string error = read_lines(text, written);
  if (error.empty())
    error = check_market(written, read);
  if (error.empty())
    error = check_fixed_price(written, read);
  if (error.empty())
    error = check_currency(written, read);
  if (error.empty())
    error = check_topic(written, read);
  if (error.empty())
    error = check_plan(written, plans, read);
  if (error.empty())
    error = check_car(written, read);
  if (error.empty())
    error = check_time_zone(written, read);
  if (error.empty())
    settings = read;
  return error;
}

// Whether saving `now` over `was` restarts a board that has a car: for another car, whose Bluetooth link and key are
// its own, or prices of another market area or in another currency, as those downloaded already are the old ones. The
// rest applies at once, as all of it does while the board has no car.
inline bool restarts(const SettingsFile &was, const SettingsFile &now) {
  return !was.vin.empty() && (now.vin != was.vin || now.area != was.area || now.currency != was.currency);
}

// Whether saving `now` over `was` deletes the schedule on a board that has a car, as Delete schedule does: for any
// change but the ntfy topic's or the unlock box's, as the schedule was made with the rest, while the topic only says
// where its message goes and the box what follows the charge.
inline bool deletes_schedule(const SettingsFile &was, const SettingsFile &now) {
  const auto rest = [](const SettingsFile &s) {
    return std::tie(s.currency, s.area, s.vat, s.margin, s.plan, s.custom_plan, s.battery_kwh, s.charging_kw, s.vin,
                    s.standard_offset);
  };
  return !was.vin.empty() && rest(was) != rest(now);
}

// `text` as a JSON string.
inline std::string json_string(std::string_view text) {
  std::string json = "\"";
  for (const char c : text) {
    if (c == '"' || c == '\\')
      json += '\\';
    json += c;
  }
  return json + "\"";
}

// What the page offers for the settings, as JSON: the market areas, and the plans built in by name and name for people.
// Calls f() with each piece in turn, as the board sends them without holding all of them.
template <typename F>
void settings_options(const Plans &plans, F f) {
  f("{\"areas\":[");
  const char *comma = "";
  for (const Area &area : AREAS) {
    f(concat({comma, json_string(area.name)}));
    comma = ",";
  }
  f("],\"plans\":[");
  comma = "";
  for (const auto &plan : plans) {
    TariffText text;
    read_tariff(std::string(plan.second), text);  // the plans built in read, as the tests check
    f(concat({comma, "[", json_string(plan.first), ",", json_string(text.name.empty() ? plan.first : text.name), "]"}));
    comma = ",";
  }
  f("]}");
}

}  // namespace esphome::scheduler
