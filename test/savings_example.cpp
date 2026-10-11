// The README's Example of yearly savings: what charging cost every night of a year, from 1 October 2025, when the
// market prices went to quarter-hours, to 30 September 2026, in Germany and in Lithuania, on the published market
// prices, with the board's own plans and scheduler (make_schedule()) and 2026's fees, taxes and margins all year,
// against charging at once and the car's own schedule, from when the plan's cheapest fee starts at night and from the
// start time that cost the least over those nights. Built like the unit tests (CONTRIBUTING.md's Checks) and run from
// the repository root, as it reads plans/; it downloads the prices with curl into test/.prices the first time:
//   g++ -std=c++17 -I . -isystem ArduinoJson/src test/savings_example.cpp -o test/savings_example
//   test/savings_example
#include "scheduler/market.h"
#include "scheduler/schedule.h"
#include "scheduler/tariff.h"

#include <spawn.h>
#include <sys/wait.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

extern char **environ;  // NOLINT(readability-redundant-declaration): Linux declares it, macOS doesn't

using namespace esphome::scheduler;

namespace {

// The car, the same every night: 20,000 km a year, about 55 km a day, at 15 kWh per 100 km, so home at 18:00 with 69%,
// and 80% by 07:00.
constexpr float KM_A_YEAR = 20000.0f;
constexpr float KWH_PER_KM = 0.15f;
constexpr float BATTERY_KWH = 75.0f;
constexpr float CHARGING_KW = 11.0f;
constexpr float LIMIT = 80.0f;
constexpr float SOC = LIMIT - KM_A_YEAR / 365 * KWH_PER_KM / BATTERY_KWH * 100;
constexpr int PLUG_IN = 18 * 60;         // local time, in minutes
constexpr int READY_BY = (24 + 7) * 60;  // the next morning
constexpr int SMARD_GERMANY = 4169;      // SMARD's number for the day-ahead prices of Germany and Luxembourg

struct Country {
  const char *name;
  const char *plan;  // in plans/, with 2026's prices, used all year
  const char *prices_from;
  float vat;       // on the market price
  float margin;    // the supplier's, per kWh with VAT
  float flat;      // the other charges per kWh with VAT, the same every quarter-hour, which plans leave out
  int cheap_from;  // when the plan's cheapest fee starts at night, in minutes, where an owner would set the car's own
                   // schedule
  int32_t standard_offset;
};

// The file `url` answers, downloaded once into test/.prices/`name`.
std::string download(const std::string &url, const std::string &name) {
  const std::filesystem::path path = std::filesystem::path("test/.prices") / name;
  if (!std::filesystem::exists(path)) {
    std::filesystem::create_directories(path.parent_path());
    const std::string part = path.string() + ".part";
    const char *args[] = {"curl", "--proto", "=https",     "-sSf",      "--retry",
                          "3",    "-o",      part.c_str(), url.c_str(), nullptr};
    pid_t pid = 0;
    int status = 0;
    if (posix_spawnp(&pid, "curl", nullptr, nullptr, const_cast<char **>(args), environ) != 0 ||
        waitpid(pid, &status, 0) != pid || status != 0) {
      std::fprintf(stderr, "Couldn't download %s\n", url.c_str());
      std::exit(1);
    }
    std::filesystem::rename(part, path);
  }
  std::ifstream file(path);
  std::stringstream text;
  text << file.rdbuf();
  return text.str();
}

// Germany's prices from SMARD, the Federal Network Agency's, a week a file: the day-ahead auction's, as Nord Pool's
// for the area, which the board downloads, are.
PriceTable german_prices(int64_t first_day, int64_t last_day) {
  std::set<std::string> urls;
  for (int64_t day = first_day; day <= last_day; ++day)
    urls.insert(smard_url(SMARD_GERMANY, local_to_utc(day, 12 * 60, CET_STANDARD_OFFSET), 0));
  PriceTable prices;
  for (const std::string &url : urls) {
    const std::string json = download(url, url.substr(url.rfind('/') + 1));
    if (prices.add_smard(json.data(), json.size()) < 0) {
      std::fprintf(stderr, "Couldn't read %s\n", url.c_str());
      std::exit(1);
    }
  }
  return prices;
}

// Lithuania's prices from Elering, Estonia's grid operator, a month a file: Nord Pool's for the Baltic areas, which it
// republishes, as Nord Pool serves only its last two months without an account.
PriceTable lithuanian_prices(int64_t first_day, int64_t last_day) {
  PriceTable prices;
  const CivilDate first = civil_from_days(first_day), last = civil_from_days(last_day);
  for (int64_t month = first.year * 12 + first.month - 1; month <= last.year * 12 + last.month - 1; ++month) {
    const auto year = static_cast<int>(month / 12), from = static_cast<int>(month % 12) + 1;
    const int next = from % 12 + 1, next_year = year + (from == 12 ? 1 : 0);
    char url[160], name[32];
    std::snprintf(url, sizeof(url),
                  "https://dashboard.elering.ee/api/nps/price?start=%04d-%02d-01T00:00:00.000Z"
                  "&end=%04d-%02d-01T00:00:00.000Z",
                  year, from, next_year, next);
    std::snprintf(name, sizeof(name), "elering_%04d-%02d.json", year, from);
    const std::string json = download(url, name);
    JsonDocument filter;
    filter["data"]["lt"][0] = true;
    JsonDocument doc;
    if (deserializeJson(doc, json, DeserializationOption::Filter(filter)) != DeserializationError::Ok) {
      std::fprintf(stderr, "Couldn't read %s\n", url);
      std::exit(1);
    }
    for (JsonObject entry : doc["data"]["lt"].as<JsonArray>())
      prices.set(entry["timestamp"].as<int64_t>(), entry["price"].as<float>() / 1000.0f);
  }
  return prices;
}

// What `grid_kwh` costs from `start`, straight through at CHARGING_KW, as the car charges at once or on its own
// schedule.
double straight_cost(const PriceTable &prices, const Tariff &tariff, const Country &country, int64_t start,
                     float grid_kwh) {
  double cost = 0;
  for (int64_t slot = start; grid_kwh > 1e-4f; slot += SLOT_SECONDS) {
    const std::optional<float> price = prices.get(slot);
    if (!price) {
      std::fprintf(stderr, "%s: no price at %lld\n", country.name, static_cast<long long>(slot));
      std::exit(1);
    }
    const float kwh = std::min(grid_kwh, CHARGING_KW * SLOT_SECONDS / 3600.0f);
    cost += kwh * (total_price(*price, slot, tariff, country.standard_offset) + country.flat);
    grid_kwh -= kwh;
  }
  return cost;
}

std::string date(int64_t day) {
  const CivilDate d = civil_from_days(day);
  char text[16];
  std::snprintf(text, sizeof(text), "%04d-%02u-%02u", static_cast<int>(d.year), d.month, d.day);
  return text;
}

std::string hhmm(int minutes) {
  char text[8];
  std::snprintf(text, sizeof(text), "%02d:%02d", minutes / 60 % 24, minutes % 60);
  return text;
}

void run(const Country &country, const PriceTable &prices, int64_t first_night, int64_t last_night) {
  std::ifstream file(std::string("plans/") + country.plan + ".yaml");
  std::stringstream plan;
  plan << file.rdbuf();
  Tariff tariff;
  const std::string error = make_tariff(plan.str(), "EUR", tariff);
  if (!error.empty()) {
    std::fprintf(stderr, "%s: %s\n", country.plan, error.c_str());
    std::exit(1);
  }
  tariff.vat = country.vat;
  tariff.margin = country.margin;
  Settings settings;
  settings.battery_kwh = BATTERY_KWH;
  settings.charging_kw = CHARGING_KW;
  settings.standard_offset = country.standard_offset;

  const int minutes = static_cast<int>(std::ceil((LIMIT - SOC) / 100.0f * BATTERY_KWH / EFFICIENCY / CHARGING_KW * 60));
  std::map<int, double> straight;  // by the start time, in minutes after midnight before the night
  double board = 0, energy = 0;
  for (int64_t day = first_night; day <= last_night; ++day) {
    ScheduleRequest request;
    request.now = local_to_utc(day, PLUG_IN, country.standard_offset);
    request.deadline = local_to_utc(day, READY_BY, country.standard_offset);
    request.soc = SOC;
    request.limit = LIMIT;
    request.tariff = tariff;
    request.settings = settings;
    const Schedule schedule = make_schedule(prices, request);
    if (!schedule.valid || schedule.unpriced_slots > 0 || schedule.soc_at_end < LIMIT - FULL_WITHIN) {
      std::fprintf(stderr, "%s: no full schedule on %s\n", country.name, date(day).c_str());
      std::exit(1);
    }
    float kwh = 0;
    for (const Window &w : schedule.windows) {
      board += w.cost + w.energy_kwh * country.flat;
      kwh += w.energy_kwh;
    }
    energy += kwh;
    // The same energy straight through: the board may leave the last 0.1% to a window it doesn't start (FULL_WITHIN).
    for (int start = PLUG_IN; start + minutes <= READY_BY; start += SLOT_SECONDS / 60)
      straight[start] += straight_cost(prices, tariff, country, local_to_utc(day, start, country.standard_offset), kwh);
  }

  const auto best = std::min_element(straight.begin(), straight.end(),
                                     [](const auto &a, const auto &b) { return a.second < b.second; });
  const double at_once = straight.at(PLUG_IN), cheap = straight.at(country.cheap_from);
  const auto row = [energy](const std::string &label, double cost) {
    std::printf("  %-42s %8.2f EUR  %.3f EUR/kWh\n", label.c_str(), cost, cost / energy);
  };
  std::printf("%s, %s, on %s's prices: %lld nights from %s to %s, %.0f kWh from the grid\n", country.name, country.plan,
              country.prices_from, static_cast<long long>(last_night - first_night + 1), date(first_night).c_str(),
              date(last_night).c_str(), energy);
  row("Charging at once, from " + hhmm(PLUG_IN), at_once);
  row("The car's own schedule, from " + hhmm(country.cheap_from), cheap);
  row("The best start, found afterwards, " + hhmm(best->first), best->second);
  row("This board", board);
  std::printf("  This board's costs %.0f%%, %.0f%% and %.0f%% less\n\n", 100 * (1 - board / at_once),
              100 * (1 - board / cheap), 100 * (1 - board / best->second));
}

}  // namespace

int main() try {
  const int64_t first_night = days_from_civil(2025, 10, 1), last_night = days_from_civil(2026, 9, 30);
  // Germany in 2026, with 19% VAT: a dynamic contract's margin of 2 ct, about the middle of the offers', and per kWh
  // the electricity tax, 2.05 ct (§ 3 StromStG), the concession fee of a town of up to 100,000, 1.59 ct (§ 2 KAV), the
  // median home's in Westnetz's area, and the levies, KWKG 0.446 ct, offshore 0.941 ct and § 19 StromNEV 1.559 ct
  // (netztransparenz.de). The low fee of Westnetz's Modul 3 starts at 00:00.
  const Country germany{"Germany", "de/westnetz-modul-3", "SMARD", 0.19f, 0.020f, 0.07837f,
                        24 * 60,   CET_STANDARD_OFFSET};
  // Lithuania in 2026, with 21% VAT: an exchange-price contract's margin of 2.5 ct, about the middle of the three
  // suppliers' (VERT's comparison, skaiciuokle.vert.lt), and per kWh VIAP, -0.044 ct, and the household extra
  // component, 0.0223 ct (VERT), with no excise on households. ESO's night fee starts at 22:00.
  const Country lithuania{"Lithuania", "lt/eso-standartinis-4-zones", "Elering", 0.21f, 0.025f, -0.00026f,
                          22 * 60,     VILNIUS_STANDARD_OFFSET};
  run(germany, german_prices(first_night, last_night + 1), first_night, last_night);
  run(lithuania, lithuanian_prices(first_night, last_night + 1), first_night, last_night);
  return 0;
} catch (const std::exception &e) {  // a file it couldn't write or read
  std::fprintf(stderr, "%s\n", e.what());
  return 1;
}
