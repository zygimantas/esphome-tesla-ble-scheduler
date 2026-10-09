// The settings page's routes, on the web server's task: /settings and /settings/options.
#include "scheduler_component.h"

#ifdef USE_WEBSERVER
#include <sys/select.h>
#include <array>

namespace esphome::scheduler {

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

}  // namespace esphome::scheduler
#endif
