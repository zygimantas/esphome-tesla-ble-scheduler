#pragma once
#include <cmath>
#include <cstdlib>
#include <string>

// The ESPHome build has the component's copy; the unit tests use the repository's.
#if __has_include("esphome/components/charging/charging.h")
#include "esphome/components/charging/charging.h"
#else
#include "charging/charging.h"
#endif

namespace charging_test {

// Stands in for the esphome-tesla-ble component and the car.
struct FakeTesla {
  bool plugged = false;
  bool charging = false;
  bool battery_known = true;
  bool complete = false;  // says Complete under the limit, and takes no start
  bool no_power = false;  // the charger withholds power: a start is kept for when it comes
  bool wants = false;
  float soc = 40;
  float limit = 80;

  // A Tesla starts charging on plug-in when no schedule applies.
  void plug_in() {
    plugged = true;
    charging = soc < limit && !complete;
  }
  void unplug() { plugged = charging = false; }
  void set_charging(bool on) {
    wants = on;
    charging = on && plugged && soc < limit && !complete && !no_power;
  }
  void power_back() {
    no_power = false;
    set_charging(wants);
  }
  std::string charging_state() const {
    return !plugged                   ? "Disconnected"
           : charging                 ? "Charging"
           : no_power                 ? "No Power"
           : complete || soc >= limit ? "Complete"
                                      : "Stopped";
  }
  // 11 kW from the charger while charging, as in simulation.yaml.
  float power_kw() const { return charging ? 11.0f : 0.0f; }
  // Into a 75 kWh pack at the planner's EFFICIENCY; stops at the limit.
  void advance(int seconds) {
    if (charging && soc < limit)
      soc = std::fmin(limit, soc + power_kw() * esphome::charging::EFFICIENCY * seconds / 3600.0f / 75.0f * 100.0f);
    charging = charging && soc < limit;
  }
  esphome::charging::CarState state(int64_t now) const {
    esphome::charging::CarState s;
    s.now = now;
    s.plugged = plugged;
    s.charging_state = charging_state();
    s.soc = battery_known ? soc : NAN;
    s.limit = battery_known ? limit : NAN;
    s.power_kw = power_kw();
    return s;
  }
};

// Plugged in: charging on its own, as a Tesla does without a schedule, or stopped.
inline FakeTesla plugged_in(bool charging = true) {
  FakeTesla tesla;
  tesla.plug_in();
  tesla.charging = charging;
  return tesla;
}

// Plugged in at startup; START_STOPPED=1 starts it not charging.
inline FakeTesla &tesla() {
  static FakeTesla instance = plugged_in(std::getenv("START_STOPPED") == nullptr);
  return instance;
}

// Three days of synthetic prices from a day before `now`: cheap 01:00-05:00 local, expensive 17:00-22:00.
// CHEAP_NOW=1 shifts them so that now is 01:30, in the cheap hours, and makes those negative, so they
// win whatever grid fee applies at this time of day.
inline void load_synthetic_prices(esphome::charging::PriceTable &prices, int64_t now) {
  using namespace esphome::charging;
  const int64_t first = floor_to_slot(now) - DAY_SECONDS;
  const bool cheap_now = std::getenv("CHEAP_NOW") != nullptr;
  int64_t shift = 0;
  if (cheap_now) {
    const int64_t local = floor_to_slot(now) + eu_offset(now, VILNIUS_STANDARD_OFFSET);
    shift = local - floor_div(local, DAY_SECONDS) * DAY_SECONDS - 90 * 60;
  }
  for (int64_t t = first; t < first + 3 * DAY_SECONDS; t += SLOT_SECONDS) {
    const int64_t local = t - shift + eu_offset(t, VILNIUS_STANDARD_OFFSET);
    const double hour = static_cast<double>(local - floor_div(local, DAY_SECONDS) * DAY_SECONDS) / 3600.0;
    double eur_mwh = 95;
    if (hour >= 1 && hour < 5)
      eur_mwh = 15 + 8 * std::fabs(hour - 3.125) - (cheap_now ? 200 : 0);
    else if (hour >= 17 && hour < 22)
      eur_mwh = 190;
    prices.set(t, static_cast<float>(eur_mwh / 1000.0));
  }
}

}  // namespace charging_test
