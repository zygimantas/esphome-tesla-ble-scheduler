#include "scheduler_component.h"

#include "esphome/components/network/util.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"

#include <cstring>
#include <memory>

namespace esphome::scheduler {

static const char *const TAG = "scheduler";

void ReadyBy::restore() {
  this->pref_ = this->make_entity_preference<datetime::TimeEntityRestoreState>();
  datetime::TimeEntityRestoreState saved{};
  if (!this->pref_.load(&saved))
    saved = {7, 0, 0};
  saved.apply(this);
}

void ReadyBy::control(const datetime::TimeCall &call) {
  datetime::TimeEntityRestoreState saved{call.get_hour().value_or(this->hour_),
                                         call.get_minute().value_or(this->minute_),
                                         call.get_second().value_or(this->second_)};
  saved.apply(this);
  this->pref_.save(&saved);
  this->parent_->reschedule();
}

void ReadyByOnce::restore() {
  this->pref_ = this->make_entity_preference<datetime::DateTimeEntityRestoreState>();
  datetime::DateTimeEntityRestoreState saved{};
  if (!this->pref_.load(&saved))
    saved = {2000, 1, 1, 0, 0, 0};
  saved.apply(this);
}

void ReadyByOnce::control(const datetime::DateTimeCall &call) {
  datetime::DateTimeEntityRestoreState saved{
      call.get_year().value_or(this->year_),     call.get_month().value_or(this->month_),
      call.get_day().value_or(this->day_),       call.get_hour().value_or(this->hour_),
      call.get_minute().value_or(this->minute_), call.get_second().value_or(this->second_)};
  saved.apply(this);
  this->pref_.save(&saved);
  this->parent_->reschedule();
}

void ActionButton::press_action() { this->parent_->press(this->action_); }

// The Tesla component creates its entities internally, so they're found by name.
template <typename List>
static auto find(const List &entities, const char *name) {
  std::decay_t<decltype(*entities.begin())> found = nullptr;
  for (auto *entity : entities) {
    if (entity->get_name() == name)
      found = entity;
  }
  return found;
}

void SchedulerComponent::load_settings() {
#ifdef USE_WEBSERVER
  // Before setup(), so that the settings page's handler goes ahead of the one ESPHome's web server adds in its own
  // setup(): its canHandle() then sees every request (keep_streams()).
  web_server_base::global_web_server_base->add_handler(&this->settings_page_);
#endif
  this->settings_pref_ = global_preferences->make_preference<SavedSettings>(fnv1_hash("scheduler_settings"));
  auto saved = std::make_unique<SavedSettings>();  // 4 kB is a lot for the stack
  // An empty file, as Restart setup saves, is none.
  if (this->settings_pref_.load(saved.get()) && saved->text[0] != '\0')
    this->use_settings(std::string(saved->text, strnlen(saved->text, sizeof(saved->text))));
  else
    this->settings_error_ = "No settings yet";
}

// A file the checks turn away stays the one the page shows, to fix, and read_settings() leaves file_ as it was.
void SchedulerComponent::use_settings(const std::string &text) {
  this->settings_text_ = text;
  const std::string error = read_settings(text, this->plans_, this->file_);
  this->settings_error_ = error.empty() ? "" : "Settings: " + error;
}

// On the web server's task, the only one that uses settings_text_ once the board runs, so the next request sees the
// new file; the rest on the next loop: the settings apply at once, or restart the board, which starts the prices and
// the car's connection afresh (restarts() in settings.h), and other values than the topic's delete the schedule
// (deletes_schedule()).
void SchedulerComponent::save_settings(const std::string &text, bool restart, bool deletes) {
  this->settings_text_ = text;
  this->defer([this, text, restart, deletes]() {
    auto saved = std::make_unique<SavedSettings>();
    std::snprintf(saved->text, sizeof(saved->text), "%s", text.c_str());
    this->settings_pref_.save(saved.get());
    if (deletes) {  // saved now, as the board may restart
      this->controller_.delete_schedule();
      this->held_ = this->controller_.held_mode();
      this->held_pref_.save(&this->held_);
    }
    global_preferences->sync();
    if (restart) {
      App.safe_reboot();
      return;
    }
    read_settings(text, this->plans_, this->file_);  // the web server checked it
    this->settings_error_.clear();
    this->apply_settings_();
    this->reschedule();  // a message not sent yet is about the old schedule
  });
}

void SchedulerComponent::setup() {
  this->ready_by_->restore();
  this->ready_by_once_->restore();
  this->held_pref_ = global_preferences->make_preference<int32_t>(fnv1_hash("scheduler_held_mode"));
  this->held_pref_.load(&this->held_);
  this->controller_.restore_mode(this->held_);
  // Straight into the controller, as 6 kB is a lot for the stack. A build with another size starts afresh (savings.h).
  this->savings_pref_ = global_preferences->make_preference<Savings>(fnv1_hash("scheduler_savings"));
  this->savings_pref_.load(&this->controller_.savings);
  this->paired_pref_ = global_preferences->make_preference<uint32_t>(fnv1_hash("scheduler_paired_vin"));
  this->paired_pref_.load(&this->paired_vin_);

  this->plug_ = find(App.get_binary_sensors(), "Charger");
  this->charging_state_ = find(App.get_text_sensors(), "Charging");
  this->battery_ = find(App.get_sensors(), "Battery");
  this->power_ = find(App.get_sensors(), "Charger Power");
  this->charger_ = find(App.get_switches(), "Charger");
  this->wake_ = find(App.get_buttons(), "Wake up");
  this->limit_ = find(App.get_numbers(), "Charging Limit");
  // A new limit, from the web page, the car or the Tesla app, reschedules right away.
  if (this->limit_ != nullptr) {
    this->limit_->add_on_state_callback([this](float value) {
      if (value != this->last_limit_) {
        this->last_limit_ = value;
        this->unsent_.reset();  // a message about the old schedule isn't true any more
        this->tick_soon_();
      }
    });
  }
  // The charge port flap, which the car reports even while it sleeps, and to any key, one it doesn't know too.
  this->port_ = find(App.get_covers(), "Charge Port Door");
#ifdef USE_LOCK
  this->port_latch_ = find(App.get_locks(), "Charge Port Latch");
#endif
  if (this->port_ != nullptr)
    this->port_->add_on_state_callback([this]() { this->port_reported_ = true; });
  // A command the car turned away, as from a key it doesn't know, says the key was removed in the car: the board
  // forgets the pairing and the plug state, and the setup's key step comes back, until the car reports the plug again.
  // esphome-tesla-ble says so only for its commands, in Last Command; its polls just go unanswered.
  text_sensor::TextSensor *last_command = find(App.get_text_sensors(), "Last Command");
  if (last_command != nullptr)
    last_command->add_on_state_callback([this](const std::string &result) {
      if (result.find("key not on whitelist") == std::string::npos || this->paired_vin_ == 0)
        return;
      ESP_LOGW(TAG, "The car doesn't know the board's key: %s", result.c_str());
      if (this->plug_ != nullptr)
        this->plug_->invalidate_state();  // else the same plug state again would publish nothing
      this->paired_vin_ = 0;
      this->paired_pref_.save(&this->paired_vin_);
      global_preferences->sync();
    });
  // Pairing sends the car a request, which the key card confirms in the car. esphome-tesla-ble then waits for an answer
  // that never comes: it sends the request again every 30 s, 6 times, and runs no other command meanwhile, so for 3
  // minutes the board can't read the car, and another request waits its turn or is turned away. A Bluetooth reconnect
  // drops it, and the car keeps the request until the key card, as Tesla's own tool sends it and disconnects. So 10 s
  // after a pairing request, from this page or any other, the board reconnects, and asks for a read every 15 s, for 3
  // minutes or until the car reports the plug state, which shows the key is paired and moves it on at once.
  button::Button *pair = find(App.get_buttons(), "Pair BLE Key");
  button::Button *read = find(App.get_buttons(), "Force data update");
  switch_::Switch *link = find(App.get_switches(), "BLE Connection");
  if (pair != nullptr && read != nullptr && this->plug_ != nullptr) {
    pair->add_on_press_callback([this, read, link]() {
      const uint32_t asked = millis();
      this->set_timeout("pairing link", 10 * 1000, [this, link]() {
        if (link == nullptr || !link->state)
          return;  // none, or turned off by hand
        link->turn_off();
        this->set_timeout("pairing link", 2 * 1000, [link]() { link->turn_on(); });
      });
      this->set_interval("pairing", 15 * 1000, [this, read, asked]() {
        if (this->plug_->has_state() || millis() - asked > 3 * 60 * 1000)
          this->cancel_interval("pairing");
        else
          read->press();
      });
    });
    // Every change, as on_state callbacks skip the first state, which is the one here.
    this->plug_->add_full_state_callback([this](optional<bool>, optional<bool> now) {
      if (now.has_value() && this->paired_vin_ != fnv1_hash(this->file_.vin))
        this->tick_soon_();
    });
  }

#ifdef USE_ESP32
  // The task for the board's HTTPS requests (start_requests_()), its stack in PSRAM, as internal RAM is short while
  // HTTPS runs. Without it, they run on the loop.
  this->requests_task_.create(
      [](void *self) {
        for (;;) {
          ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
          static_cast<SchedulerComponent *>(self)->run_requests_();
        }
      },
      "requests", 8192, this, 1, true);
#endif

  if (this->settings_error_.empty())
    this->apply_settings_();
}

// What the settings set outside the file: the clock's time zone, market prices or none, and the tariff.
void SchedulerComponent::apply_settings_() {
#ifdef USE_TIME_TIMEZONE
  // The clock's time zone: its offset in winter, and the EU's summer time from 01:00 UTC on the last Sunday of March
  // to 01:00 UTC on the last Sunday of October, as calendar.h has it.
  time::ParsedTimezone zone{};
  zone.std_offset_seconds = -this->file_.standard_offset;
  zone.dst_offset_seconds = -this->file_.standard_offset - 3600;
  zone.dst_start = {3600 + this->file_.standard_offset, 0, time::DSTRuleType::MONTH_WEEK_DAY, 3, 5, 0};
  zone.dst_end = {7200 + this->file_.standard_offset, 0, time::DSTRuleType::MONTH_WEEK_DAY, 10, 5, 0};
  time::set_global_tz(zone);
#endif
  this->controller_.set_market_prices(this->file_.area != nullptr);
  // read_settings() made the same tariff
  this->apply_tariff_(this->file_.plan.empty() ? this->file_.custom_plan : std::string(this->file_.plan_text));
}

// A text sensor's new state, if it's new.
static void publish(text_sensor::TextSensor *sensor, const std::string &value) {
  if (sensor->state != value)
    sensor->publish_state(value);
}

void SchedulerComponent::update() {
  // Nothing to schedule until there are settings, nor until the setup's car step, after its prices, names the car.
  if (!this->settings_error_.empty() || this->file_.vin.empty()) {
    publish(this->status_, this->settings_error_.empty() ? "No car yet" : this->settings_error_);
    publish(this->mode_, "wait");
    return;
  }
  const ESPTime now = this->clock_->now();
  const CarState car = this->read_car_(now.is_valid() ? now.timestamp : 0);
  // The clock keeps running through a restart, so wait for the network too.
  if (network::is_connected() && !this->requesting_)
    this->start_downloads_(car.now);
  const Decision d = this->controller_.tick(car, this->schedule_settings_());
  this->persist_(d);
  this->carry_out_(d, car.now);
  this->publish_(d, car.now);
}

// The car as the Tesla's entities report it, at `now` (0 until the clock is set).
CarState SchedulerComponent::read_car_(int64_t now) {
  CarState car;
  car.now = now;
  if (this->plug_ != nullptr && this->plug_->has_state())
    car.plugged = this->plug_->state;
  if (this->charging_state_ != nullptr && this->charging_state_->has_state())
    car.charging_state = this->charging_state_->state;
  car.soc = this->battery_ != nullptr ? this->battery_->state : NAN;
  car.limit = this->limit_ != nullptr && this->limit_->has_state() ? this->limit_->state : NAN;
  car.power_kw = this->power_ != nullptr ? this->power_->state : NAN;
  car.port_open = this->port_reported_ && this->port_->position == cover::COVER_OPEN;

  // The car reports the plug state only to a key it knows: its first report shows the key is paired with the car the
  // settings name, which erasing the board, another VIN or the car turning the key away undoes. The charge port flap
  // shows nothing, as the car reports it to any key.
  const uint32_t vin = fnv1_hash(this->file_.vin);
  if (car.plugged.has_value() && this->paired_vin_ != vin) {
    this->paired_vin_ = vin;
    this->paired_pref_.save(&this->paired_vin_);
    global_preferences->sync();
  }
  car.paired = this->paired_vin_ == vin;
  return car;
}

// What the schedule needs of the settings file, and Ready by and Ready by once.
Settings SchedulerComponent::schedule_settings_() const {
  Settings settings;
  settings.currency = this->file_.currency.c_str();
  settings.battery_kwh = this->file_.battery_kwh;
  settings.charging_kw = this->file_.charging_kw;
  settings.standard_offset = this->file_.standard_offset;
  settings.unlock_when_charged = this->file_.unlock_when_charged;
  settings.ready_by_minutes = this->ready_by_->hour * 60 + this->ready_by_->minute;
  const ReadyByOnce &once = *this->ready_by_once_;
  settings.ready_by_once = once.year >= 2020 ? local_to_utc(days_from_civil(once.year, once.month, once.day),
                                                            once.hour * 60 + once.minute, settings.standard_offset)
                                             : 0;
  return settings;
}

// Keeps in flash what a restart mustn't lose: what the buttons chose, and the savings when the controller says so.
void SchedulerComponent::persist_(const Decision &d) {
  if (this->controller_.held_mode() != this->held_) {
    this->held_ = this->controller_.held_mode();
    this->held_pref_.save(&this->held_);
    global_preferences->sync();  // now, in case the board restarts soon after
  }
  if (d.save_savings) {
    this->savings_pref_.save(&this->controller_.savings);
    global_preferences->sync();
  }
}

// The decision's phone message, through ntfy, and its command to the car.
void SchedulerComponent::carry_out_(const Decision &d, int64_t now) {
  if (d.notification && !this->file_.ntfy_topic.empty()) {  // a new message replaces an unsent older one
    this->unsent_ = *d.notification;
    this->unsent_since_ = now;
    this->message_tried_at_ = 0;
  }
  if (d.mode == "unplugged")  // whatever the message said is over
    this->unsent_.reset();
  this->send_unsent_(now);
  if (d.command == Command::START_CHARGING && this->charger_ != nullptr) {
    ESP_LOGI(TAG, "Start charging (%s)", d.status.c_str());
    this->charger_->turn_on();
  } else if (d.command == Command::STOP_CHARGING && this->charger_ != nullptr) {
    ESP_LOGI(TAG, "Stop charging (%s)", d.status.c_str());
    this->charger_->turn_off();
  } else if (d.command == Command::WAKE && this->wake_ != nullptr) {
    ESP_LOGI(TAG, "Wake the car (%s)", d.status.c_str());
    this->wake_->press();
  }
#ifdef USE_LOCK
  if (d.unlock_port && this->port_latch_ != nullptr) {
    ESP_LOGI(TAG, "Unlock the charge port (charged)");
    this->port_latch_->unlock();
  }
#endif
}

// The decision on the page's entities, with the status saying so when the Tesla's entities are missing.
void SchedulerComponent::publish_(const Decision &d, int64_t now) {
  const bool found = this->plug_ != nullptr && this->charging_state_ != nullptr && this->battery_ != nullptr &&
                     this->power_ != nullptr && this->charger_ != nullptr && this->wake_ != nullptr &&
                     this->limit_ != nullptr;
  publish(this->status_, found ? d.status : "Tesla entities not found");
  publish(this->mode_, d.mode);
  publish(this->windows_, d.windows);
  publish(this->prices_until_, std::to_string(this->controller_.prices.known_until(now)));
  publish(this->savings_, d.savings);
}

void SchedulerComponent::dump_config() {
  const SettingsFile &file = this->file_;
  if (!this->settings_error_.empty()) {
    ESP_LOGCONFIG(TAG, "Scheduler:\n  %s", this->settings_error_.c_str());
    return;
  }
  const std::string market =
      file.area != nullptr ? concat({file.area->name, " from ", market_name(file.area->market)}) : "none";
  const char *plan = !file.plan.empty() ? file.plan.c_str() : file.custom_plan.empty() ? "none" : "custom plan";
  ESP_LOGCONFIG(TAG,
                "Scheduler:\n"
                "  Market: %s, prices in %s\n"
                "  Plan: %s\n"
                "  Battery: %.0f kWh\n"
                "  Charging power: %.1f kW\n"
                "  Phone messages: %s",
                market.c_str(), file.currency.c_str(), plan, file.battery_kwh, file.charging_kw,
                file.ntfy_topic.empty() ? "off" : "on");
  LOG_UPDATE_INTERVAL(this);
}

// A new Ready by: the message about the old schedule, if still unsent, isn't true any more.
void SchedulerComponent::reschedule() {
  this->unsent_.reset();
  this->controller_.reschedule();
  this->tick_soon_();
}

void SchedulerComponent::press(Action action) {
  if (action != Action::RESET_SAVINGS)  // a schedule button: a message about the old schedule isn't true any more
    this->unsent_.reset();
  switch (action) {
    case Action::CREATE_SCHEDULE:
      this->controller_.create_schedule();
      break;
    case Action::CHARGE_NOW:
      this->controller_.charge_now();
      break;
    case Action::STOP_CHARGING:
      this->controller_.stop_charging();
      break;
    case Action::RESET_SAVINGS:
      this->controller_.reset_savings();
      break;
    case Action::RESTART_SETUP:
      this->restart_setup_();
      return;
  }
  this->tick_soon_();
}

// Restart setup: forgets what the board keeps of its own, the settings, the savings, the buttons' hold, the pairing and
// Ready by, makes a new key, which the car doesn't know, and restarts into the setup. ESPHome's Wi-Fi stays.
void SchedulerComponent::restart_setup_() {
  ESP_LOGW(TAG, "Restart setup");
  this->settings_pref_.save(std::make_unique<SavedSettings>().get());
  this->held_ = 0;
  this->held_pref_.save(&this->held_);
  this->paired_vin_ = 0;
  this->paired_pref_.save(&this->paired_vin_);
  this->ready_by_->make_call().set_time(7, 0, 0).perform();
  this->ready_by_once_->make_call().set_datetime(2000, 1, 1, 0, 0, 0).perform();
  button::Button *new_key = find(App.get_buttons(), "Regenerate key");
  if (new_key != nullptr)
    new_key->press();
  global_preferences->sync();
  App.safe_reboot();
}

// On the next loop, so a tick never runs inside another entity's callback.
void SchedulerComponent::tick_soon_() {
  this->defer("tick", [this]() { this->update(); });
}

// Uses `text`, a plan's text, built in, downloaded or custom. Returns what's wrong, or "".
std::string SchedulerComponent::apply_tariff_(const std::string &text) {
  Tariff tariff;
  const std::string error = make_tariff(text, this->file_.currency, tariff);
  if (!error.empty())
    return error;
  tariff.vat = this->file_.vat;
  tariff.margin = this->file_.margin;
  this->controller_.set_tariff(tariff);
  this->plan_text_ = text;
  return "";
}

}  // namespace esphome::scheduler
