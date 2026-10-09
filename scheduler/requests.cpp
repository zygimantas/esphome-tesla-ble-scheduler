// The requests task: the downloads of the market's prices, the ECB's rate and the plan, and the phone message, as
// batches (Requests), each answer handed to the loop.
#include "scheduler_component.h"

#include "esphome/components/json/json_util.h"
#include "esphome/components/network/util.h"
#include "esphome/core/log.h"

namespace esphome::scheduler {

static const char *const TAG = "scheduler";

// The most the board reads of an answer: a day of LT prices is about 11 kB, SMARD's week 15 kB and a plan up to
// about 2 kB. OKTE's day, 67 kB, and SEMOpx's, 20 kB, go through a filter as they come instead (read_body_()).
static constexpr size_t MAX_BODY_BYTES = 24 * 1024;
// The plans as their maintainers keep them current, on GitHub.
static const char *const PLANS = "https://raw.githubusercontent.com/zygimantas/esphome-tesla-ble-scheduler/main/plans/";
// Phone messages go through ntfy's own server, to the topic the settings name.
static const char *const NTFY = "https://ntfy.sh";

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
    while (count < length) {
      const int c = this->read();
      if (c < 0)
        break;
      buffer[count++] = static_cast<char>(c);
    }
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
// With a `filter`, what it keeps of the JSON, read as it comes, so the whole answer needn't fit: as JSON, or what
// `keep` makes of it; nothing when it can't be read. Without App.feed_wdt(): the watchdog doesn't watch the requests
// task, and on the loop the read feeds it.
std::optional<std::string> SchedulerComponent::read_body_(http_request::HttpContainer &response,
                                                          const JsonDocument *filter, const Keep &keep) {
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
    if (keep)
      return keep(doc);
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
// cut off, through `filter` and `keep` if any (read_body_()). The response ends before the body is parsed: the
// connection's memory isn't needed any more.
std::optional<int> SchedulerComponent::fetch_(const std::string &url, std::optional<std::string> &body,
                                              const JsonDocument *filter, const Keep &keep) {
  auto response = this->http_->get(url);
  if (response == nullptr)
    return std::nullopt;
  const int status = response->status_code;
  if (status == http_request::HTTP_STATUS_OK)
    body = this->read_body_(*response, filter, keep);
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
    case Market::SEMOPX:
      return semopx_url(batch.semopx_list.data(), batch.semopx_list.size(), batch.now, day);
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
    case Market::SEMOPX:
      return prices.add_semopx(body.data(), body.size(), area.source_name);
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
  // SEMOpx gives a day's results by an id in its list of the newest: the list first, for both days
  if (area.market == Market::SEMOPX) {
    std::optional<std::string> list;
    if (this->fetch_(SEMOPX_LIST_URL, list) != http_request::HTTP_STATUS_OK || !list) {
      ESP_LOGW(TAG, "%s: its list of results couldn't be read", source);
      return;
    }
    this->requests_.semopx_list = std::move(*list);
  }
  // Not out yet: Nord Pool has no content for tomorrow, SMARD no file for a week that hasn't begun, OMIE none for the
  // day, OKTE an empty list, which stores none, and SEMOpx's list no results for the day.
  const int not_yet =
      area.market == Market::NORD_POOL ? http_request::HTTP_STATUS_NO_CONTENT : http_request::HTTP_STATUS_NOT_FOUND;
  // OKTE's and SEMOpx's answers come in through a filter, as they're too long to hold, and SEMOpx's to the area's
  // prices
  const bool filtered = area.market == Market::OKTE || area.market == Market::SEMOPX;
  const JsonDocument filter = area.market == Market::OKTE ? okte_filter() : semopx_filter();
  Keep keep;
  if (area.market == Market::SEMOPX)
    keep = [&area](const JsonDocument &doc) { return semopx_kept(doc, area.source_name); };
  std::string fetched;  // SMARD's file has the week, so tomorrow's prices are often in today's
  for (int day = this->requests_.first_day; day < 2; day++) {
    const std::string url = this->prices_url_(day);
    if (url.empty()) {  // not in SEMOpx's list
      if (day == 0)
        ESP_LOGW(TAG, "%s has no prices for %s today: check Country / Area under Settings", source, area.name);
      continue;
    }
    if (url == fetched)
      continue;
    fetched = url;
    std::optional<std::string> body;
    const std::optional<int> status = this->fetch_(url, body, filtered ? &filter : nullptr, keep);
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

}  // namespace esphome::scheduler
