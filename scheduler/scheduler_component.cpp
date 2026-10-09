#include "scheduler_component.h"

#include "esphome/components/json/json_util.h"
#include "esphome/components/network/util.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"

#include <array>
#include <cstring>
#include <memory>
#ifdef USE_WEBSERVER
#include <sys/select.h>
#endif

namespace esphome::scheduler {

static const char *const TAG = "scheduler";

// The most the board reads of an answer: a day of LT prices is about 11 kB, SMARD's week 15 kB and a plan up to
// about 2 kB. OKTE's day, 67 kB, goes through okte_filter() as it comes instead (read_body_()).
static constexpr size_t MAX_BODY_BYTES = 24 * 1024;
// The plans as their maintainers keep them current, on GitHub.
static const char *const PLANS = "https://raw.githubusercontent.com/zygimantas/esphome-tesla-ble-scheduler/main/plans/";
// Phone messages go through ntfy's own server, to the topic the settings name.
static const char *const NTFY = "https://ntfy.sh";

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
  this->controller_.reset_savings();
  this->savings_pref_.save(&this->controller_.savings);
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

// The downloads due at `now`, the market's prices and the plan, as a batch of the requests task.
void SchedulerComponent::start_downloads_(int64_t now) {
  this->requests_ = Requests{};
  Requests &batch = this->requests_;
  batch.now = now;
  batch.area = this->controller_.fetch_prices_due(now) ? this->file_.area : nullptr;
  // today's only while some of it is missing
  batch.first_day = this->controller_.prices.known_until(now) >= end_of_delivery_day(now) ? 1 : 0;
  batch.currency = this->file_.currency;
  if (!this->file_.plan.empty() && plan_due(now, this->plan_tried_at_, this->plan_usable_)) {
    this->plan_tried_at_ = now;
    this->plan_usable_ = false;
    batch.plan = this->file_.plan;
  }
  if (batch.area != nullptr || !batch.plan.empty())
    this->start_requests_();
}

// Runs the batch in the requests task (setup()), one at a time. Each request takes the board a second or two, a new TLS
// connection every time, which the loop can't spare: the Bluetooth events wait for it, the car's among them, and drop
// once ESPHome's queue for them is full.
void SchedulerComponent::start_requests_() {
  this->requesting_ = true;
#ifdef USE_ESP32
  if (this->requests_task_.is_created()) {
    xTaskNotifyGive(this->requests_task_.get_handle());
    return;
  }
#endif
  this->run_requests_();
}

// A batch. Each answer goes to the loop, which uses the prices, the plan and the message meanwhile, and so does the
// batch's end, after the answers, with a tick at once, as new prices or a new plan change the schedule.
void SchedulerComponent::run_requests_() {
  if (this->requests_.area != nullptr)
    this->fetch_prices_();
  if (!this->requests_.plan.empty())
    this->fetch_plan_();
  if (this->requests_.message)
    this->send_message_();
  this->defer([this]() {
    this->requesting_ = false;
    this->tick_soon_();
  });
}

// ArduinoJson's reader of a response, a chunk at a time as it comes: -1 once it fails, times out or ends.
class ResponseReader {
 public:
  ResponseReader(http_request::HttpContainer &response, uint32_t timeout) : response_(response), timeout_(timeout) {}
  int read() {
    if (this->at_ == this->size_ && !this->fill_())
      return -1;
    return this->chunk_[this->at_++];
  }
  size_t readBytes(char *buffer, size_t length) {
    size_t count = 0;
    for (int c; count < length && (c = this->read()) >= 0; count++)
      buffer[count] = static_cast<char>(c);
    return count;
  }

 protected:
  // The next chunk, waiting for it up to the timeout: false, then and after, once the read fails, times out or ends.
  bool fill_() {
    uint32_t last_data = millis();
    while (!this->ended_) {
      const int read = this->response_.read(this->chunk_, sizeof(this->chunk_));
      const auto result =
          http_request::http_read_loop_result(read, last_data, this->timeout_, this->response_.is_read_complete());
      if (result == http_request::HttpReadLoopResult::DATA) {
        this->at_ = 0;
        this->size_ = read;
        return true;
      }
      this->ended_ = result != http_request::HttpReadLoopResult::RETRY;
    }
    return false;
  }

  http_request::HttpContainer &response_;
  uint32_t timeout_;
  uint8_t chunk_[512];
  size_t at_ = 0, size_ = 0;
  bool ended_ = false;
};

// A response's whole body, or nothing when the read fails, times out or passes MAX_BODY_BYTES before it's complete.
// With a `filter`, what it keeps of the JSON, as JSON, read as it comes, so the whole answer needn't fit: nothing when
// it can't be read. Without App.feed_wdt(): the watchdog doesn't watch the requests task, and on the loop the read
// feeds it.
std::optional<std::string> SchedulerComponent::read_body_(http_request::HttpContainer &response,
                                                          const JsonDocument *filter) {
  if (filter != nullptr) {
    ResponseReader reader(response, this->http_->get_timeout());
#ifdef USE_PSRAM
    json::SpiRamAllocator allocator;  // the kept part of a day of OKTE's is about 9 kB
    JsonDocument doc(&allocator);
#else
    JsonDocument doc;
#endif
    if (deserializeJson(doc, reader, DeserializationOption::Filter(*filter)) != DeserializationError::Ok)
      return std::nullopt;
    std::string kept;
    kept.reserve(measureJson(doc));
    serializeJson(doc, kept);
    return kept;
  }
  std::string body;
  uint8_t chunk[512];
  uint32_t last_data = millis();
  while (body.size() <= MAX_BODY_BYTES) {  // a byte past it tells a longer body from one of exactly MAX_BODY_BYTES
    const int read = response.read(chunk, std::min(sizeof(chunk), MAX_BODY_BYTES + 1 - body.size()));
    const auto result =
        http_request::http_read_loop_result(read, last_data, this->http_->get_timeout(), response.is_read_complete());
    if (result == http_request::HttpReadLoopResult::COMPLETE)
      return body;
    if (result == http_request::HttpReadLoopResult::DATA)
      body.append(reinterpret_cast<const char *>(chunk), read);
    else if (result != http_request::HttpReadLoopResult::RETRY)
      break;
  }
  return std::nullopt;
}

// GETs `url`: the answer's HTTP status, or nothing when the request fails, and with 200 its body, or nothing when it's
// cut off, through `filter` if any (read_body_()). The response ends before the body is parsed: the connection's memory
// isn't needed any more.
std::optional<int> SchedulerComponent::fetch_(const std::string &url, std::optional<std::string> &body,
                                              const JsonDocument *filter) {
  auto response = this->http_->get(url);
  if (response == nullptr)
    return std::nullopt;
  const int status = response->status_code;
  if (status == http_request::HTTP_STATUS_OK)
    body = this->read_body_(*response, filter);
  response->end();
  return status;
}

// Where the prices of the CET delivery day `day` days after the batch's time are.
std::string SchedulerComponent::prices_url_(int day) const {
  const Requests &batch = this->requests_;
  switch (batch.area->market) {
    case Market::SMARD:
      return smard_url(batch.area->smard_filter, batch.now, day);
    case Market::OMIE:
      return omie_url(batch.now, day);
    case Market::OKTE:
      return okte_url(batch.now, day);
    default:
      return nord_pool_url(batch.area->source_name, batch.currency.c_str(), batch.now, day);
  }
}

// On the loop: stores an answer's prices, SMARD's times `rate`: how many, or -1 if it can't be read.
int SchedulerComponent::store_prices_(const std::string &body, float rate) {
  PriceTable &prices = this->controller_.prices;
  const Area &area = *this->file_.area;
  switch (area.market) {
    case Market::SMARD:
      return prices.add_smard(body.data(), body.size(), rate);
    case Market::OMIE:
      return prices.add_omie(body.data(), body.size(), area.source_name);
    case Market::OKTE:
      return prices.add_okte(body.data(), body.size());
    default:
      return prices.add_nord_pool(body.data(), body.size(), area.source_name);
  }
}

// Today's and tomorrow's CET delivery days, from the batch's first. A failed request ends the try: the next one would
// fail the same way.
void SchedulerComponent::fetch_prices_() {
  const int64_t now = this->requests_.now;
  const Area &area = *this->requests_.area;
  const char *source = market_name(area.market);
  // SMARD's prices are in euros: in the country's own currency, where the settings ask for it, at the ECB's latest rate
  float rate = 1.0f;
  if (area.market == Market::SMARD && this->requests_.currency != "EUR") {
    const std::optional<float> ecb = this->fetch_rate_();
    if (!ecb)
      return;
    rate = *ecb;
  }
  // Not out yet: Nord Pool has no content for tomorrow, SMARD no file for a week that hasn't begun, OMIE none for the
  // day, and OKTE an empty list, which stores none.
  const int not_yet =
      area.market == Market::NORD_POOL ? http_request::HTTP_STATUS_NO_CONTENT : http_request::HTTP_STATUS_NOT_FOUND;
  const JsonDocument okte = okte_filter();
  std::string fetched;  // SMARD's file has the week, so tomorrow's prices are often in today's
  for (int day = this->requests_.first_day; day < 2; day++) {
    const std::string url = this->prices_url_(day);
    if (url == fetched)
      continue;
    fetched = url;
    std::optional<std::string> body;
    const std::optional<int> status = this->fetch_(url, body, area.market == Market::OKTE ? &okte : nullptr);
    if (!status) {
      ESP_LOGW(TAG, "%s request failed", source);
      break;
    }
    if (*status == http_request::HTTP_STATUS_OK) {
      this->defer([this, now, day, rate, source, name = area.name, body = std::move(body)]() {
        PriceTable &prices = this->controller_.prices;
        const int64_t until = prices.known_until(now);
        const int stored = body ? this->store_prices_(*body, rate) : -1;
        if (stored < 0) {
          ESP_LOGW(TAG, "%s: %s", source, body ? "could not parse the answer" : "the answer was cut off");
        } else if (prices.known_until(now) > until) {  // not just the week's earlier prices again, as SMARD sends
          ESP_LOGI(TAG, "%s: stored %d quarter-hours", source, stored);
          this->controller_.reschedule();
        } else if (stored == 0 && day == 0) {
          ESP_LOGW(TAG, "%s: no prices for %s in the answer: check Country / Area under Settings", source, name);
        }
      });
    } else if (*status != not_yet) {
      ESP_LOGW(TAG, "%s answered HTTP %d", source, *status);
    } else if (day == 0) {  // today's prices are always out: tomorrow's may not be yet
      ESP_LOGW(TAG, "%s has no prices for %s today: check Country / Area under Settings", source, area.name);
    }
  }
}

// What a euro is worth in the settings' currency, from the ECB's latest reference rates, or nothing when the request
// fails: then the prices wait for the next try.
std::optional<float> SchedulerComponent::fetch_rate_() {
  const char *currency = this->requests_.currency.c_str();
  std::optional<std::string> body;
  const std::optional<int> status = this->fetch_(ECB_RATES_URL, body);
  const std::optional<float> rate = body ? ecb_rate(body->data(), body->size(), currency) : std::nullopt;
  if (!status)
    ESP_LOGW(TAG, "ECB request failed");
  else if (*status != http_request::HTTP_STATUS_OK)
    ESP_LOGW(TAG, "ECB answered HTTP %d", *status);
  else if (rate)
    ESP_LOGI(TAG, "ECB: 1 EUR = %.4f %s", *rate, currency);
  else
    ESP_LOGW(TAG, "ECB: no rate for %s in the answer", currency);
  return rate;
}

// The plan as its maintainer keeps it, from GitHub. A failed or cut-off download, or a plan the board can't use, leaves
// the one in use, and the board tries again in an hour. So does a save that names another plan meanwhile.
void SchedulerComponent::fetch_plan_() {
  const char *plan = this->requests_.plan.c_str();
  std::optional<std::string> text;
  const std::optional<int> status = this->fetch_(std::string(PLANS) + plan + ".yaml", text);
  if (!status) {
    ESP_LOGW(TAG, "Plan %s: request failed", plan);
  } else if (*status != http_request::HTTP_STATUS_OK) {
    ESP_LOGW(TAG, "Plan %s: GitHub answered HTTP %d", plan, *status);
  } else if (!text) {
    ESP_LOGW(TAG, "Plan %s: the download was cut off, so the one in use stays", plan);
  } else {
    this->defer([this, plan = std::string(plan), text = std::move(*text)]() {
      if (this->file_.plan != plan)
        return;
      if (text == this->plan_text_) {
        this->plan_usable_ = true;
        return;
      }
      const std::string error = this->apply_tariff_(text);
      this->plan_usable_ = error.empty();
      if (error.empty())
        ESP_LOGI(TAG, "Plan %s: new prices", plan.c_str());
      else
        ESP_LOGW(TAG, "Plan %s: %s, so the one in use stays", plan.c_str(), error.c_str());
    });
  }
}

// Sends the message that hasn't gone out, as a batch of the requests task, once a minute while the network is up, and
// drops it after half an hour: a schedule from then is still worth reading, an older one isn't.
void SchedulerComponent::send_unsent_(int64_t now) {
  if (!this->unsent_ || now - this->message_tried_at_ < 60)
    return;
  if (now - this->unsent_since_ > 30 * 60) {
    ESP_LOGW(TAG, "ntfy message dropped after 30 minutes of failed sends");
    this->unsent_.reset();
    return;
  }
  if (!network::is_connected() || this->requesting_)
    return;
  this->message_tried_at_ = now;
  this->requests_ = Requests{};
  this->requests_.message = this->unsent_;
  this->requests_.message_json = this->message_json_(*this->unsent_);
  this->start_requests_();
}

// A message as ntfy takes it. A tap opens the page, on the home Wi-Fi, at the board's address now, which every phone
// opens, as some Android phones don't find tesla.local. An unset address reads 0.0.0.0.
std::string SchedulerComponent::message_json_(const Notification &message) const {
  std::string host = App.get_name() + ".local";
  for (const auto &ip : network::get_ip_addresses()) {
    char address[network::IP_ADDRESS_BUFFER_SIZE];
    if (ip.is_ip4() && std::strcmp(ip.str_to(address), "0.0.0.0") != 0) {
      host = address;
      break;
    }
  }
  return json::build_json([this, &message, &host](JsonObject root) {
    root["topic"] = this->file_.ntfy_topic;
    root["title"] = message.title;
    root["message"] = message.message;
    root["tags"].to<JsonArray>().add("electric_plug");
    root["click"] = "http://" + host;
  });
}

// Sends the batch's message through ntfy. Once ntfy has it, the loop drops it, unless a newer one has replaced it.
void SchedulerComponent::send_message_() {
  auto response = this->http_->post(NTFY, this->requests_.message_json);
  if (response == nullptr) {
    ESP_LOGW(TAG, "ntfy message failed; it's tried again in a minute");
    return;
  }
  const bool taken = response->status_code == http_request::HTTP_STATUS_OK;
  if (!taken)
    ESP_LOGW(TAG, "ntfy answered HTTP %d; the message is tried again in a minute", response->status_code);
  response->end();
  if (taken)
    this->defer([this, sent = *this->requests_.message]() {
      if (this->unsent_ && this->unsent_->title == sent.title && this->unsent_->message == sent.message)
        this->unsent_.reset();
    });
}

#ifdef USE_WEBSERVER
// ESP-IDF's web server keeps 7 connections and, for another, closes the one whose last request is the oldest: a page's
// live stream (/events), as its only request was its first. Pages open on two or three phones and computers then closed
// each other's streams in turn, and kept reconnecting. So each request marks the streams as just used, and a browser's
// idle connection goes instead. Not a stream that takes no more, from a page that stopped reading: ESPHome gives up on
// it but leaves it open, and it may still go.
static void keep_streams(httpd_handle_t server) {
  std::array<int, CONFIG_LWIP_MAX_SOCKETS> fds{};
  size_t count = fds.size();
  if (httpd_get_client_list(server, &count, fds.data()) != ESP_OK)
    return;
  for (size_t i = 0; i < count; i++) {
    if (httpd_sess_get_ctx(server, fds[i]) == nullptr)  // of ESPHome's connections, only a stream has a context
      continue;
    fd_set writable;
    FD_ZERO(&writable);
    FD_SET(fds[i], &writable);
    timeval now{};
    if (select(fds[i] + 1, nullptr, &writable, nullptr, &now) == 1)
      httpd_sess_update_lru_counter(server, fds[i]);
  }
}

bool SettingsPage::canHandle(AsyncWebServerRequest *request) const {
  const httpd_req_t *req = *request;
  keep_streams(req->handle);  // every request passes here first (load_settings())
  char buffer[AsyncWebServerRequest::URL_BUF_SIZE];
  const StringRef url = request->url_to(buffer);
  return (url == "/settings" && (request->method() == HTTP_GET || request->method() == HTTP_POST)) ||
         (url == "/settings/options" && request->method() == HTTP_GET);
}

// Keeps a byte past MAX_SETTINGS_BYTES at most, enough for read_settings() to tell the file is too long.
void SettingsPage::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (index == 0)
    this->body_.clear();
  this->body_.append(reinterpret_cast<const char *>(data), std::min(len, MAX_SETTINGS_BYTES + 1 - this->body_.size()));
}

void SettingsPage::handleRequest(AsyncWebServerRequest *request) {
  static const char *const TEXT = "text/plain; charset=utf-8";
  // Only the board's own page, as with ESPHome's own routes, or another site's could read the VIN or replace the
  // settings. The board's page sends no Origin or one that is the Host it asked after the scheme; curl sends none. 400,
  // as this web server sends a 403 as 500.
  const std::optional<std::string> origin = request->get_header("Origin");
  if (origin.has_value() && !origin->ends_with("://" + request->get_header("Host").value_or(""))) {
    request->send(400, TEXT, "Only the board's own page can read or change the settings");
    this->body_.clear();
    return;
  }
  char buffer[AsyncWebServerRequest::URL_BUF_SIZE];
  if (request->url_to(buffer) == "/settings/options") {
    // In chunks of about 1 kB: all 11 kB in one string, which doubles as it grows, took more memory than the board had
    // left while it fetched over HTTPS, and it restarted.
    httpd_req_t *req = *request;
    httpd_resp_set_type(req, "application/json");
    std::string chunk;
    settings_options(this->parent_->plans(), [req, &chunk](const std::string &piece) {
      chunk += piece;
      if (chunk.size() >= 1024) {
        httpd_resp_send_chunk(req, chunk.data(), chunk.size());
        chunk.clear();
      }
    });
    if (!chunk.empty())  // an empty chunk would end the answer
      httpd_resp_send_chunk(req, chunk.data(), chunk.size());
    httpd_resp_send_chunk(req, nullptr, 0);
    return;
  }
  if (request->method() == HTTP_GET) {
    request->send(200, TEXT, this->parent_->settings_text().c_str());
    return;
  }
  SettingsFile file, was;
  const std::string error = read_settings(this->body_, this->parent_->plans(), file);
  if (error.empty()) {
    // the settings the board runs on, none if they don't read; the answer goes out before a restart
    read_settings(this->parent_->settings_text(), this->parent_->plans(), was);
    const bool restart = restarts(was, file);
    request->send(200, TEXT, restart ? "Saved: the board restarts" : "Saved");
    this->parent_->save_settings(this->body_, restart, deletes_schedule(was, file));
  } else {
    request->send(400, TEXT, error.c_str());
  }
  this->body_.clear();
}
#endif

}  // namespace esphome::scheduler
