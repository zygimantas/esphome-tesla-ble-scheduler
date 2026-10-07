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
    {"Africa/Ceuta", 1},     {"Atlantic/Canary", 0},   {"Atlantic/Madeira", 0}, {"Europe/Amsterdam", 1},
    {"Europe/Berlin", 1},    {"Europe/Brussels", 1},   {"Europe/Bucharest", 2}, {"Europe/Budapest", 1},
    {"Europe/Busingen", 1},  {"Europe/Copenhagen", 1}, {"Europe/Helsinki", 2},  {"Europe/Lisbon", 0},
    {"Europe/Ljubljana", 1}, {"Europe/Luxembourg", 1}, {"Europe/Madrid", 1},    {"Europe/Mariehamn", 2},
    {"Europe/Oslo", 1},      {"Europe/Paris", 1},      {"Europe/Prague", 1},    {"Europe/Riga", 2},
    {"Europe/Rome", 1},      {"Europe/Sofia", 2},      {"Europe/Stockholm", 1}, {"Europe/Tallinn", 2},
    {"Europe/Vienna", 1},    {"Europe/Vilnius", 2},    {"Europe/Warsaw", 1},    {"Europe/Zagreb", 1},
    {"Europe/Zurich", 1},
};

struct SettingsFile {
  std::string currency;        // of the market prices and the tariff
  const Area *area = nullptr;  // null without market prices
  float vat = 0.0f;            // on the market prices
  float margin = 0.0f;         // the supplier's, per kWh with VAT: on top of the market's, or its fixed part
  std::string ntfy_topic;      // empty: no phone messages
  std::string plan;            // the plan's name, empty without one
  std::string_view plan_text;  // the plan as built in
  std::string custom_plan;     // a custom plan's text instead, as read_tariff() reads it: its lines count from 1
  float battery_kwh = 0.0f;
  float charging_kw = 0.0f;
  std::string vin;
  int32_t standard_offset = 0;  // the time zone's, in seconds
};

// `text` in capitals.
inline std::string upper(std::string text) {
  for (char &c : text)
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  return text;
}

// Reads and checks the settings file: two-space indents, `key: value` or `key:` lines, comments, and values in quotes
// or not. tariff: has a plan built in, or a custom plan: a plan's file (plans/README.md), each line indented by two
// spaces, whose line numbers count from the line after tariff:, as in the file. Returns what's wrong, or "".
inline std::string read_settings(const std::string &text, const Plans &plans, SettingsFile &settings) {
  if (text.size() > MAX_SETTINGS_BYTES)
    return "the file is longer than 4 kB";
  SettingsFile read;
  std::string area, battery, charging, currency, fixed, margin, vat, zone;
  // Each setting by its place in the file, and where its value goes; market: and tariff: head lines of their own.
  const std::pair<const char *, std::string *> places[] = {{"currency", &currency},
                                                           {"fixed_price", &fixed},
                                                           {"market", nullptr},
                                                           {"market: area", &area},
                                                           {"market: margin", &margin},
                                                           {"market: vat", &vat},
                                                           {"ntfy_topic", &read.ntfy_topic},
                                                           {"tariff", nullptr},
                                                           {"tariff: plan", &read.plan},
                                                           {"tesla_battery_kwh", &battery},
                                                           {"tesla_charging_kw", &charging},
                                                           {"tesla_vin", &read.vin},
                                                           {"timezone", &zone}};
  std::vector<std::string> seen;
  std::string section;
  bool custom = false;  // a custom plan's lines under tariff:
  size_t start = 0;
  for (int number = 1; start < text.size(); number++) {
    const size_t end = std::min(text.find('\n', start), text.size());
    std::string line = text.substr(start, end - start);
    start = end + 1;
    line.erase(std::min(line.find(" #"), line.size()));
    line.erase(line.find_last_not_of(" \r") + 1);
    const size_t indent = line.find_first_not_of(' ');
    const auto at = [&](std::string_view what) { return concat({"line ", std::to_string(number), " ", what}); };
    if (indent == std::string::npos || line[indent] == '#') {
      if (section == "tariff")
        read.custom_plan += "\n";
      continue;
    }
    // A custom plan's lines, which read_tariff() checks with the plan file's own line numbers.
    if (section == "tariff" && indent >= 2 && line.compare(2, 5, "plan:") != 0) {
      read.custom_plan += line.substr(2) + "\n";
      custom = true;
      continue;
    }
    const size_t colon = line.find(':');
    if (colon == std::string::npos || colon == indent || (colon + 1 < line.size() && line[colon + 1] != ' '))
      return at("isn't a key and a value");
    const std::string key = line.substr(indent, colon - indent);
    std::string value = line.substr(colon + 1);
    value.erase(0, value.find_first_not_of(' '));
    const bool heading = value.empty();  // a key with lines of its own below, unlike one with "" for a value
    if (value.size() >= 2 && (value[0] == '"' || value[0] == '\'') && value.back() == value[0])
      value = value.substr(1, value.size() - 2);
    std::string place;
    if (indent == 0) {
      section = heading ? key : "";
      place = key;
    } else if (indent == 2 && section == "market" && !heading) {
      place = "market: " + key;
    } else if (indent == 2 && section == "tariff") {  // plan:, as a custom plan's lines are read above
      place = "tariff: plan";
    } else {
      return at(concat({"doesn't belong there: ", key}));
    }
    const auto *found =
        std::find_if(std::begin(places), std::end(places), [&](const auto &known) { return place == known.first; });
    if (found == std::end(places))
      return at(concat({"has ", place, ", which isn't a setting"}));
    if (std::find(seen.begin(), seen.end(), place) != seen.end())
      return at(concat({"has ", place, " again"}));
    seen.push_back(place);
    if (found->second != nullptr)
      *found->second = value;
  }

  if (std::find(seen.begin(), seen.end(), "market") != seen.end()) {
    for (const Area &known : AREAS)
      if (upper(area) == known.name)
        read.area = &known;
    if (read.area == nullptr)
      return "market: area must be one the board knows, like LT or SE3: see Countries and plans";
    read.vat = number(vat);
    if (!(read.vat >= 0.0f && read.vat < 1.0f))
      return "market: vat must be the VAT as a fraction, like 0.21 for 21%";
    read.margin = margin.empty() ? 0.0f : number(margin);
    if (!(read.margin >= 0.0f && read.margin < MAX_PRICE))
      return "market: margin must be a price per kWh, like 0.012";
  }
  // A fixed price's supplier part, without the grid fees, which the board adds to every quarter-hour like a margin.
  if (!fixed.empty()) {
    if (read.area != nullptr)
      return "fixed_price goes without market:; on top of a market price, use market: margin";
    read.margin = number(fixed);
    if (!(read.margin >= 0.0f && read.margin < MAX_PRICE))
      return "fixed_price must be a price per kWh, like 0.15";
  }

  read.currency = upper(!currency.empty() ? currency : read.area != nullptr ? own_currency(*read.area) : "EUR");
  if (read.currency.size() != 3 ||
      !std::all_of(read.currency.begin(), read.currency.end(), [](char c) { return c >= 'A' && c <= 'Z'; }))
    return "currency must be a currency's three-letter code, like EUR";
  if (read.area != nullptr) {
    const char *own = own_currency(*read.area), *converted = converted_currency(*read.area);
    if (read.currency != own && (converted == nullptr || read.currency != converted))
      return concat({"currency: ", market_name(read.area->market), "'s prices for ", read.area->name, " come in ", own,
                     converted != nullptr ? concat({", or ", converted, " at the ECB's daily rate"}) : ""});
  }

  if (read.ntfy_topic.size() > 64 || !std::all_of(read.ntfy_topic.begin(), read.ntfy_topic.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_';
      }))
    return "ntfy_topic must be the topic's name, not its address: up to 64 letters, digits, - and _";

  if (!read.plan.empty() && custom)
    return "tariff: a plan or a custom plan, not both";
  if (!read.plan.empty()) {
    const auto found =
        std::find_if(plans.begin(), plans.end(), [&](const auto &known) { return known.first == read.plan; });
    if (found == plans.end())
      return concat({"tariff: there's no plan ", read.plan});
    read.plan_text = found->second;
  }
  // Without market: and tariff:, the board has no prices, and the car charges as usual. A tariff: with neither a plan
  // nor a calendar under it, like one with the plan's name on its own line, would add nothing.
  TariffText custom_text;
  if (read.plan_text.empty() && read_tariff(read.custom_plan, custom_text).empty() && custom_text.calendar.empty() &&
      std::find(seen.begin(), seen.end(), "tariff") != seen.end())
    // nothing under tariff: names no plan; a custom plan that doesn't read gets make_tariff()'s error
    return read.custom_plan.empty() ? "tariff: there's no plan" : "custom plan: there's no calendar";
  Tariff tariff;
  if (const std::string error =
          make_tariff(custom ? read.custom_plan : std::string(read.plan_text), read.currency, tariff);
      !error.empty())
    return concat({custom ? "custom plan: " : "tariff: ", error});

  // The car, all three or none yet, as the setup saves the prices before the car's step. Wider than the page's 20 to
  // 200 kWh and 1 to 22 kW, and closed, so a schedule's numbers stay in range.
  if (!battery.empty() || !charging.empty() || !read.vin.empty()) {
    read.battery_kwh = number(battery);
    if (!(read.battery_kwh >= 10.0f && read.battery_kwh <= 1000.0f))
      return "tesla_battery_kwh must be the battery's size in kWh, like 75";
    read.charging_kw = number(charging);
    if (!(read.charging_kw >= 0.5f && read.charging_kw <= 100.0f))
      return "tesla_charging_kw must be the charging power in kW, like 11";
    if (read.vin.size() != 17 || !std::all_of(read.vin.begin(), read.vin.end(), [](char c) {
          return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z' && c != 'I' && c != 'O' && c != 'Q');
        }))
      return "tesla_vin must be the car's VIN: 17 capital letters and digits, none of them I, O or Q, on the car's "
             "screen under Controls, Software";
  }
  const auto *found = std::find_if(std::begin(TIME_ZONES), std::end(TIME_ZONES),
                                   [&](const auto &known) { return zone == known.first; });
  if (found == std::end(TIME_ZONES))
    return "timezone must be one the board knows, like Europe/Vilnius";
  read.standard_offset = found->second * 3600;
  settings = read;
  return "";
}

// Whether saving `now` over `was` restarts a board that has a car: for another car, whose Bluetooth link and key are
// its own, or prices of another market area or in another currency, as those downloaded already are the old ones. The
// rest applies at once, as all of it does while the board has no car.
inline bool restarts(const SettingsFile &was, const SettingsFile &now) {
  return !was.vin.empty() && (now.vin != was.vin || now.area != was.area || now.currency != was.currency);
}

// Whether saving `now` over `was` deletes the schedule on a board that has a car, as Delete schedule does: for any
// change but the ntfy topic's, as the schedule was made with the rest, and the topic only says where its message goes.
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
inline std::string settings_options(const Plans &plans) {
  std::string json = "{\"areas\":[";
  for (const Area &area : AREAS)
    json += concat({json.back() == '[' ? "" : ",", json_string(area.name)});
  json += "],\"plans\":[";
  for (const auto &plan : plans) {
    TariffText text;
    read_tariff(std::string(plan.second), text);  // the plans built in read, as the tests check
    json += concat({json.back() == '[' ? "" : ",", "[", json_string(plan.first), ",",
                    json_string(text.name.empty() ? plan.first : text.name), "]"});
  }
  return json + "]}";
}

}  // namespace esphome::scheduler
