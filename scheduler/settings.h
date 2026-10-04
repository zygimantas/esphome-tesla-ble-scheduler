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
  std::string ntfy_topic;  // empty: no phone messages
  std::string plan;        // the plan's name, empty without one
  std::string tariff;      // the tariff's own settings, as read_tariff() reads them
  float battery_kwh = 0.0f;
  float charging_kw = 0.0f;
  std::string vin;
  int32_t standard_offset = 0;  // the time zone's, in seconds
};

// The settings file as written: each key once, by its place like "market: area", with its line's number and value.
using Written = std::vector<std::pair<std::string, std::pair<int, std::string>>>;

// Reads the settings file: two-space indents, `key: value` or `key:` lines, comments, and values in quotes or not, with
// the tariff: block as docs/tariff.md has it. Returns what's wrong, or "".
inline std::string read_settings_text(const std::string &text, Written &written, std::string &tariff) {
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
    std::string own;  // the line in the tariff's own settings: the same numbers in read_tariff()'s messages
    const auto at = [&](std::string_view what) { return concat({"line ", std::to_string(number), " ", what}); };
    if (indent == std::string::npos || line[indent] == '#') {
      tariff += "\n";
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
    tariff += own + "\n";
    if (place.empty())
      continue;
    if (std::any_of(written.begin(), written.end(), [&](const auto &other) { return other.first == place; }))
      return at(concat({"has ", place, " again"}));
    written.push_back({place, {number, value}});
  }
  return "";
}

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

// Reads and checks the settings file, with the plans built in. Returns what's wrong, or "".
inline std::string read_settings(const std::string &text, const Plans &plans, SettingsFile &settings) {
  if (text.size() > MAX_SETTINGS_BYTES)
    return "the file is longer than 4 kB";
  Written written;
  SettingsFile read;
  if (const std::string error = read_settings_text(text, written, read.tariff); !error.empty())
    return error;
  const auto value = [&](const char *place) -> const std::string * {
    for (const auto &[key, line] : written)
      if (key == place)
        return &line.second;
    return nullptr;
  };
  static constexpr const char *KEYS[] = {
      "currency", "market",       "market: area",      "market: margin",    "market: vat", "ntfy_server", "ntfy_topic",
      "tariff",   "tariff: plan", "tesla_battery_kwh", "tesla_charging_kw", "tesla_vin",   "timezone"};
  for (const auto &entry : written)
    if (std::find_if(std::begin(KEYS), std::end(KEYS), [&](const char *known) { return entry.first == known; }) ==
        std::end(KEYS))
      return concat({"line ", std::to_string(entry.second.first), " has ", entry.first, ", which isn't a setting"});
  for (const char *key : {"tariff", "tesla_battery_kwh", "tesla_charging_kw", "tesla_vin", "timezone"})
    if (value(key) == nullptr)
      return concat({key, " is missing"});

  if (value("market") != nullptr) {
    const std::string *area = value("market: area");
    const std::string *vat = value("market: vat");
    if (area == nullptr || vat == nullptr)
      return area == nullptr ? "market: area is missing" : "market: vat is missing";
    for (const Area &known : AREAS)
      if (upper(*area) == known.name)
        read.area = &known;
    if (read.area == nullptr)
      return concat({"market: area ", *area, " isn't one the board knows, like LT or SE3: see Countries"});
    read.vat = number(*vat);
    if (!(read.vat >= 0.0f && read.vat < 1.0f))
      return "market: vat must be the VAT as a fraction, like 0.21 for 21%";
    const std::string *margin = value("market: margin");
    read.margin = margin != nullptr ? number(*margin) : 0.0f;
    if (!(read.margin >= 0.0f && read.margin < MAX_PRICE))
      return "market: margin must be a price per kWh, like 0.012";
  }

  const std::string *currency = value("currency");
  read.currency = upper(currency != nullptr ? *currency : read.area != nullptr ? own_currency(*read.area) : "EUR");
  if (read.currency.size() != 3 ||
      !std::all_of(read.currency.begin(), read.currency.end(), [](char c) { return c >= 'A' && c <= 'Z'; }))
    return "currency must be a currency's three-letter code, like EUR";
  if (read.area != nullptr && !comes_in(*read.area, read.currency))
    return concat({"currency: ", market_name(read.area->market), "'s prices come in ",
                   read.area->market == Market::NORD_POOL ? "DKK, EUR, NOK, PLN, RON or SEK" : "EUR"});

  if (const std::string *server = value("ntfy_server"); server != nullptr)
    read.ntfy_server = *server;
  const size_t scheme = read.ntfy_server.find("://");
  if (scheme == std::string::npos ||
      (read.ntfy_server.compare(0, scheme, "https") != 0 && read.ntfy_server.compare(0, scheme, "http") != 0))
    return "ntfy_server must be the server's address, like https://ntfy.sh";
  if (const std::string *topic = value("ntfy_topic"); topic != nullptr)
    read.ntfy_topic = *topic;
  if (read.ntfy_topic.size() > 64 || !std::all_of(read.ntfy_topic.begin(), read.ntfy_topic.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_';
      }))
    return "ntfy_topic must be the topic's name, not its address: up to 64 letters, digits, - and _";

  std::string plan_text;
  if (const std::string *plan = value("tariff: plan"); plan != nullptr) {
    const auto found =
        std::find_if(plans.begin(), plans.end(), [&](const auto &known) { return known.first == *plan; });
    if (found == plans.end()) {
      std::string names;
      for (const auto &known : plans)
        names += concat({names.empty() ? "" : ", ", known.first});
      return concat({"tariff: there's no plan ", *plan, "; there are ", names});
    }
    read.plan = *plan;
    plan_text = found->second;
  }
  TariffText plan_tariff, own;
  read_tariff(plan_text, plan_tariff);
  if (!plan_text.empty() && plan_tariff.currency != read.currency)
    return concat(
        {"tariff: the plan ", read.plan, " is in ", plan_tariff.currency, ": set currency: ", plan_tariff.currency});
  if (const std::string error = read_tariff(read.tariff, own); !error.empty())
    return concat({"tariff: ", error});
  if (plan_text.empty() && own.calendar.empty())
    return "tariff needs a plan, or a calendar of its own";
  Tariff tariff;
  if (const std::string error = make_tariff(plan_text, read.tariff, read.currency, tariff); !error.empty())
    return concat({"tariff: ", error});

  read.battery_kwh = number(*value("tesla_battery_kwh"));
  if (!(read.battery_kwh > 0.0f && std::isfinite(read.battery_kwh)))
    return "tesla_battery_kwh must be the battery's size in kWh, like 75";
  read.charging_kw = number(*value("tesla_charging_kw"));
  if (!(read.charging_kw > 0.0f && std::isfinite(read.charging_kw)))
    return "tesla_charging_kw must be the charging power in kW, like 11";
  read.vin = *value("tesla_vin");
  if (read.vin.size() != 17 || !std::all_of(read.vin.begin(), read.vin.end(), [](char c) {
        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z' && c != 'I' && c != 'O' && c != 'Q');
      }))
    return "tesla_vin must be the car's VIN: 17 capital letters and digits, none of them I, O or Q, on the car's "
           "screen under Controls, Software";
  const std::string &zone = *value("timezone");
  const auto found = std::find_if(std::begin(TIME_ZONES), std::end(TIME_ZONES),
                                  [&](const auto &known) { return zone == known.first; });
  if (found == std::end(TIME_ZONES))
    return concat({"timezone: ", zone, " isn't one the board knows, like Europe/Vilnius"});
  read.standard_offset = found->second * 3600;
  settings = read;
  return "";
}

}  // namespace esphome::scheduler
