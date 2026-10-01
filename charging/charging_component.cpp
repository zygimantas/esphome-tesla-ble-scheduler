#include "charging_component.h"

#include "esphome/components/json/json_util.h"
#include "esphome/components/network/util.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"

namespace esphome::charging {

static const char *const TAG = "charging";

// Keys of ESPHome's template time and datetime, so Ready by keeps the value saved before this component.
static constexpr uint32_t READY_BY_KEY = 194434060U;
static constexpr uint32_t READY_BY_ONCE_KEY = 194434090U;
// One day of LT prices is about 11 kB.
static constexpr size_t MAX_PRICES_BYTES = 24 * 1024;

void ReadyBy::restore() {
  this->pref_ = this->make_entity_preference<datetime::TimeEntityRestoreState>(READY_BY_KEY);
  datetime::TimeEntityRestoreState saved{};
  if (this->pref_.load(&saved)) {
    saved.apply(this);
    return;
  }
  this->hour_ = 7;
  this->minute_ = 0;
  this->second_ = 0;
  this->publish_state();
}

void ReadyBy::control(const datetime::TimeCall &call) {
  if (call.get_hour().has_value())
    this->hour_ = *call.get_hour();
  if (call.get_minute().has_value())
    this->minute_ = *call.get_minute();
  if (call.get_second().has_value())
    this->second_ = *call.get_second();
  this->publish_state();
  datetime::TimeEntityRestoreState saved{this->hour_, this->minute_, this->second_};
  this->pref_.save(&saved);
  this->parent_->replan();
}

void ReadyByOnce::restore() {
  this->pref_ = this->make_entity_preference<datetime::DateTimeEntityRestoreState>(READY_BY_ONCE_KEY);
  datetime::DateTimeEntityRestoreState saved{};
  if (this->pref_.load(&saved)) {
    saved.apply(this);
    return;
  }
  this->year_ = 2000;
  this->month_ = 1;
  this->day_ = 1;
  this->hour_ = 0;
  this->minute_ = 0;
  this->second_ = 0;
  this->publish_state();
}

void ReadyByOnce::control(const datetime::DateTimeCall &call) {
  if (call.get_year().has_value())
    this->year_ = *call.get_year();
  if (call.get_month().has_value())
    this->month_ = *call.get_month();
  if (call.get_day().has_value())
    this->day_ = *call.get_day();
  if (call.get_hour().has_value())
    this->hour_ = *call.get_hour();
  if (call.get_minute().has_value())
    this->minute_ = *call.get_minute();
  if (call.get_second().has_value())
    this->second_ = *call.get_second();
  this->publish_state();
  datetime::DateTimeEntityRestoreState saved{this->year_, this->month_,  this->day_,
                                             this->hour_, this->minute_, this->second_};
  this->pref_.save(&saved);
  this->parent_->replan();
}

void PlanButton::press_action() { this->parent_->press(this->action_); }

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

void ChargingComponent::setup() {
  this->ready_by_->restore();
  this->ready_by_once_->restore();
  this->held_pref_ = global_preferences->make_preference<int32_t>(fnv1_hash("charging_held_mode"));
  this->held_pref_.load(&this->held_);
  this->controller_.restore_mode(this->held_);

  this->plug_ = find(App.get_binary_sensors(), "Charger");
  this->charging_state_ = find(App.get_text_sensors(), "Charging");
  this->battery_ = find(App.get_sensors(), "Battery");
  this->charger_ = find(App.get_switches(), "Charger");
  this->wake_ = find(App.get_buttons(), "Wake up");
  this->limit_ = find(App.get_numbers(), "Charging Limit");
  // A new limit, from the web page, the car or the Tesla app, re-plans right away.
  if (this->limit_ != nullptr) {
    this->limit_->add_on_state_callback([this](float value) {
      if (value != this->last_limit_) {
        this->last_limit_ = value;
        this->tick_soon_();
      }
    });
  }
#ifdef USE_COVER
  // The charge port flap, which the car reports even while it sleeps.
  this->port_ = find(App.get_covers(), "Charge Port Door");
  if (this->port_ != nullptr)
    this->port_->add_on_state_callback([this]() { this->port_reported_ = true; });
#endif
}

void ChargingComponent::update() {
  const ESPTime now = this->clock_->now();
  CarState car;
  car.now = now.is_valid() ? now.timestamp : 0;
  if (this->plug_ != nullptr && this->plug_->has_state())
    car.plugged = this->plug_->state;
  if (this->charging_state_ != nullptr && this->charging_state_->has_state())
    car.charging_state = this->charging_state_->state;
  car.soc = this->battery_ != nullptr ? this->battery_->state : NAN;
  car.limit = this->limit_ != nullptr && this->limit_->has_state() ? this->limit_->state : NAN;
#ifdef USE_COVER
  car.port_open = this->port_reported_ && this->port_->position == cover::COVER_OPEN;
#endif

  // The clock's time zone without summer time: charging.h applies EU summer time itself.
  Settings &settings = this->settings_;
  settings.standard_offset = ESPTime::timezone_offset() - (now.is_dst ? 3600 : 0);
  settings.ready_by_minutes = this->ready_by_->hour * 60 + this->ready_by_->minute;
  const ReadyByOnce &once = *this->ready_by_once_;
  settings.ready_by_once = once.year >= 2020 ? local_to_utc(days_from_civil(once.year, once.month, once.day),
                                                            once.hour * 60 + once.minute, settings.standard_offset)
                                             : 0;

  // The clock keeps running through a restart, so wait for the network too.
  if (network::is_connected() && this->controller_.fetch_prices_due(car.now))
    this->fetch_prices_(car.now);

  Decision d = this->controller_.tick(car, settings);
  if (this->controller_.held_mode() != this->held_) {
    this->held_ = this->controller_.held_mode();
    this->held_pref_.save(&this->held_);
    global_preferences->sync();  // now, in case the board restarts soon after
  }
  if (d.notification)
    this->send_message_(*d.notification);
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
  if (this->charger_ == nullptr || this->plug_ == nullptr)
    d.status = "Tesla entities not found";

  auto publish = [](text_sensor::TextSensor *sensor, const std::string &value) {
    if (sensor->state != value)
      sensor->publish_state(value);
  };
  publish(this->status_, d.status);
  publish(this->mode_, d.mode);
  publish(this->windows_, d.windows);
  publish(this->prices_until_, std::to_string(this->controller_.prices.known_until(car.now)));
}

void ChargingComponent::dump_config() {
  ESP_LOGCONFIG(TAG,
                "Charging:\n"
                "  Nord Pool area: %s\n"
                "  Battery: %.0f kWh\n"
                "  Charging power: %.1f kW\n"
                "  Phone messages: %s",
                this->area_, this->settings_.capacity_kwh, this->settings_.charge_kw,
                this->ntfy_topic_[0] != '\0' ? "on" : "off");
  LOG_UPDATE_INTERVAL(this);
}

void ChargingComponent::replan() {
  this->controller_.replan();
  this->tick_soon_();
}

void ChargingComponent::press(Action action) {
  switch (action) {
    case Action::CREATE_PLAN:
      this->controller_.create_plan();
      break;
    case Action::CHARGE_NOW:
      this->controller_.charge_now();
      break;
    case Action::STOP_CHARGING:
      this->controller_.stop_charging();
      break;
  }
  this->tick_soon_();
}

// On the next loop, so a tick never runs inside another entity's callback.
void ChargingComponent::tick_soon_() {
  this->defer("tick", [this]() { this->update(); });
}

// Today's and tomorrow's CET delivery days, today's only while some of it is missing. A day not published
// yet answers 204. A failed request ends the try: the next one would fail the same way and block the loop
// again.
void ChargingComponent::fetch_prices_(int64_t now) {
  for (int day = this->controller_.prices.known_until(now) >= end_of_delivery_day(now) ? 1 : 0; day < 2; day++) {
    auto response = this->http_->get(nord_pool_url(this->area_, now, day));
    if (response == nullptr) {
      ESP_LOGW(TAG, "Nord Pool request failed");
      break;
    }
    if (response->status_code == http_request::HTTP_STATUS_OK) {
      std::string body;
      uint8_t chunk[512];
      uint32_t last_data = millis();
      while (body.size() < MAX_PRICES_BYTES) {
        const int read = response->read(chunk, std::min(sizeof(chunk), MAX_PRICES_BYTES - body.size()));
        App.feed_wdt();
        yield();
        const auto result = http_request::http_read_loop_result(read, last_data, this->http_->get_timeout(),
                                                                response->is_read_complete());
        if (result == http_request::HttpReadLoopResult::RETRY)
          continue;
        if (result != http_request::HttpReadLoopResult::DATA)
          break;
        body.append(reinterpret_cast<const char *>(chunk), read);
      }
      response->end();  // before the parse: the connection's memory isn't needed any more
      const int stored = this->controller_.prices.add_nord_pool(body.data(), body.size(), this->area_);
      if (stored < 0) {
        ESP_LOGW(TAG, "Nord Pool: could not parse %u bytes (cut off?)", static_cast<unsigned>(body.size()));
      } else if (stored == 0) {
        ESP_LOGW(TAG, "Nord Pool: no prices for %s in the answer: check nordpool: area", this->area_);
      } else {
        ESP_LOGI(TAG, "Nord Pool: stored %d quarter-hours", stored);
        this->controller_.replan();
      }
    } else if (response->status_code != http_request::HTTP_STATUS_NO_CONTENT) {
      ESP_LOGW(TAG, "Nord Pool answered HTTP %d", response->status_code);
    } else if (day == 0) {  // today's prices are always out: tomorrow's may not be yet
      ESP_LOGW(TAG, "Nord Pool has no prices for %s today: check nordpool: area", this->area_);
    }
    response->end();
  }
}

void ChargingComponent::send_message_(const Notification &message) {
  if (this->ntfy_topic_[0] == '\0')
    return;
  const std::string body = json::build_json([this, &message](JsonObject root) {
    root["topic"] = this->ntfy_topic_;
    root["title"] = message.title;
    root["message"] = message.message;
    root["tags"].to<JsonArray>().add("electric_plug");
    root["click"] = "http://" + App.get_name() + ".local";  // a tap opens the page, on the home Wi-Fi
  });
  auto response = this->http_->post(this->ntfy_server_, body);
  if (response == nullptr) {
    ESP_LOGW(TAG, "ntfy message failed");
    return;
  }
  if (response->status_code != http_request::HTTP_STATUS_OK)
    ESP_LOGW(TAG, "ntfy answered HTTP %d", response->status_code);
  response->end();
}

}  // namespace esphome::charging
