// The README's Example of yearly savings: what charging cost every night of a year, from 1 October 2025, when the
// market prices went to quarter-hours, to 30 September 2026, in Germany, Lithuania and Poland, on the published market
// prices, with the board's own plans and scheduler (make_schedule()) and 2026's fees, taxes and margins all year,
// against charging at once and the car's own schedule, from when the plan's cheapest fee starts at night and from the
// start time that cost the least over those nights; Poland's in złoty, as its board counts, and in euros at the ECB's
// daily rate. Built like the unit tests (CONTRIBUTING.md's Checks) and run from
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
constexpr int SMARD_GERMANY = 4169;      // SMARD's numbers for the day-ahead prices of Germany and Luxembourg,
constexpr int SMARD_POLAND = 257;        // and of Poland

struct Country {
  const char *name;
  const char *plan;  // in plans/, with 2026's prices, used all year
  const char *prices_from;
  const char *currency;  // the plan's, which the board counts in
  float vat;             // on the market price
  float margin;          // the supplier's, per kWh with VAT
  float flat;            // the other charges per kWh with VAT, the same every quarter-hour, which plans leave out
  int cheap_from;  // when the plan's cheapest fee starts at night, in minutes, where an owner would set the car's own
                   // schedule
  int32_t standard_offset;
};

std::string date(int64_t day) {
  const CivilDate d = civil_from_days(day);
  char text[16];
  std::snprintf(text, sizeof(text), "%04d-%02u-%02u", static_cast<int>(d.year), d.month, d.day);
  return text;
}

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

// The ECB's rates of the euro in `currency` by day, from the week before `first_day`; days without one, like
// weekends, take the one before.
using Rates = std::map<int64_t, double>;
Rates ecb_rates(const char *currency, int64_t first_day, int64_t last_day) {
  const std::string from = date(first_day - 7), to = date(last_day);
  const std::string text = download(std::string("https://data-api.ecb.europa.eu/service/data/EXR/D.") + currency +
                                        ".EUR.SP00.A?startPeriod=" + from + "&endPeriod=" + to + "&format=csvdata",
                                    std::string("ecb_") + currency + "_" + from + "_" + to + ".csv");
  Rates rates;
  std::istringstream lines(text);
  std::string line;
  std::getline(lines, line);  // the heading: KEY,FREQ,CURRENCY,CURRENCY_DENOM,EXR_TYPE,EXR_SUFFIX,TIME_PERIOD,OBS_VALUE
  while (std::getline(lines, line)) {
    int year, month, day;
    double rate;
    size_t at = 0;
    for (int comma = 0; comma < 6 && at != std::string::npos; ++comma)
      at = line.find(',', at + 1);
    if (at != std::string::npos && std::sscanf(line.c_str() + at + 1, "%d-%d-%d,%lf", &year, &month, &day, &rate) == 4)
      rates[days_from_civil(year, month, day)] = rate;
  }
  if (rates.empty()) {
    std::fprintf(stderr, "No ECB rates for %s\n", currency);
    std::exit(1);
  }
  return rates;
}

double rate_on(const Rates &rates, int64_t day) {
  const auto after = rates.upper_bound(day);
  if (after == rates.begin()) {
    std::fprintf(stderr, "No ECB rate on %s\n", date(day).c_str());
    std::exit(1);
  }
  return std::prev(after)->second;
}

// SMARD's prices, the Federal Network Agency's, a week a file, in euros: the day-ahead auction's, as Nord Pool's for
// the area, which the board downloads, are.
PriceTable smard_prices(int filter, int64_t first_day, int64_t last_day) {
  std::set<std::string> urls;
  for (int64_t day = first_day; day <= last_day; ++day)
    urls.insert(smard_url(filter, local_to_utc(day, 12 * 60, CET_STANDARD_OFFSET), 0));
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

// The prices in another currency, each delivery day's at that day's rate, as Nord Pool's in złoty are.
PriceTable in_currency(const PriceTable &euros, const Rates &rates, int64_t first_day, int64_t last_day) {
  PriceTable prices;
  for (int64_t slot = local_to_utc(first_day, 0, CET_STANDARD_OFFSET);
       slot < local_to_utc(last_day + 1, 0, CET_STANDARD_OFFSET); slot += SLOT_SECONDS)
    if (const std::optional<float> price = euros.get(slot))
      prices.set(slot, static_cast<float>(*price * rate_on(rates, local_day_of(slot, CET_STANDARD_OFFSET))));
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

std::string hhmm(int minutes) {
  char text[8];
  std::snprintf(text, sizeof(text), "%02d:%02d", minutes / 60 % 24, minutes % 60);
  return text;
}

void run(const Country &country, const PriceTable &prices, const Rates &rates, int64_t first_night,
         int64_t last_night) {
  std::ifstream file(std::string("plans/") + country.plan + ".yaml");
  std::stringstream plan;
  plan << file.rdbuf();
  Tariff tariff;
  const std::string error = make_tariff(plan.str(), country.currency, tariff);
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
  // By the start time, in minutes after midnight before the night, in the plan's currency and in euros.
  std::map<int, double> straight, straight_euros;
  double board = 0, board_euros = 0, energy = 0;
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
    const double rate = std::string(country.currency) == "EUR" ? 1 : rate_on(rates, day);
    float kwh = 0;
    double cost = 0;
    for (const Window &w : schedule.windows) {
      cost += w.cost + w.energy_kwh * country.flat;
      kwh += w.energy_kwh;
    }
    board += cost;
    board_euros += cost / rate;
    energy += kwh;
    // The same energy straight through: the board may leave the last 0.1% to a window it doesn't start (FULL_WITHIN).
    for (int start = PLUG_IN; start + minutes <= READY_BY; start += SLOT_SECONDS / 60) {
      const double straight_night =
          straight_cost(prices, tariff, country, local_to_utc(day, start, country.standard_offset), kwh);
      straight[start] += straight_night;
      straight_euros[start] += straight_night / rate;
    }
  }

  const auto best = std::min_element(straight.begin(), straight.end(),
                                     [](const auto &a, const auto &b) { return a.second < b.second; });
  const double at_once = straight.at(PLUG_IN), cheap = straight.at(country.cheap_from);
  const auto row = [&country, energy](const std::string &label, double cost, double euros) {
    std::printf("  %-42s %8.2f %s  %.3f %s/kWh", label.c_str(), cost, country.currency, cost / energy,
                country.currency);
    if (std::string(country.currency) != "EUR")
      std::printf("  %8.2f EUR", euros);
    std::printf("\n");
  };
  std::printf("%s, %s, on %s's prices: %lld nights from %s to %s, %.0f kWh from the grid\n", country.name, country.plan,
              country.prices_from, static_cast<long long>(last_night - first_night + 1), date(first_night).c_str(),
              date(last_night).c_str(), energy);
  row("Charging at once, from " + hhmm(PLUG_IN), at_once, straight_euros.at(PLUG_IN));
  row("The car's own schedule, from " + hhmm(country.cheap_from), cheap, straight_euros.at(country.cheap_from));
  row("The best start, found afterwards, " + hhmm(best->first), best->second, straight_euros.at(best->first));
  row("This board", board, board_euros);
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
  const Country germany{"Germany", "de/westnetz-modul-3", "SMARD", "EUR", 0.19f, 0.020f, 0.07837f,
                        24 * 60,   CET_STANDARD_OFFSET};
  // Lithuania in 2026, with 21% VAT: an exchange-price contract's margin of 2.5 ct, about the middle of the three
  // suppliers' (VERT's comparison, skaiciuokle.vert.lt), and per kWh VIAP, -0.044 ct, and the household extra
  // component, 0.0223 ct (VERT), with no excise on households. ESO's night fee starts at 22:00.
  const Country lithuania{"Lithuania", "lt/eso-standartinis-4-zones", "Elering", "EUR", 0.21f, 0.025f, -0.00026f,
                          22 * 60,     VILNIUS_STANDARD_OFFSET};
  // Poland in 2026, with 23% VAT: the margin of Tauron's own dynamic offer, 0.0842 zł, about the middle of the big
  // suppliers' in the Energy Regulatory Office's comparison of offers, and per kWh the quality fee, 0.0332 zł (PSE's
  // tariff), the renewable energy fee, 0.0073 zł, the cogeneration fee, 0.0030 zł (Dz.U. 2025 poz. 1664), and excise,
  // 0.0050 zł. Tauron Dystrybucja's G12 night fee starts at 22:00 winter time, so by 23:00 on the clock all year.
  const Country poland{"Poland", "pl/tauron-g12",    "SMARD", "PLN", 0.23f, 0.10357f, 0.059655f,
                       23 * 60,  CET_STANDARD_OFFSET};
  run(germany, smard_prices(SMARD_GERMANY, first_night, last_night + 1), {}, first_night, last_night);
  run(lithuania, lithuanian_prices(first_night, last_night + 1), {}, first_night, last_night);
  const Rates zloty = ecb_rates("PLN", first_night, last_night + 1);
  run(poland, in_currency(smard_prices(SMARD_POLAND, first_night, last_night + 1), zloty, first_night, last_night + 1),
      zloty, first_night, last_night);
  return 0;
} catch (const std::exception &e) {  // a file it couldn't write or read
  std::fprintf(stderr, "%s\n", e.what());
  return 1;
}
