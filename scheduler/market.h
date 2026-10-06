#pragma once
// The market: the day-ahead prices from Nord Pool, SMARD or OMIE, where to download them, and the quarter-hours'
// prices. Plain C++17 plus ArduinoJson, with nothing from ESPHome, like charger.h.

#include "calendar.h"

#include <ArduinoJson.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace esphome::scheduler {

// The markets' delivery days run midnight to midnight CET.
constexpr int32_t CET_STANDARD_OFFSET = 3600;

// Where the board downloads the market prices.
enum class Market { NORD_POOL, SMARD, OMIE };

// The source of the market prices, as the log and the settings' messages name it.
inline const char *market_name(Market market) {
  switch (market) {
    case Market::SMARD:
      return "SMARD";
    case Market::OMIE:
      return "OMIE";
    default:
      return "Nord Pool";
  }
}

// Where electricity is bought, as market: area names it: a country's code, or the price area where a country has
// several. DE and LU are the area Germany and Luxembourg share. SMARD, Germany's Federal Network Agency, and OMIE, the
// Iberian market, have their areas' prices in euros only.
struct Area {
  const char *name;
  const char *source_name;  // the area as its market names it
  Market market;
  int smard_filter;  // SMARD's number for it
};

constexpr Area AREAS[] = {
    {"AT", "AT", Market::NORD_POOL, 0},
    {"BE", "BE", Market::NORD_POOL, 0},
    {"BG", "BG", Market::NORD_POOL, 0},
    {"CH", "CH", Market::SMARD, 259},
    {"CZ", "CZ", Market::SMARD, 261},
    {"DE", "GER", Market::NORD_POOL, 0},
    {"DK1", "DK1", Market::NORD_POOL, 0},
    {"DK2", "DK2", Market::NORD_POOL, 0},
    {"EE", "EE", Market::NORD_POOL, 0},
    {"ES", "ES", Market::OMIE, 0},
    {"FI", "FI", Market::NORD_POOL, 0},
    {"FR", "FR", Market::NORD_POOL, 0},
    {"HR", "HR", Market::NORD_POOL, 0},
    {"HU", "HU", Market::SMARD, 262},
    {"IT-NORTH", "IT-NORTH", Market::SMARD, 255},
    {"LT", "LT", Market::NORD_POOL, 0},
    {"LU", "GER", Market::NORD_POOL, 0},
    {"LV", "LV", Market::NORD_POOL, 0},
    {"NL", "NL", Market::NORD_POOL, 0},
    {"NO1", "NO1", Market::NORD_POOL, 0},
    {"NO2", "NO2", Market::NORD_POOL, 0},
    {"NO3", "NO3", Market::NORD_POOL, 0},
    {"NO4", "NO4", Market::NORD_POOL, 0},
    {"NO5", "NO5", Market::NORD_POOL, 0},
    {"PL", "PL", Market::NORD_POOL, 0},
    {"PT", "PT", Market::OMIE, 0},
    {"RO", "TEL", Market::NORD_POOL, 0},
    {"SE1", "SE1", Market::NORD_POOL, 0},
    {"SE2", "SE2", Market::NORD_POOL, 0},
    {"SE3", "SE3", Market::NORD_POOL, 0},
    {"SE4", "SE4", Market::NORD_POOL, 0},
    {"SI", "SI", Market::SMARD, 260},
};

// The currencies Nord Pool's prices come in; the other markets' come in euros.
constexpr const char *NORD_POOL_CURRENCIES[] = {"DKK", "EUR", "NOK", "PLN", "RON", "SEK"};

// The currency of an area's market prices unless the settings set one: the country's own where Nord Pool has it,
// otherwise euros.
inline const char *own_currency(const Area &area) {
  static constexpr const char *OWN[][2] = {{"DK", "DKK"}, {"NO", "NOK"}, {"PL", "PLN"}, {"RO", "RON"}, {"SE", "SEK"}};
  for (const auto &[country, currency] : OWN)
    if (std::strncmp(area.name, country, 2) == 0)
      return currency;
  return "EUR";
}

// The country's own currency for an area whose prices come only from SMARD, in euros, which the board converts them
// into at the ECB's daily rate when the settings ask for it; nullptr for the others.
inline const char *converted_currency(const Area &area) {
  static constexpr const char *CONVERTED[][2] = {{"CH", "CHF"}, {"CZ", "CZK"}, {"HU", "HUF"}};
  for (const auto &[country, currency] : CONVERTED)
    if (std::strncmp(area.name, country, 2) == 0)
      return currency;
  return nullptr;
}

// Whether an area's market prices can come in `currency`.
inline bool comes_in(const Area &area, const std::string &currency) {
  if (area.market != Market::NORD_POOL) {
    const char *converted = converted_currency(area);
    return currency == "EUR" || (converted != nullptr && currency == converted);
  }
  return std::any_of(std::begin(NORD_POOL_CURRENCIES), std::end(NORD_POOL_CURRENCIES),
                     [&](const char *own) { return currency == own; });
}

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

// SMARD's quarter-hour prices of the area it numbers `filter`, in its file for the week (from Monday 00:00 German time)
// of the CET delivery day `day_offset` days after `now`.
inline std::string smard_url(int filter, int64_t now, int day_offset) {
  const int64_t day = local_day_of(now, CET_STANDARD_OFFSET) + day_offset;
  const int64_t monday = local_to_utc(day - (weekday(day) + 6) % 7, 0, CET_STANDARD_OFFSET);
  char buf[128];
  std::snprintf(buf, sizeof(buf), "https://www.smard.de/app/chart_data/%d/DE/%d_DE_quarterhour_%lld.json", filter,
                filter, static_cast<long long>(monday) * 1000);
  return buf;
}

// OMIE's file of the day-ahead prices of Spain and Portugal for the CET delivery day `day_offset` days after `now`.
inline std::string omie_url(int64_t now, int day_offset) {
  const CivilDate date = civil_from_days(local_day_of(now, CET_STANDARD_OFFSET) + day_offset);
  char buf[128];
  std::snprintf(buf, sizeof(buf),
                "https://www.omie.es/es/file-download?parents=marginalpdbc&filename=marginalpdbc_%04d%02u%02u.1",
                static_cast<int>(date.year), date.month, date.day);
  return buf;
}

// The ECB's euro reference rates of the last working day, out around 16:00 CET.
constexpr const char *ECB_RATES_URL = "https://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml";

// What a euro is worth in `currency`, from the ECB's file, as <Cube currency='CZK' rate='24.335'/>, or nothing.
inline std::optional<float> ecb_rate(const char *xml, size_t length, const char *currency) {
  const std::string file(xml, length), key = std::string("currency='") + currency + "' rate='";
  const size_t at = file.find(key);
  float rate;
  if (at == std::string::npos || std::sscanf(file.c_str() + at + key.size(), "%f", &rate) != 1 || !(rate > 0.0f))
    return std::nullopt;
  return rate;
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
  // Stores the quarter-hour prices (per kWh) from a Nord Pool DayAheadPrices response, each at its entry's start.
  // Returns how many were stored, or -1 if the JSON could not be parsed.
  int add_nord_pool(const char *json, size_t length, const char *area) {
    JsonDocument filter;
    filter["multiAreaEntries"][0]["deliveryStart"] = true;
    filter["multiAreaEntries"][0]["entryPerArea"][area] = true;
    JsonDocument doc;
    if (deserializeJson(doc, json, length, DeserializationOption::Filter(filter)) != DeserializationError::Ok)
      return -1;
    int stored = 0;
    for (JsonObject entry : doc["multiAreaEntries"].as<JsonArray>()) {
      JsonVariant value = entry["entryPerArea"][area];
      const auto start = parse_iso8601(entry["deliveryStart"].as<const char *>());
      if (value.isNull() || !start)
        continue;
      set(*start, value.as<float>() / 1000.0f);
      ++stored;
    }
    return stored;
  }

  // Stores the quarter-hour prices (per kWh) from SMARD's file of a week: [start in ms, price per MWh] each, with null
  // for the prices not out yet, in euros times `rate`. Returns how many were stored, or -1 if the JSON could not be
  // parsed.
  int add_smard(const char *json, size_t length, float rate = 1.0f) {
    JsonDocument filter;
    filter["series"] = true;
    JsonDocument doc;
    if (deserializeJson(doc, json, length, DeserializationOption::Filter(filter)) != DeserializationError::Ok)
      return -1;
    int stored = 0;
    for (JsonArray point : doc["series"].as<JsonArray>()) {
      if (!point[0].is<int64_t>() || !point[1].is<float>())
        continue;
      set(point[0].as<int64_t>() / 1000, point[1].as<float>() * rate / 1000.0f);
      ++stored;
    }
    return stored;
  }

  // Stores the quarter-hour prices (per kWh) from OMIE's file of a day: a line per quarter-hour of the CET day,
  // year;month;day;quarter-hour from 1;Portugal's price;Spain's price; per MWh. Returns how many were stored for
  // `area`, ES or PT, or -1 if it isn't such a file.
  int add_omie(const char *text, size_t length, const char *area) {
    const std::string file(text, length);
    if (file.rfind("MARGINALPDBC;", 0) != 0)
      return -1;
    const bool portugal = std::strcmp(area, "PT") == 0;
    int stored = 0;
    for (size_t at = file.find('\n'); at != std::string::npos; at = file.find('\n', at + 1)) {
      int year, month, day, quarter;
      float pt, es;
      if (std::sscanf(file.c_str() + at + 1, "%d;%d;%d;%d;%f;%f;", &year, &month, &day, &quarter, &pt, &es) != 6)
        continue;
      const int64_t midnight = local_to_utc(days_from_civil(year, month, day), 0, CET_STANDARD_OFFSET);
      set(midnight + (quarter - 1) * SLOT_SECONDS, (portugal ? pt : es) / 1000.0f);
      ++stored;
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

}  // namespace esphome::scheduler
