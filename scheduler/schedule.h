#pragma once
// The schedule: the cheapest quarter-hours before the deadline, priced at Nord Pool's spot price (market.h) plus VAT
// and the tariff (tariff.h), that bring the battery to the car's charge limit, plus a buffer slot. Plain C++17, with
// nothing from ESPHome, like charger.h.

#include "calendar.h"
#include "market.h"
#include "tariff.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <numeric>
#include <string>
#include <vector>

namespace esphome::scheduler {

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
  const char *currency = "EUR";  // of the Nord Pool prices and the tariff
};

struct ScheduleRequest {
  int64_t now = 0;
  int64_t deadline = 0;
  float soc = NAN;
  float limit = NAN;
  Tariff tariff;
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

struct Schedule {
  bool valid = false;      // false when this quarter-hour has no price
  int needed_slots = 0;    // to reach the limit, plus the buffer
  int unpriced_slots = 0;  // the last ones before the deadline, whose prices aren't out yet
  std::vector<Window> windows;
  float avg_price = NAN;   // EUR/kWh at total prices (see total_price()) for the energy the car is expected to take
  float soc_at_end = NAN;  // the battery level after the windows, in %

  bool contains(int64_t t) const {
    return std::any_of(windows.begin(), windows.end(), [t](const Window &w) { return w.start <= t && t < w.end; });
  }
};

inline Schedule make_schedule(const PriceTable &prices, const ScheduleRequest &request) {
  Schedule schedule;
  const int64_t known_until = prices.known_until(request.now);
  if (known_until <= request.now)
    return schedule;
  schedule.valid = true;
  const int64_t priced_end = std::min(request.deadline, known_until);
  const int64_t first = floor_to_slot(request.now);
  std::vector<float> totals;  // total_price() of each slot from `first` to priced_end
  for (int64_t t = first; t + SLOT_SECONDS <= priced_end; t += SLOT_SECONDS)
    // Up to known_until(), every slot has a price.
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access,clang-analyzer-core.CallAndMessage)
    totals.push_back(total_price(*prices.get(t), t, request.tariff, request.settings.standard_offset));
  schedule.unpriced_slots = static_cast<int>((request.deadline - priced_end) / SLOT_SECONDS);
  const float missing_kwh = (request.limit - request.soc) / 100.0f * request.settings.capacity_kwh;
  const float slot_kwh = request.settings.charge_kw * (SLOT_SECONDS / 3600.0f) * EFFICIENCY;
  schedule.needed_slots = missing_kwh > 0.05f ? static_cast<int>(std::ceil(missing_kwh / slot_kwh)) + BUFFER_SLOTS : 0;
  // No guessing: while the quarter-hours without prices could still do all the charging, it waits for
  // their prices; else it buys only what they can't do.
  const int buy = schedule.needed_slots - schedule.unpriced_slots;
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
    if (schedule.windows.empty() || schedule.windows.back().end != start)
      schedule.windows.push_back({start, start});
    Window &w = schedule.windows.back();
    w.end = start + SLOT_SECONDS;
    w.energy_kwh += grid_kwh;
    w.cost_eur += grid_kwh * totals[i];
  }
  for (Window &w : schedule.windows) {
    const auto begin = totals.begin() + (w.start - first) / SLOT_SECONDS;
    const int64_t count = (w.end - w.start) / SLOT_SECONDS;
    w.avg_price = w.energy_kwh > 0 ? w.cost_eur / w.energy_kwh
                                   : std::accumulate(begin, begin + count, 0.0f) / static_cast<float>(count);
  }
  schedule.avg_price = cost_eur / energy_kwh;  // NaN (0 / 0) when the car takes nothing
  schedule.soc_at_end = soc;
  return schedule;
}

// The schedule's windows for the web page: "<currency>;<start>,<end>,<price per kWh>[,spare];...", in UTC seconds with
// each window's price (see Window::avg_price), marked spare when the car shouldn't need it. For example
// "EUR;1790463600,1790466300,0.203;1790467200,1790468100,0.211,spare". Just the currency without windows.
inline std::string format_windows(const Schedule &schedule, const char *currency) {
  std::string text = currency;
  char part[64];
  for (const Window &w : schedule.windows) {
    std::snprintf(part, sizeof(part), ";%lld,%lld,%.3f%s", static_cast<long long>(w.start),
                  static_cast<long long>(w.end), w.avg_price, w.spare() ? ",spare" : "");
    text += part;
  }
  return text;
}

}  // namespace esphome::scheduler
