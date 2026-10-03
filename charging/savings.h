#pragma once
// What charging cost and saved, by local day, as the board keeps it in flash and the page shows it. Plain C++17, with
// nothing from ESPHome, like charging.h.

#include "calendar.h"

#include <cstdint>
#include <cstdio>
#include <string>

namespace esphome::charging {

// The energy the car took from the grid on one local day, what it cost, and what it would have cost at the
// delivery day's average total price and charging at once (see Controller::count_() in charging.h). Money in
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

}  // namespace esphome::charging
