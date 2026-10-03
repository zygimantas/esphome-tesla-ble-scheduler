#pragma once
// The market: Nord Pool's day-ahead prices, where to download them, and the quarter-hours' prices. Plain C++17 plus
// ArduinoJson, with nothing from ESPHome, like charger.h.

#include "calendar.h"

#include <ArduinoJson.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace esphome::scheduler {

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

}  // namespace esphome::scheduler
