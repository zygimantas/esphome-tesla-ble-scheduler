#pragma once
// The board's settings: the file uploaded on its page (README.md's Settings), read and checked before the board takes
// it. Plain C++17, with nothing from ESPHome, like charger.h.

#include "market.h"
#include "tariff.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace esphome::scheduler {

// The most a settings file can have.
constexpr size_t MAX_SETTINGS_BYTES = 4096;

// The plans built in, by name like lt/eso-standartinis-4-zones, with their text.
using Plans = std::vector<std::pair<std::string_view, std::string_view>>;

// The time zones whose clocks change on the EU's dates, as calendar.h has them, with their offset from UTC in winter,
// in hours.
constexpr std::pair<const char *, int> TIME_ZONES[] = {
    {"Africa/Ceuta", 1},      {"Arctic/Longyearbyen", 1}, {"Asia/Famagusta", 2},     {"Asia/Nicosia", 2},
    {"Atlantic/Canary", 0},   {"Atlantic/Faroe", 0},      {"Atlantic/Madeira", 0},   {"Europe/Amsterdam", 1},
    {"Europe/Andorra", 1},    {"Europe/Athens", 2},       {"Europe/Belgrade", 1},    {"Europe/Berlin", 1},
    {"Europe/Bratislava", 1}, {"Europe/Brussels", 1},     {"Europe/Bucharest", 2},   {"Europe/Budapest", 1},
    {"Europe/Busingen", 1},   {"Europe/Copenhagen", 1},   {"Europe/Dublin", 0},      {"Europe/Gibraltar", 1},
    {"Europe/Guernsey", 0},   {"Europe/Helsinki", 2},     {"Europe/Isle_of_Man", 0}, {"Europe/Jersey", 0},
    {"Europe/Lisbon", 0},     {"Europe/Ljubljana", 1},    {"Europe/London", 0},      {"Europe/Luxembourg", 1},
    {"Europe/Madrid", 1},     {"Europe/Malta", 1},        {"Europe/Mariehamn", 2},   {"Europe/Monaco", 1},
    {"Europe/Oslo", 1},       {"Europe/Paris", 1},        {"Europe/Podgorica", 1},   {"Europe/Prague", 1},
    {"Europe/Riga", 2},       {"Europe/Rome", 1},         {"Europe/San_Marino", 1},  {"Europe/Sarajevo", 1},
    {"Europe/Skopje", 1},     {"Europe/Sofia", 2},        {"Europe/Stockholm", 1},   {"Europe/Tallinn", 2},
    {"Europe/Tirane", 1},     {"Europe/Vaduz", 1},        {"Europe/Vatican", 1},     {"Europe/Vienna", 1},
    {"Europe/Vilnius", 2},    {"Europe/Warsaw", 1},       {"Europe/Zagreb", 1},      {"Europe/Zurich", 1},
};

struct SettingsFile {
  std::string currency;        // of the market prices and the tariff
  const Area *area = nullptr;  // null without market prices
  float vat = 0.0f;            // on the market prices
  float margin = 0.0f;         // the supplier's, per kWh with VAT
  std::string ntfy_server = "https://ntfy.sh";
  std::string ntfy_topic;      // empty: no phone messages
  std::string plan;            // the plan's name, empty without one
  std::string_view plan_text;  // the plan as built in
  std::string tariff;          // the tariff's own settings, as read_tariff() reads them
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

// `text` as a number, or NaN.
inline float number(const std::string &text) {
  char *end = nullptr;
  const float value = std::strtof(text.c_str(), &end);
  return !text.empty() && end == text.c_str() + text.size() ? value : NAN;
}

// Reads and checks the settings file: two-space indents, `key: value` or `key:` lines, comments, and values in quotes
// or not, with the tariff: block as docs/tariff.md has it and the plans built in. Returns what's wrong, or "".
inline std::string read_settings(const std::string &text, const Plans &plans, SettingsFile &settings) {
  if (text.size() > MAX_SETTINGS_BYTES)
    return "the file is longer than 4 kB";
  SettingsFile read;
  std::string area, battery, charging, currency, margin, vat, zone;
  // Each setting by its place in the file, and where its value goes; market: and tariff: head lines of their own.
  const std::pair<const char *, std::string *> places[] = {{"currency", &currency},
                                                           {"market", nullptr},
                                                           {"market: area", &area},
                                                           {"market: margin", &margin},
                                                           {"market: vat", &vat},
                                                           {"ntfy_server", &read.ntfy_server},
                                                           {"ntfy_topic", &read.ntfy_topic},
                                                           {"tariff", nullptr},
                                                           {"tariff: plan", &read.plan},
                                                           {"tesla_battery_kwh", &battery},
                                                           {"tesla_charging_kw", &charging},
                                                           {"tesla_vin", &read.vin},
                                                           {"timezone", &zone}};
  std::vector<std::string> seen;
  std::string section;
  size_t start = 0;
  for (int number = 1; start < text.size(); number++) {
    size_t end = text.find('\n', start);
    end = end == std::string::npos ? text.size() : end;
    std::string line = text.substr(start, end - start);
    start = end + 1;
    line.erase(std::min(line.find(" #"), line.size()));
    line.erase(line.find_last_not_of(" \r") + 1);
    const size_t indent = line.find_first_not_of(' ');
    std::string own;  // the line in the tariff's own settings, which keep the file's line numbers for read_tariff()
    const auto at = [&](std::string_view what) { return concat({"line ", std::to_string(number), " ", what}); };
    if (indent == std::string::npos || line[indent] == '#') {
      read.tariff += "\n";
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
    } else if (indent == 2 && section == "tariff" && key == "plan") {
      place = "tariff: plan";
    } else if (indent >= 2 && section == "tariff") {
      own = line.substr(2);
    } else {
      return at(concat({"doesn't belong there: ", key}));
    }
    read.tariff += own + "\n";
    if (place.empty())
      continue;
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
      return "market: area must be one the board knows, like LT or SE3: see Countries";
    read.vat = number(vat);
    if (!(read.vat >= 0.0f && read.vat < 1.0f))
      return "market: vat must be the VAT as a fraction, like 0.21 for 21%";
    read.margin = margin.empty() ? 0.0f : number(margin);
    if (!(read.margin >= 0.0f && read.margin < MAX_PRICE))
      return "market: margin must be a price per kWh, like 0.012";
  }

  read.currency = upper(!currency.empty() ? currency : read.area != nullptr ? own_currency(*read.area) : "EUR");
  if (read.currency.size() != 3 ||
      !std::all_of(read.currency.begin(), read.currency.end(), [](char c) { return c >= 'A' && c <= 'Z'; }))
    return "currency must be a currency's three-letter code, like EUR";
  if (read.area != nullptr && !comes_in(*read.area, read.currency))
    return concat({"currency: ", market_name(read.area->market), "'s prices come in ",
                   read.area->market == Market::NORD_POOL ? "DKK, EUR, NOK, PLN, RON or SEK" : "EUR"});

  const size_t scheme = read.ntfy_server.find("://");
  if (scheme == std::string::npos ||
      (read.ntfy_server.compare(0, scheme, "https") != 0 && read.ntfy_server.compare(0, scheme, "http") != 0))
    return "ntfy_server must be the server's address, like https://ntfy.sh";
  if (read.ntfy_topic.size() > 64 || !std::all_of(read.ntfy_topic.begin(), read.ntfy_topic.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_';
      }))
    return "ntfy_topic must be the topic's name, not its address: up to 64 letters, digits, - and _";

  if (!read.plan.empty()) {
    const auto found =
        std::find_if(plans.begin(), plans.end(), [&](const auto &known) { return known.first == read.plan; });
    if (found == plans.end()) {
      std::string names;
      for (const auto &known : plans)
        names += concat({names.empty() ? "" : ", ", known.first});
      return concat({"tariff: there's no plan ", read.plan, "; there are ", names});
    }
    read.plan_text = found->second;
  }
  const std::string plan_text(read.plan_text);
  TariffText plan_tariff, own;
  read_tariff(plan_text, plan_tariff);
  if (!plan_text.empty() && plan_tariff.currency != read.currency)
    return concat(
        {"tariff: the plan ", read.plan, " is in ", plan_tariff.currency, ": set currency: ", plan_tariff.currency});
  if (const std::string error = read_tariff(read.tariff, own); !error.empty())
    return concat({"tariff: ", error});
  // Without either, the board has no prices yet, as after the setup's first step, and the car charges as usual.
  if (plan_text.empty() && own.calendar.empty() && read.area == nullptr &&
      std::find(seen.begin(), seen.end(), "tariff") != seen.end())
    return "without market:, tariff needs a plan or a calendar of its own";
  Tariff tariff;
  if (const std::string error = make_tariff(plan_text, read.tariff, read.currency, tariff); !error.empty())
    return concat({"tariff: ", error});

  read.battery_kwh = number(battery);
  if (!(read.battery_kwh > 0.0f && std::isfinite(read.battery_kwh)))
    return "tesla_battery_kwh must be the battery's size in kWh, like 75";
  read.charging_kw = number(charging);
  if (!(read.charging_kw > 0.0f && std::isfinite(read.charging_kw)))
    return "tesla_charging_kw must be the charging power in kW, like 11";
  if (read.vin.size() != 17 || !std::all_of(read.vin.begin(), read.vin.end(), [](char c) {
        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z' && c != 'I' && c != 'O' && c != 'Q');
      }))
    return "tesla_vin must be the car's VIN: 17 capital letters and digits, none of them I, O or Q, on the car's "
           "screen under Controls, Software";
  const auto *found = std::find_if(std::begin(TIME_ZONES), std::end(TIME_ZONES),
                                   [&](const auto &known) { return zone == known.first; });
  if (found == std::end(TIME_ZONES))
    return "timezone must be one the board knows, like Europe/Vilnius";
  read.standard_offset = found->second * 3600;
  settings = read;
  return "";
}

// A plan's name for people, from its first line, like "# ESO Standartinis, four zones, prices with VAT:
// ..."; empty when the line isn't like that.
inline std::string plan_title(std::string_view text) {
  const size_t end = text.find(", prices with VAT");
  return text.rfind("# ", 0) == 0 && end < text.find('\n') ? std::string(text.substr(2, end - 2)) : "";
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

// What the settings form on the page offers, as JSON: the market areas, the plans built in by name and name for people,
// and the time zones.
inline std::string settings_options(const Plans &plans) {
  std::string json = "{\"areas\":[";
  for (const Area &area : AREAS)
    json += concat({json.back() == '[' ? "" : ",", json_string(area.name)});
  json += "],\"plans\":[";
  for (const auto &plan : plans) {
    const std::string title = plan_title(plan.second);
    json += concat({json.back() == '[' ? "" : ",", "[", json_string(plan.first), ",",
                    json_string(title.empty() ? plan.first : title), "]"});
  }
  json += "],\"time_zones\":[";
  for (const auto &zone : TIME_ZONES)
    json += concat({json.back() == '[' ? "" : ",", json_string(zone.first)});
  return json + "]}";
}

}  // namespace esphome::scheduler
