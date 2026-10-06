#pragma once
// Connects the controller in charger.h to ESPHome: the settings file (settings.h), the web page's entities, market
// price and plan downloads, phone messages through ntfy, and the Tesla's entities from esphome-tesla-ble.

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
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"
#include "settings.h"
#ifdef USE_WEBSERVER
#include "esphome/components/web_server_base/web_server_base.h"
#endif

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

#ifdef USE_WEBSERVER
// /settings: GET answers the settings file saved, and POST takes a new one, which the board checks and saves
// (save_settings()), or answers what's wrong with it. GET /settings/options answers what the page's form offers.
class SettingsPage : public AsyncWebHandler {
 public:
  explicit SettingsPage(SchedulerComponent *parent) : parent_(parent) {}
  bool canHandle(AsyncWebServerRequest *request) const override;
  void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) override;
  void handleRequest(AsyncWebServerRequest *request) override;

 protected:
  SchedulerComponent *parent_;
  std::string body_;  // the file being uploaded
};
#endif

// The settings file as saved: up to MAX_SETTINGS_BYTES and a null.
struct SavedSettings {
  char text[MAX_SETTINGS_BYTES + 1];
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
  // A plan of this release's plans/, which the settings can name; the board downloads it anew every day.
  void add_plan(const char *name, const char *text) { this->plans_.emplace_back(name, text); }
  const Plans &plans() const { return this->plans_; }
  // The settings saved on the board, read once the plans are added and before setup().
  void load_settings();
  // Takes a settings file before the web server starts, as the simulation does.
  void use_settings(const std::string &text);
  // Saves a settings file that read_settings() took, from the web server's task: a board's first applies at once,
  // later ones restart it.
  void save_settings(const std::string &text);
  // The settings file saved, empty without one (for the web server's task), and the car's VIN from it.
  const std::string &settings_text() const { return this->settings_text_; }
  const std::string &vin() const { return this->file_.vin; }
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
  void apply_settings_();
  std::string apply_tariff_(const std::string &text);
  std::optional<std::string> read_body_(http_request::HttpContainer &response);
  std::optional<int> fetch_(const std::string &url, std::optional<std::string> &body);
  void fetch_prices_(int64_t now);
  std::optional<float> fetch_rate_();
  std::string prices_url_(int64_t now, int day) const;
  int store_prices_(const std::string &body, float rate);
  void fetch_plan_(int64_t now);
  void send_unsent_(int64_t now);
  bool send_message_(const Notification &message);

  Controller controller_;
  Settings settings_;
  time::RealTimeClock *clock_{nullptr};
  http_request::HttpRequestComponent *http_{nullptr};
  Plans plans_;
  ESPPreferenceObject settings_pref_;
  std::string settings_text_;   // the settings file saved: once the board runs, only the web server's task uses it
  std::string settings_error_;  // why there are none, for the page's status
  SettingsFile file_;
#ifdef USE_WEBSERVER
  SettingsPage settings_page_{this};
#endif
  std::string plan_text_;  // the plan in use: the copy built in until a download brings another
  int64_t plan_tried_at_{0};
  bool plan_usable_{false};             // whether the latest download brought a plan the board can use
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
  bool reported_{false};       // the car has reported since the start, or since it turned the key away
  float last_limit_{NAN};

  // What the buttons chose, kept across a restart (Controller::held_mode()).
  ESPPreferenceObject held_pref_;
  int32_t held_{0};
  ESPPreferenceObject savings_pref_;  // Controller::savings
  // The hash of the VIN of the car that has reported to the board's key, which pairing makes it do.
  ESPPreferenceObject paired_pref_;
  uint32_t paired_vin_{0};
};

}  // namespace esphome::scheduler
