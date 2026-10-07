#pragma once
// What charging cost and saved, by local day, as the board keeps it in flash and the page shows it, and its counting as
// the car charges. Plain C++17, with nothing from ESPHome, like charger.h.

#include "calendar.h"
#include "market.h"
#include "schedule.h"
#include "tariff.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

namespace esphome::scheduler {

// The energy the car took from the grid on one local day, what it cost, and what it would have cost at the
// delivery day's average total price and charging at once (see SavingsCounter::count()). Money in
// hundredths of the currency, rounded from the day's sum rather than from each tick.
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
static_assert(sizeof(Savings) == 5848, "saved byte for byte: another size starts every board's savings afresh");

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

// Counts what the car's charging costs into Savings, tick by tick, for Controller in charger.h.
class SavingsCounter {
 public:
  // Counts the energy the car took since the last tick (its power times the time, at most a minute back) into
  // `savings`: at the total price of each quarter-hour, at the delivery day's average total price, and at the price
  // it would have had charging at once. Charging at once starts when the car needs charging (`needed`: plugged in and
  // not full, once the plug state is known), at a plug-in or when it's no longer full, and replays the car's charging
  // time from then. Until it starts (after a restart, when the car was plugged in is unknown), past a day of it, or
  // without its price, it counts as the day's average. A quarter-hour without a price counts as the day's average
  // throughout, or as nothing without that: it neither saves nor costs. Returns whether to write `savings` to flash.
  bool count(Savings &savings, int64_t now, bool charging, float power_kw, std::optional<bool> needed,
             const PriceTable &prices, const Tariff &tariff, const Settings &settings) {
    const int64_t today = local_day_of(now, settings.standard_offset);
    if (std::strncmp(savings.currency, settings.currency, sizeof(savings.currency)) != 0) {
      savings = Savings{};
      std::snprintf(savings.currency, sizeof(savings.currency), "%s", settings.currency);
      savings.day = static_cast<int32_t>(today);
      counted_day_ = -1;
      unsaved_ = true;
    }
    for (; savings.day < today; ++savings.day)  // a new day takes the place of the one a year before
      savings.days[ring_index(savings.day + 1)] = SavingsDay{};

    if (needed.value_or(false) && !needed_) {
      at_once_prices_.assign(DAY_SECONDS / SLOT_SECONDS, NAN);
      at_once_from_ = floor_to_slot(now);
      at_once_charged_ = now - at_once_from_;
    }
    if (needed)
      needed_ = *needed;
    for (size_t i = 0; i < at_once_prices_.size(); ++i) {  // as each price comes out, then kept
      const int64_t slot = at_once_from_ + static_cast<int64_t>(i) * SLOT_SECONDS;
      const std::optional<float> spot = std::isnan(at_once_prices_[i]) ? prices.get(slot) : std::nullopt;
      if (spot)
        at_once_prices_[i] = *spot;
    }

    for (int64_t from = std::max(counted_until_, now - 60); charging && power_kw > 0 && from < now;) {
      const int64_t slot = floor_to_slot(from);
      const int64_t to = std::min(now, slot + SLOT_SECONDS);
      const float kwh = power_kw * static_cast<float>(to - from) / 3600.0f;
      const float average = day_average_(prices, tariff, slot, settings.standard_offset);
      const auto replayed = static_cast<size_t>(at_once_charged_ / SLOT_SECONDS);
      const float at_once =
          replayed < at_once_prices_.size() && !std::isnan(at_once_prices_[replayed])
              ? total_price(at_once_prices_[replayed], at_once_from_ + static_cast<int64_t>(replayed) * SLOT_SECONDS,
                            tariff, settings.standard_offset)
              : average;
      at_once_charged_ += to - from;
      const int64_t day = local_day_of(slot, settings.standard_offset);
      if (const std::optional<float> spot = prices.get(slot)) {
        add_(savings, day, kwh, total_price(*spot, slot, tariff, settings.standard_offset), average, at_once);
      } else {
        const float neutral = std::isnan(average) ? 0.0f : average;
        add_(savings, day, kwh, neutral, neutral, neutral);
      }
      from = to;
    }
    counted_until_ = now;

    // While the car charges, once a quarter-hour: a restart loses at most that.
    const bool save = unsaved_ && (floor_to_slot(now) != saved_slot_ || !charging);
    if (save) {
      unsaved_ = false;
      saved_slot_ = floor_to_slot(now);
    }
    return save;
  }

 private:
  // Adds `kwh` bought at `paid` per kWh, and its cost at the two other prices, to a local day. The day's figures add
  // up in floats, as the cents of each tick would round away.
  void add_(Savings &savings, int64_t day, float kwh, float paid, float average, float at_once) {
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
    unsaved_ = true;
  }

  // The average total price of the delivery day of `t`, over the quarter-hours with a price; NaN without one.
  static float day_average_(const PriceTable &prices, const Tariff &tariff, int64_t t, int32_t standard_offset) {
    float sum = 0;
    int count = 0;
    prices.for_each(start_of_delivery_day(t), end_of_delivery_day(t), [&](int64_t slot, float spot) {
      sum += total_price(spot, slot, tariff, standard_offset);
      ++count;
    });
    return sum / static_cast<float>(count);  // NaN (0 / 0) without prices
  }

  struct Figures {
    float kwh, paid, average, at_once;
  };
  Figures counted_{};  // of counted_day_
  bool unsaved_ = false;
  bool needed_ = true;  // plugged in and not full, at the last tick with a plug state
  int64_t counted_day_ = -1;
  int64_t counted_until_ = 0;
  int64_t saved_slot_ = 0;
  // Charging at once: the market prices of a day of quarter-hours from at_once_from_, NaN until known, with the tariff
  // added as the car charges, as for its own charging, so new fees count on both sides; and how far into them it has
  // got: the time into the first when it started, and how long the car has charged since.
  std::vector<float> at_once_prices_;
  int64_t at_once_from_ = 0;
  int64_t at_once_charged_ = 0;
};

}  // namespace esphome::scheduler
