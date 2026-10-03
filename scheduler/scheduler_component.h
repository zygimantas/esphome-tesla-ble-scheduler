#pragma once
// Connects the controller in charger.h to ESPHome: the web page's entities, Nord Pool and plan downloads, phone
// messages through ntfy, and the Tesla's entities from esphome-tesla-ble.

#include "charger.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/datetime/datetime_entity.h"
#include "esphome/components/datetime/time_entity.h"
#include "esphome/components/http_request/http_request.h"
#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"

namespace esphome::scheduler {

class SchedulerComponent;

// "Ready by": the daily local time the car should be charged by, saved under its name.
class ReadyBy : public datetime::TimeEntity, public Parented<SchedulerComponent> {
 public:
  void restore();

 protected:
  void control(const datetime::TimeCall &call) override;
  ESPPreferenceObject pref_;
};

// "Ready by once": a one-off date and time used instead of Ready by while it's ahead; 2000-01-01 when
// unset. Saved under its name.
class ReadyByOnce : public datetime::DateTimeEntity, public Parented<SchedulerComponent> {
 public:
  void restore();

 protected:
  void control(const datetime::DateTimeCall &call) override;
  ESPPreferenceObject pref_;
};

enum class Action { CREATE_SCHEDULE, CHARGE_NOW, STOP_CHARGING, RESET_SAVINGS };

// The page's Create schedule, Start charging now, Stop charging and Reset savings.
class ActionButton : public button::Button, public Parented<SchedulerComponent> {
 public:
  void set_action(Action action) { this->action_ = action; }

 protected:
  void press_action() override;
  Action action_{};
};

class SchedulerComponent : public PollingComponent {
 public:
  void setup() override;
  // One tick: read the car, decide, carry out the command and publish the results.
  void update() override;
  void dump_config() override;
  // After Wi-Fi, so the first tick doesn't run while setup still waits for it.
  float get_setup_priority() const override { return setup_priority::LATE; }

  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }
  void set_http(http_request::HttpRequestComponent *http) { this->http_ = http; }
  void set_nord_pool_area(const char *area) { this->area_ = area; }
  void set_currency(const char *currency) { this->settings_.currency = currency; }
  void set_battery_kwh(float kwh) { this->settings_.battery_kwh = kwh; }
  void set_charging_kw(float kw) { this->settings_.charging_kw = kw; }
  void set_ntfy(const char *server, const char *topic) {
    this->ntfy_server_ = server;
    this->ntfy_topic_ = topic;
  }
  // The VAT on market prices; the plan's name and the copy built in, both empty without a plan; and the tariff:
  // settings of config.yaml, written as read_tariff() reads them.
  void set_tariff(float vat, const char *plan, const char *text, const char *own) {
    this->vat_ = vat;
    this->plan_ = plan;
    this->plan_text_ = text;
    this->own_ = own;
  }
  void set_ready_by(ReadyBy *ready_by) { this->ready_by_ = ready_by; }
  void set_ready_by_once(ReadyByOnce *ready_by_once) { this->ready_by_once_ = ready_by_once; }
  void set_status(text_sensor::TextSensor *status) { this->status_ = status; }
  void set_mode(text_sensor::TextSensor *mode) { this->mode_ = mode; }
  void set_windows(text_sensor::TextSensor *windows) { this->windows_ = windows; }
  void set_prices_until(text_sensor::TextSensor *prices_until) { this->prices_until_ = prices_until; }
  void set_savings(text_sensor::TextSensor *savings) { this->savings_ = savings; }

  // For the simulation, which loads made-up prices.
  Controller &controller() { return this->controller_; }

  // A new Ready by or a button: reschedule or act, then tick at once.
  void reschedule();
  void press(Action action);

 protected:
  void tick_soon_();
  std::string apply_tariff_(const std::string &text);
  std::optional<std::string> read_body_(http_request::HttpContainer &response);
  void fetch_prices_(int64_t now);
  void fetch_plan_(int64_t now);
  void send_unsent_(int64_t now);
  bool send_message_(const Notification &message);

  Controller controller_;
  Settings settings_;
  time::RealTimeClock *clock_{nullptr};
  http_request::HttpRequestComponent *http_{nullptr};
  const char *area_{""};
  float vat_{0.0f};
  const char *plan_{""};
  const char *own_{""};
  std::string plan_text_;  // the plan in use: the copy built in until a download brings another
  int64_t plan_tried_at_{0};
  bool plan_usable_{false};  // whether the latest download brought a plan the board can use
  const char *ntfy_server_{""};
  const char *ntfy_topic_{""};          // empty: no phone messages
  std::optional<Notification> unsent_;  // the last message until ntfy has taken it
  int64_t unsent_since_{0};
  int64_t message_tried_at_{0};

  ReadyBy *ready_by_{nullptr};
  ReadyByOnce *ready_by_once_{nullptr};
  text_sensor::TextSensor *status_{nullptr};
  text_sensor::TextSensor *mode_{nullptr};
  text_sensor::TextSensor *windows_{nullptr};
  text_sensor::TextSensor *prices_until_{nullptr};
  text_sensor::TextSensor *savings_{nullptr};

  // The Tesla's entities, found by name; null when missing.
  binary_sensor::BinarySensor *plug_{nullptr};
  text_sensor::TextSensor *charging_state_{nullptr};
  sensor::Sensor *battery_{nullptr};
  sensor::Sensor *power_{nullptr};
  number::Number *limit_{nullptr};
  switch_::Switch *charger_{nullptr};
  button::Button *wake_{nullptr};
  cover::Cover *port_{nullptr};
  bool port_reported_{false};  // the cover reads open until the car reports it
  float last_limit_{NAN};

  // What the buttons chose, kept across a restart (Controller::held_mode()).
  ESPPreferenceObject held_pref_;
  int32_t held_{0};
  ESPPreferenceObject savings_pref_;  // Controller::savings
};

}  // namespace esphome::scheduler
