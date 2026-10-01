#pragma once
// Connects the planner and controller in charging.h to ESPHome: the web page's entities, Nord Pool
// downloads, phone messages through ntfy, and the Tesla's entities from esphome-tesla-ble.

#include "charging.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/datetime/datetime_entity.h"
#include "esphome/components/datetime/time_entity.h"
#include "esphome/components/http_request/http_request.h"
#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"
#ifdef USE_COVER
#include "esphome/components/cover/cover.h"
#endif

namespace esphome::charging {

class ChargingComponent;

// "Ready by": the daily local time the car should be charged by. Saved like ESPHome's template time,
// so the value set before this component existed carries over.
class ReadyBy : public datetime::TimeEntity, public Parented<ChargingComponent> {
 public:
  void restore();

 protected:
  void control(const datetime::TimeCall &call) override;
  ESPPreferenceObject pref_;
};

// "Ready by once": a one-off date and time used instead of Ready by while it's ahead; 2000-01-01 when
// unset. Saved like ESPHome's template datetime.
class ReadyByOnce : public datetime::DateTimeEntity, public Parented<ChargingComponent> {
 public:
  void restore();

 protected:
  void control(const datetime::DateTimeCall &call) override;
  ESPPreferenceObject pref_;
};

enum class Action { CREATE_PLAN, CHARGE_NOW, STOP_CHARGING };

// The page's Create charging plan, Start charging now and Stop charging.
class PlanButton : public button::Button, public Parented<ChargingComponent> {
 public:
  void set_action(Action action) { this->action_ = action; }

 protected:
  void press_action() override;
  Action action_{};
};

class ChargingComponent : public PollingComponent {
 public:
  void setup() override;
  // One tick: read the car, decide, carry out the command and publish the results.
  void update() override;
  void dump_config() override;
  // After the Tesla's entities and the clock.
  float get_setup_priority() const override { return setup_priority::LATE; }

  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }
  void set_http(http_request::HttpRequestComponent *http) { this->http_ = http; }
  void set_nord_pool_area(const char *area) { this->area_ = area; }
  void set_battery_kwh(float kwh) { this->settings_.capacity_kwh = kwh; }
  void set_charging_kw(float kw) { this->settings_.charge_kw = kw; }
  void set_ntfy(const char *server, const char *topic) {
    this->ntfy_server_ = server;
    this->ntfy_topic_ = topic;
  }
  void set_grid(const Grid &grid) { this->controller_.set_grid(grid); }
  void set_ready_by(ReadyBy *ready_by) { this->ready_by_ = ready_by; }
  void set_ready_by_once(ReadyByOnce *ready_by_once) { this->ready_by_once_ = ready_by_once; }
  void set_status(text_sensor::TextSensor *status) { this->status_ = status; }
  void set_mode(text_sensor::TextSensor *mode) { this->mode_ = mode; }
  void set_windows(text_sensor::TextSensor *windows) { this->windows_ = windows; }
  void set_prices_until(text_sensor::TextSensor *prices_until) { this->prices_until_ = prices_until; }

  // For the simulation, which loads made-up prices.
  Controller &controller() { return this->controller_; }

  // A new Ready by or a button: re-plan or act, then tick at once.
  void replan();
  void press(Action action);

 protected:
  void tick_soon_();
  void fetch_prices_(int64_t now);
  void send_message_(const Notification &message);

  Controller controller_;
  Settings settings_;
  time::RealTimeClock *clock_{nullptr};
  http_request::HttpRequestComponent *http_{nullptr};
  const char *area_{""};
  const char *ntfy_server_{""};
  const char *ntfy_topic_{""};  // empty: no phone messages

  ReadyBy *ready_by_{nullptr};
  ReadyByOnce *ready_by_once_{nullptr};
  text_sensor::TextSensor *status_{nullptr};
  text_sensor::TextSensor *mode_{nullptr};
  text_sensor::TextSensor *windows_{nullptr};
  text_sensor::TextSensor *prices_until_{nullptr};

  // The Tesla's entities, found by name; null when missing.
  binary_sensor::BinarySensor *plug_{nullptr};
  text_sensor::TextSensor *charging_state_{nullptr};
  sensor::Sensor *battery_{nullptr};
  number::Number *limit_{nullptr};
  switch_::Switch *charger_{nullptr};
  button::Button *wake_{nullptr};
#ifdef USE_COVER
  cover::Cover *port_{nullptr};
  bool port_reported_{false};  // the cover reads open until the car reports it
#endif
  float last_limit_{NAN};

  // What the buttons chose, kept across a restart (Controller::held_mode()).
  ESPPreferenceObject held_pref_;
  int32_t held_{0};
};

}  // namespace esphome::charging
