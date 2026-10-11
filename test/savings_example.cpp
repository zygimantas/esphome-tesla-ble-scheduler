// The README's Example of yearly savings and its countries: what charging cost every night of a year, from 1 October
// 2025, when the market prices went to quarter-hours, to 30 September 2026, on the published market prices, on every
// plan in plans/ of the countries where homes pay them, else on the regulated or fixed price, with the board's own
// scheduler (make_schedule()) and 2026's fees, taxes and margins all year, against charging at once and the car's own
// schedule, from when the plan's cheapest fee starts at night, or midnight where it's the same all night, and from the
// start time that cost the least over those nights. It prints the README's list of countries last. Each country's
// figures and their sources are in plans/<country>/AGENTS.md. Built like the unit tests (CONTRIBUTING.md's Checks) and
// run from the repository root, as it reads plans/; it downloads the prices with curl into test/.prices the first time:
//   g++ -std=c++17 -I . -isystem ArduinoJson/src test/savings_example.cpp -o test/savings_example
//   test/savings_example
#include "scheduler/market.h"
#include "scheduler/schedule.h"
#include "scheduler/tariff.h"

#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

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

// Where a country's market prices come from, in euros, or NONE where households pay regulated or fixed prices, which a
// plan then holds.
enum class Source { SMARD, ELERING, OMIE, ELPRISETJUSTNU, ENERGY_CHARTS, NONE };

struct Country {
  const char *name;
  const char *code;  // its folder in plans/, each of whose plans it takes in turn, with 2026's prices all year
  Source source;
  const char *area;      // in the source: SMARD's series, Elering's, elprisetjustnu.se's or Energy-Charts' area, OMIE's
  const char *currency;  // the plans', which the board counts in
  float vat;             // on the market price
  float margin;          // the supplier's, per kWh with VAT
  float flat;            // the other charges per kWh with VAT, the same every quarter-hour, which plans leave out
  int32_t standard_offset;
  const char *confidence;           // the README's, from the research in plans/<code>/AGENTS.md
  float factor = 1;                 // on all of it, for a tax that's a share of the bill, as Spain's is
  float market_factor = 1;          // on the market price, for the grid losses a supplier passes on, as Spain's do
  bool hourly = false;              // billed at each hour's average price, as meters that record hours are
  const char *plan_text = nullptr;  // its one plan instead, the whole price, where homes pay a regulated or fixed one
  float support_share = 0;          // what the state pays back of the market price above `support_from`, without
  float support_from = 0;           // VAT, as Norway's strømstøtte does, each hour
  std::string plan{};               // this run's, like lt/eso-standartinis-4-zones
  std::string area_name{};          // this plan's market area, where the country has several
};

// What a plan changes in its country's figures: its market area, where the country has several, the one most of its
// homes are in, and what only some plans' homes pay: Vienna's levy, and VAT, which Norway's north doesn't.
void take_plan(Country &c, const std::string &plan) {
  c.plan = plan;
  struct Area {
    const char *plan, *area, *name;
  };
  static const Area AREAS[] = {
      {"dk/cerius", "253", "DK2"},  {"dk/radius", "253", "DK2"},      {"dk/", "252", "DK1"},
      {"no/arva", "NO4", "NO4"},    {"no/bkk", "NO5", "NO5"},         {"no/elvia", "NO1", "NO1"},
      {"no/fagne", "NO2", "NO2"},   {"no/glitre", "NO2", "NO2"},      {"no/lede", "NO1", "NO1"},
      {"no/lnett", "NO2", "NO2"},   {"no/tensio", "NO3", "NO3"},      {"se/eon-nord", "SE2", "SE2"},
      {"se/eon-syd", "SE4", "SE4"}, {"se/kraftringen", "SE4", "SE4"}, {"se/oresundskraft", "SE4", "SE4"},
      {"se/", "SE3", "SE3"},
  };
  for (const Area &a : AREAS)
    if (plan.rfind(a.plan, 0) == 0) {
      c.area = a.area;
      c.area_name = a.name;
      break;
    }
  if (plan == "no/arva-households") {
    c.vat = 0;         // and so the support adds none
    c.flat = 0.0813f;  // the electricity tax and the Enova fee, without VAT
  }
  if (std::string(c.code) == "at" && plan != "at/wiener-netze") {
    c.factor = 1;  // Vienna's Gebrauchsabgabe is Vienna's
    c.flat = 0.00864f;
  }
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

[[noreturn]] void fail(const std::string &what) {
  std::fprintf(stderr, "%s\n", what.c_str());
  std::exit(1);
}

// The file `url` answers, downloaded once into test/.prices/`name`, or nothing if it can't be.
std::optional<std::string> fetch(const std::string &url, const std::string &name) {
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
      std::filesystem::remove(part);
      return std::nullopt;
    }
    std::filesystem::rename(part, path);
  }
  std::ifstream file(path);
  std::stringstream text;
  text << file.rdbuf();
  return text.str();
}

std::string download(const std::string &url, const std::string &name) {
  std::optional<std::string> text = fetch(url, name);
  if (!text)
    fail("Couldn't download " + url);
  return *text;
}

// The time given as text like 2026-01-01T00:00:00+01:00.
std::optional<int64_t> parse_offset_time(const char *text) {
  int year, month, day, hour, minute, second, offset_hours, offset_minutes;
  char sign;
  if (text == nullptr || std::sscanf(text, "%d-%d-%dT%d:%d:%d%c%d:%d", &year, &month, &day, &hour, &minute, &second,
                                     &sign, &offset_hours, &offset_minutes) != 9)
    return std::nullopt;
  const int64_t offset = (sign == '-' ? -1 : 1) * (offset_hours * 3600 + offset_minutes * 60);
  return days_from_civil(year, month, day) * DAY_SECONDS + hour * 3600 + minute * 60 + second - offset;
}

// SMARD's, the Federal Network Agency's, a week a file, as the board downloads them for some areas.
void add_smard(PriceTable &prices, const char *series, int64_t first_day, int64_t last_day) {
  std::set<std::string> urls;
  for (int64_t day = first_day; day <= last_day; ++day)
    urls.insert(smard_url(std::atoi(series), local_to_utc(day, 12 * 60, CET_STANDARD_OFFSET), 0));
  for (const std::string &url : urls) {
    const std::string json = download(url, url.substr(url.rfind('/') + 1));
    if (prices.add_smard(json.data(), json.size()) < 0)
      fail("Couldn't read " + url);
  }
}

// Elering's, Estonia's grid operator's, which republishes Nord Pool's for the Baltic areas and Finland, a month a file.
void add_elering(PriceTable &prices, const char *area, int64_t first_day, int64_t last_day) {
  const CivilDate first = civil_from_days(first_day), last = civil_from_days(last_day);
  for (int64_t month = first.year * 12 + first.month - 1; month <= last.year * 12 + last.month - 1; ++month) {
    const auto year = static_cast<int>(month / 12), from = static_cast<int>(month % 12) + 1;
    const int next = from % 12 + 1, next_year = year + (from == 12 ? 1 : 0);
    char url[160], name[32];
    std::snprintf(url, sizeof(url),
                  "https://dashboard.elering.ee/api/nps/price?start=%04d-%02d-01T00:00:00.000Z"
                  "&end=%04d-%02d-01T00:00:00.000Z",
                  year, from, next_year, next);
    std::snprintf(name, sizeof(name), "elering_%04d-%02d.json", year, from);  // all four areas in each
    const std::string json = download(url, name);
    JsonDocument filter;
    filter["data"][area][0] = true;
    JsonDocument doc;
    if (deserializeJson(doc, json, DeserializationOption::Filter(filter)) != DeserializationError::Ok)
      fail(std::string("Couldn't read ") + url);
    for (JsonObject entry : doc["data"][area].as<JsonArray>())
      prices.set(entry["timestamp"].as<int64_t>(), entry["price"].as<float>() / 1000.0f);
  }
}

// OMIE's, the Iberian market's, a day a file, as the board downloads and reads them. OMIE keeps only a day's latest
// file, whose number grows when it corrects one, as it did 30 October 2025's.
void add_omie(PriceTable &prices, const char *area, int64_t first_day, int64_t last_day) {
  for (int64_t day = first_day; day <= last_day; ++day) {
    std::string url = omie_url(local_to_utc(day, 12 * 60, CET_STANDARD_OFFSET), 0);
    std::optional<std::string> text;
    for (char version = '1'; version <= '5' && !text; ++version) {
      url.back() = version;
      text = fetch(url, "omie_" + date(day) + ".txt");
    }
    if (!text || prices.add_omie(text->data(), text->size(), area) < 0)
      fail("Couldn't download or read OMIE's prices for " + date(day));
  }
}

// elprisetjustnu.se's, which republishes Nord Pool's for Sweden, a day a file: in euros, as its own kronor come at
// another rate.
void add_elprisetjustnu(PriceTable &prices, const char *area, int64_t first_day, int64_t last_day) {
  for (int64_t day = first_day; day <= last_day; ++day) {
    const CivilDate d = civil_from_days(day);
    char url[96];
    std::snprintf(url, sizeof(url), "https://www.elprisetjustnu.se/api/v1/prices/%04d/%02u-%02u_%s.json",
                  static_cast<int>(d.year), d.month, d.day, area);
    const std::string json = download(url, std::string("elprisetjustnu_") + date(day) + "_" + area + ".json");
    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok)
      fail(std::string("Couldn't read ") + url);
    for (JsonObject entry : doc.as<JsonArray>())
      if (const std::optional<int64_t> start = parse_offset_time(entry["time_start"].as<const char *>()))
        prices.set(*start, entry["EUR_per_kWh"].as<float>());
  }
}

// Energy-Charts', Fraunhofer ISE's, for the areas no other source here has, a month a file. It answers few requests a
// minute, so the program waits before each.
void add_energy_charts(PriceTable &prices, const char *area, int64_t first_day, int64_t last_day) {
  const CivilDate first = civil_from_days(first_day), last = civil_from_days(last_day);
  for (int64_t month = first.year * 12 + first.month - 1; month <= last.year * 12 + last.month - 1; ++month) {
    const auto year = static_cast<int>(month / 12), from = static_cast<int>(month % 12) + 1;
    const int next = from % 12 + 1, next_year = year + (from == 12 ? 1 : 0);
    char url[192], name[48];
    std::snprintf(url, sizeof(url),
                  "https://api.energy-charts.info/price?bzn=%s&start=%04d-%02d-01T00:00Z&end=%04d-%02d-01T00:00Z", area,
                  year, from, next_year, next);
    std::snprintf(name, sizeof(name), "energy-charts_%s_%04d-%02d.json", area, year, from);
    std::optional<std::string> json;
    for (int attempt = 0; attempt < 5 && !json; ++attempt) {
      if (!std::filesystem::exists(std::filesystem::path("test/.prices") / name))
        sleep(10);
      json = fetch(url, name);
    }
    if (!json)
      fail(std::string("Couldn't download ") + url);
    JsonDocument doc;
    if (deserializeJson(doc, *json) != DeserializationError::Ok)
      fail(std::string("Couldn't read ") + url);
    const auto times = doc["unix_seconds"].as<JsonArray>();
    const auto values = doc["price"].as<JsonArray>();
    for (size_t i = 0; i < std::min(times.size(), values.size()); ++i)
      if (values[i].is<float>())
        prices.set(times[i].as<int64_t>(), values[i].as<float>() / 1000.0f);
  }
}

PriceTable market_prices(const Country &country, int64_t first_day, int64_t last_day) {
  PriceTable prices;
  switch (country.source) {
    case Source::SMARD:
      add_smard(prices, country.area, first_day, last_day);
      break;
    case Source::ELERING:
      add_elering(prices, country.area, first_day, last_day);
      break;
    case Source::OMIE:
      add_omie(prices, country.area, first_day, last_day);
      break;
    case Source::ELPRISETJUSTNU:
      add_elprisetjustnu(prices, country.area, first_day, last_day);
      break;
    case Source::ENERGY_CHARTS:
      add_energy_charts(prices, country.area, first_day, last_day);
      break;
    case Source::NONE:
      for (int64_t slot = local_to_utc(first_day, 0, CET_STANDARD_OFFSET);
           slot < local_to_utc(last_day + 1, 0, CET_STANDARD_OFFSET); slot += SLOT_SECONDS)
        prices.set(slot, 0.0f);
      break;
  }
  return prices;
}

// The ECB's rates of the euro in `currency` by day, from the week before `first_day`; days without one, like weekends,
// take the one before.
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
  if (rates.empty())
    fail(std::string("No ECB rates for ") + currency);
  return rates;
}

double rate_on(const Rates &rates, int64_t day) {
  if (rates.empty())
    return 1;
  const auto after = rates.upper_bound(day);
  if (after == rates.begin())
    fail("No ECB rate on " + date(day));
  return std::prev(after)->second;
}

// The prices in another currency, each delivery day's at that day's rate, as Nord Pool's in kronor and złoty are.
PriceTable in_currency(const PriceTable &euros, const Rates &rates, int64_t first_day, int64_t last_day) {
  PriceTable prices;
  for (int64_t slot = local_to_utc(first_day, 0, CET_STANDARD_OFFSET);
       slot < local_to_utc(last_day + 1, 0, CET_STANDARD_OFFSET); slot += SLOT_SECONDS)
    if (const std::optional<float> price = euros.get(slot))
      prices.set(slot, static_cast<float>(*price * rate_on(rates, local_day_of(slot, CET_STANDARD_OFFSET))));
  return prices;
}

// What the bill counts for a kWh in the quarter-hour at `slot`: the market price, as the country bills it, with VAT,
// the margin, the plan's fee and the other charges.
float billed_price(const PriceTable &prices, const Tariff &tariff, const Country &country, int64_t slot) {
  const int64_t from = country.hourly ? floor_div(slot, 3600) * 3600 : slot;
  const int64_t to = country.hourly ? from + 3600 : slot + SLOT_SECONDS;
  float market = 0;
  for (int64_t at = from; at < to; at += SLOT_SECONDS) {
    const std::optional<float> price = prices.get(at);
    if (!price)
      fail(std::string(country.name) + ": no price at " + std::to_string(at));
    market += *price * SLOT_SECONDS / static_cast<float>(to - from);
  }
  const float support = country.support_share * std::max(0.0f, market - country.support_from) * (1 + country.vat);
  return (total_price(market * country.market_factor, slot, tariff, country.standard_offset) + country.flat) *
             country.factor -
         support;
}

// What `grid_kwh` costs from `start`, straight through at CHARGING_KW, as the car charges at once, on its own schedule
// or in one of the board's windows.
double straight_cost(const PriceTable &prices, const Tariff &tariff, const Country &country, int64_t start,
                     float grid_kwh) {
  double cost = 0;
  for (int64_t slot = start; grid_kwh > 1e-4f; slot += SLOT_SECONDS) {
    const float kwh = std::min(grid_kwh, CHARGING_KW * SLOT_SECONDS / 3600.0f);
    cost += kwh * billed_price(prices, tariff, country, slot);
    grid_kwh -= kwh;
  }
  return cost;
}

// When an owner would start the car's own schedule on a plan: when its cheapest fee of the night starts on a workday,
// in the month that's latest, so the schedule is cheap all year, or midnight where the fee is the same all night.
int night_start(const Tariff &tariff, int32_t standard_offset) {
  int latest = -1;
  for (unsigned month = 1; month <= 12; ++month) {
    int64_t day = days_from_civil(2026, month, 15);
    while (weekday(day) != 3)  // a Wednesday
      ++day;
    std::map<int, float> fees;
    for (int at = PLUG_IN; at < READY_BY; at += SLOT_SECONDS / 60)
      fees[at] = tariff_fee(local_to_utc(day, at, standard_offset), tariff, standard_offset);
    const auto [low, high] =
        std::minmax_element(fees.begin(), fees.end(), [](const auto &a, const auto &b) { return a.second < b.second; });
    if (high->second - low->second > 1e-6f)
      latest = std::max(latest, low->first);  // the first of the cheapest
  }
  return latest < 0 ? 24 * 60 : latest;
}

// A plan's year, in its currency and in euros at the ECB's daily rate.
struct Result {
  Country country;
  std::string name;  // the plan's
  double at_once = 0, schedule = 0, best = 0, board = 0, energy = 0;
  double at_once_euros = 0, schedule_euros = 0, best_euros = 0, board_euros = 0;
  int schedule_from = 0, best_from = 0;
};

Result run(const Country &country, const PriceTable &prices, const Rates &rates, int64_t first_night,
           int64_t last_night) {
  std::stringstream plan;
  if (country.plan_text != nullptr)
    plan << country.plan_text;
  else
    plan << std::ifstream("plans/" + country.plan + ".yaml").rdbuf();
  TariffText text;
  Tariff tariff;
  std::string error = read_tariff(plan.str(), text);
  if (error.empty())
    error = make_tariff(plan.str(), country.currency, tariff);
  if (!error.empty())
    fail(country.name + std::string(", ") + country.plan + ": " + error);
  tariff.vat = country.vat;
  tariff.margin = country.margin;
  Settings settings;
  settings.battery_kwh = BATTERY_KWH;
  settings.charging_kw = CHARGING_KW;
  settings.standard_offset = country.standard_offset;

  const int minutes = static_cast<int>(std::ceil((LIMIT - SOC) / 100.0f * BATTERY_KWH / EFFICIENCY / CHARGING_KW * 60));
  // By the start time, in minutes after midnight before the night, in the plan's currency and in euros.
  std::map<int, double> straight, straight_euros;
  Result result{country, text.name};
  result.schedule_from = night_start(tariff, country.standard_offset);
  for (int64_t day = first_night; day <= last_night; ++day) {
    ScheduleRequest request;
    request.now = local_to_utc(day, PLUG_IN, country.standard_offset);
    request.deadline = local_to_utc(day, READY_BY, country.standard_offset);
    request.soc = SOC;
    request.limit = LIMIT;
    request.tariff = tariff;
    request.settings = settings;
    const Schedule schedule = make_schedule(prices, request);
    if (!schedule.valid || schedule.unpriced_slots > 0 || schedule.soc_at_end < LIMIT - FULL_WITHIN)
      fail(std::string(country.name) + ": no full schedule on " + date(day));
    const double rate = rate_on(rates, day);
    float kwh = 0;
    double cost = 0;
    for (const Window &w : schedule.windows) {
      cost += straight_cost(prices, tariff, country, w.start, w.energy_kwh);
      kwh += w.energy_kwh;
    }
    result.board += cost;
    result.board_euros += cost / rate;
    result.energy += kwh;
    // The same energy straight through: the board may leave the last 0.1% to a window it doesn't start (FULL_WITHIN).
    for (int start = PLUG_IN; start + minutes <= READY_BY; start += SLOT_SECONDS / 60) {
      const double night =
          straight_cost(prices, tariff, country, local_to_utc(day, start, country.standard_offset), kwh);
      straight[start] += night;
      straight_euros[start] += night / rate;
    }
  }
  const auto best = std::min_element(straight.begin(), straight.end(),
                                     [](const auto &a, const auto &b) { return a.second < b.second; });
  result.best_from = best->first;
  result.at_once = straight.at(PLUG_IN);
  result.at_once_euros = straight_euros.at(PLUG_IN);
  result.best = best->second;
  result.best_euros = straight_euros.at(best->first);
  result.schedule = straight.at(result.schedule_from);
  result.schedule_euros = straight_euros.at(result.schedule_from);

  const auto row = [&country, &result](const std::string &label, double cost, double in_euros) {
    std::printf("  %-42s %8.2f %s  %.3f %s/kWh", label.c_str(), cost, country.currency, cost / result.energy,
                country.currency);
    if (std::string(country.currency) != "EUR")
      std::printf("  %8.2f EUR", in_euros);
    std::printf("\n");
  };
  std::printf("%s, %s%s: %d nights from %s to %s, %.0f kWh from the grid\n", country.name, result.name.c_str(),
              country.area_name.empty() ? "" : (" (" + country.area_name + ")").c_str(),
              static_cast<int>(last_night - first_night + 1), date(first_night).c_str(), date(last_night).c_str(),
              result.energy);
  row("Charging at once, from " + hhmm(PLUG_IN), result.at_once, result.at_once_euros);
  row("The car's own schedule, from " + hhmm(result.schedule_from), result.schedule, result.schedule_euros);
  row("The best start, found afterwards, " + hhmm(result.best_from), result.best, result.best_euros);
  row("This board", result.board, result.board_euros);
  std::printf("\n");
  return result;
}

// How much less the board's charging cost than another way, in whole percents.
long percent_less(double board, double other) { return std::lround(100 * (1 - board / other)); }

// A whole amount with commas between the thousands, as the README writes them.
std::string whole(double amount) {
  std::string digits = std::to_string(std::lround(amount));
  for (auto at = static_cast<int>(digits.size()) - 3; at > (digits[0] == '-' ? 1 : 0); at -= 3)
    digits.insert(static_cast<size_t>(at), ",");
  return digits;
}

}  // namespace

int main() try {
  const int64_t first_night = days_from_civil(2025, 10, 1), last_night = days_from_civil(2026, 9, 30);
  // Each country's figures, with VAT, and their sources are in plans/<country>/AGENTS.md.
  std::vector<Country> countries;
  {
    Country c{"Austria",
              "at",
              Source::SMARD,
              "4170",
              "EUR",
              0.20f,
              0.018f,
              0.00807f,
              CET_STANDARD_OFFSET,
              "Low: about 1% of homes paid the market price in 2025"};
    c.factor = 1.07f;  // Vienna's Gebrauchsabgabe
    c.hourly = true;   // as most dynamic contracts bill
    countries.push_back(c);
  }
  {
    Country c{"Belgium",
              "be",
              Source::SMARD,
              "4996",
              "EUR",
              0.06f,
              0.01676f,
              0.0953f,
              CET_STANDARD_OFFSET,
              "Low: 0.4% of homes pay the market price, and Flanders' fee per kW isn't counted"};
    c.hourly = true;  // as Luminus, TotalEnergies, Eneco and Mega bill
    countries.push_back(c);
  }
  {
    // Households pay the EWRC's regulated price, with ERM Zapad's network fees and ESO's in the rates.
    Country c{"Bulgaria",
              "bg",
              Source::NONE,
              "",
              "EUR",
              0.0f,
              0.0f,
              0.0f,
              CET_STANDARD_OFFSET + 3600,
              "High: nearly every home pays a regulated price, cheaper at night"};
    c.plan_text =
        "name: Regulated price, in ERM Zapad's area\ncurrency: EUR\ncalendar:\n  apr-oct:\n    mon-sun: night "
        "07:00 day 23:00 night\n  nov-mar:\n    mon-sun: night 06:00 day 22:00 night\nrates:\n  night: "
        "0.09192\n  day: 0.15425\n";
    countries.push_back(c);
  }
  {
    // Households pay the government's regulated price, with HEP ODS's network fees (Bijeli) and the renewables fee in
    // the rates, the night from 21:00 winter time, so 22:00 on the clock in summer.
    Country c{"Croatia",
              "hr",
              Source::NONE,
              "",
              "EUR",
              0.0f,
              0.0f,
              0.0f,
              CET_STANDARD_OFFSET,
              "High: no home pays the market price, and the regulated one is cheaper at night"};
    c.plan_text =
        "name: Regulated price, HEP ODS Bijeli\ncurrency: EUR\nclock: winter\ncalendar:\n  jan-dec:\n    "
        "mon-sun: nt 07:00 vt 21:00 nt\nrates:\n  nt: 0.10127\n  vt: 0.19903\n";
    countries.push_back(c);
  }
  {
    Country c{"Czechia",
              "cz",
              Source::SMARD,
              "261",
              "CZK",
              0.21f,
              0.4477f,
              0.23297f,
              CET_STANDARD_OFFSET,
              "Low: few homes pay the market price"};
    c.hourly = true;  // as Delta Green's and Yello's price lists say
    countries.push_back(c);
  }
  {
    Country c{"Denmark",
              "dk",
              Source::SMARD,
              "253",
              "DKK",
              0.25f,
              0.10f,
              0.15375f,
              CET_STANDARD_OFFSET,
              "High: most homes pay the market price"};
    c.hourly = true;  // as Radius reads meters by the hour unless asked
    countries.push_back(c);
  }
  {
    Country c{"Estonia",
              "ee",
              Source::ELERING,
              "ee",
              "EUR",
              0.24f,
              0.005f,
              0.02704f,
              VILNIUS_STANDARD_OFFSET,
              "High: exchange-price packages are common"};
    c.hourly = true;  // most meters still record hours
    countries.push_back(c);
  }
  {
    Country c{"Finland",
              "fi",
              Source::ELERING,
              "fi",
              "EUR",
              0.255f,
              0.0055f,
              0.02918f,
              VILNIUS_STANDARD_OFFSET,
              "High: a third of homes pay the market price"};
    c.hourly = true;  // most meters still record hours
    countries.push_back(c);
  }
  {
    // TURPE's network fee for a home with off-peak hours (CU4), 22:00 - 06:00, which Enedis sets for each address.
    Country c{"France",
              "fr",
              Source::SMARD,
              "254",
              "EUR",
              0.20f,
              0.0234f,
              0.03702f,
              CET_STANDARD_OFFSET,
              "Low: two small suppliers bill the market price, and off-peak hours differ by address"};
    c.hourly = true;  // as Frank Énergie bills
    c.plan_text =
        "name: TURPE CU4, off-peak 22:00 - 06:00\ncurrency: EUR\ncalendar:\n  nov-mar:\n    mon-sun: hch 06:00 hph "
        "22:00 hch\n  apr-oct:\n    mon-sun: hcb 06:00 hpb 22:00 hcb\nrates:\n  hph: 0.08988\n  hch: 0.04764\n  hpb: "
        "0.01992\n  hcb: 0.01392\n";
    countries.push_back(c);
  }
  {
    Country c{"Germany",
              "de",
              Source::SMARD,
              "4169",
              "EUR",
              0.19f,
              0.020f,
              0.07837f,
              CET_STANDARD_OFFSET,
              "Low: dynamic contracts and Modul 3 need a smart meter, and 6% of meters are one"};
    countries.push_back(c);
  }
  {
    // Households pay universal service's regulated price, and an electric car's owner can have A2 on the car's own
    // meter, in ELMŰ's area: cheaper from 22:00 winter time on workdays and all weekend, within its own 2,523 kWh a
    // year. The car's 3,333 kWh pass that, so each rate blends its price with the 70.104 Ft above the band, by their
    // shares.
    Country c{"Hungary",
              "hu",
              Source::NONE,
              "",
              "HUF",
              0.0f,
              0.0f,
              0.0f,
              CET_STANDARD_OFFSET,
              "Medium: homes pay regulated prices, and the car's own meter (A2) pays less at night"};
    c.plan_text =
        "name: A2 on the car's own meter, ELMŰ\ncurrency: HUF\nclock: winter\ncalendar:\n  jan-dec:\n    "
        "mon-fri: off 06:00 peak 22:00 off\n    sat-sun: off\nrates:\n  off: 41.388\n  peak: 49.175\n";
    countries.push_back(c);
  }
  {
    // EV owners take a fixed plan with a cheap night instead of a dynamic contract, which costs more: Electric
    // Ireland's Night Boost from 1 July 2026.
    Country c{"Ireland",
              "ie",
              Source::NONE,
              "",
              "EUR",
              0.0f,
              0.0f,
              0.0f,
              0,
              "High: electric car owners usually take a fixed plan with a cheap night, cheaper than dynamic contracts"};
    c.plan_text =
        "name: Electric Ireland Night Boost\ncurrency: EUR\ncalendar:\n  jan-dec:\n    mon-sun: night 02:00 "
        "boost 04:00 night 08:00 day 23:00 night\nrates:\n  boost: 0.10880\n  night: 0.18540\n  day: 0.37600\n";
    countries.push_back(c);
  }
  {
    // Hourly offers bill the national PUN Index GME, which only GME serves, with a session token: IT-North's price,
    // which the board uses, stands in for it.
    Country c{"Italy",
              "it",
              Source::SMARD,
              "255",
              "EUR",
              0.10f,
              0.0242f,
              0.08286f,
              CET_STANDARD_OFFSET,
              "Medium: about 3% of homes pay the hourly price"};
    c.market_factor = 1.10f;  // the PUN billed on 10% grid losses too
    c.hourly = true;
    countries.push_back(c);
  }
  {
    Country c{"Latvia",
              "lv",
              Source::ELERING,
              "lv",
              "EUR",
              0.21f,
              0.014f,
              0.0f,
              VILNIUS_STANDARD_OFFSET,
              "High: one home in six pays the market price"};
    countries.push_back(c);
  }
  {
    Country c{"Lithuania",
              "lt",
              Source::ELERING,
              "lt",
              "EUR",
              0.21f,
              0.025f,
              -0.00026f,
              VILNIUS_STANDARD_OFFSET,
              "Medium: a few percent of homes pay each quarter-hour's price, most a fixed one"};
    countries.push_back(c);
  }
  {
    Country c{"Luxembourg",
              "lu",
              Source::SMARD,
              "4169",
              "EUR",
              0.08f,
              0.0216f,
              0.0f,
              CET_STANDARD_OFFSET,
              "Low: few homes pay the market price"};
    c.hourly = true;  // as Encevo's suppliers billed until 31 May 2026
    countries.push_back(c);
  }
  {
    Country c{"Netherlands",
              "nl",
              Source::SMARD,
              "256",
              "EUR",
              0.21f,
              0.019f,
              0.11085f,
              CET_STANDARD_OFFSET,
              "High: 8% of homes pay the market price"};
    c.hourly = true;  // as about half the dynamic contracts bill
    c.plan_text =
        "name: No grid fee per kWh\ncurrency: EUR\ncalendar:\n  jan-dec:\n    mon-sun: flat\nrates:\n  flat: 0\n";
    countries.push_back(c);
  }
  {
    // Spot contracts with the state's support, which most homes' power in NO1 has left for Norgespris' fixed price.
    Country c{"Norway",
              "no",
              Source::ENERGY_CHARTS,
              "NO1",
              "NOK",
              0.25f,
              0.059f,
              0.10163f,
              CET_STANDARD_OFFSET,
              "Medium: most homes' power is at Norgespris, a fixed price, where the car's own schedule does as well"};
    c.hourly = true;  // meters record hours
    c.support_share = 0.9f;
    c.support_from = 0.77f;
    countries.push_back(c);
  }
  {
    Country c{"Poland",
              "pl",
              Source::SMARD,
              "257",
              "PLN",
              0.23f,
              0.10357f,
              0.059655f,
              CET_STANDARD_OFFSET,
              "Low: 0.03% of homes have a dynamic contract, and it bills another auction's price"};
    c.hourly = true;  // as dynamic contracts bill TGE's Fixing I, a price the board doesn't have, by the hour
    countries.push_back(c);
  }
  {
    Country c{"Portugal", "pt",     Source::OMIE, "PT", "EUR",
              0.23f,      0.03444f, 0.00377f,     0,    "Low: under 1% of homes pay the market price"};
    c.market_factor = 1.15f;  // the grid losses indexed offers bill
    countries.push_back(c);
  }
  {
    Country c{"Romania",
              "ro",
              Source::ENERGY_CHARTS,
              "RO",
              "RON",
              0.21f,
              0.242f,
              0.17847f,
              CET_STANDARD_OFFSET + 3600,
              "Low: no home had a dynamic contract in 2025"};
    countries.push_back(c);
  }
  {
    // Households pay ÚRSO's regulated price, ZSE's DD3 without the state's aid, with ZSD's fees and levies in the
    // rates, its low tariff at the hours of most of ZSD's programs.
    Country c{"Slovakia",
              "sk",
              Source::NONE,
              "",
              "EUR",
              0.0f,
              0.0f,
              0.0f,
              CET_STANDARD_OFFSET,
              "High: nearly every home pays a regulated price, cheaper at night"};
    c.plan_text =
        "name: Regulated price, DD3 in ZSD's area\ncurrency: EUR\ncalendar:\n  jan-dec:\n    mon-sun: nt "
        "05:30 vt 13:30 nt 15:30 vt 23:30 nt\nrates:\n  nt: 0.15264\n  vt: 0.21046\n";
    countries.push_back(c);
  }
  {
    Country c{"Slovenia",
              "si",
              Source::SMARD,
              "260",
              "EUR",
              0.22f,
              0.01586f,
              0.003f,
              CET_STANDARD_OFFSET,
              "Low: 0.3% of homes pay the market price"};
    c.hourly = true;  // as Bisol, most dynamic homes' supplier, bills
    countries.push_back(c);
  }
  {
    Country c{"Spain",
              "es",
              Source::OMIE,
              "ES",
              "EUR",
              0.21f,
              0.03267f,
              0.0f,
              CET_STANDARD_OFFSET,
              "High: over a quarter of homes pay each hour's price, most on the regulated PVPC"};
    c.factor = 1.0511269632f;  // the electricity tax
    c.market_factor = 1.167f;  // grid losses at night and the municipal rate
    c.hourly = true;
    countries.push_back(c);
  }
  {
    Country c{"Sweden",
              "se",
              Source::ELPRISETJUSTNU,
              "SE3",
              "SEK",
              0.25f,
              0.12f,
              0.45f,
              CET_STANDARD_OFFSET,
              "High: one contract in seven pays each quarter-hour's or hour's price, most the month's average"};
    countries.push_back(c);
  }
  {
    // Households pay EKZ's regulated price, the same every hour, with its network fee and the federal levies.
    Country c{"Switzerland",
              "ch",
              Source::NONE,
              "",
              "CHF",
              0.0f,
              0.0f,
              0.0f,
              CET_STANDARD_OFFSET,
              "High: homes pay their grid operator's price, which at EKZ, the largest, is the same at every hour"};
    c.plan_text =
        "name: EKZ's price, Netz 400ST\ncurrency: CHF\ncalendar:\n  apr-sep:\n    mon-sun: summer\n  "
        "oct-mar:\n    mon-sun: winter\nrates:\n  summer: 0.21771\n  winter: 0.26419\n";
    countries.push_back(c);
  }

  // Every plan of each country, the market prices loaded once for each area.
  std::map<std::string, PriceTable> markets;
  std::map<std::string, Rates> rates;
  std::vector<Result> results;
  for (const Country &country : countries) {
    std::vector<std::string> plans;
    if (country.plan_text == nullptr) {
      for (const auto &entry : std::filesystem::directory_iterator(std::string("plans/") + country.code))
        if (entry.path().extension() == ".yaml")
          plans.push_back(std::string(country.code) + "/" + entry.path().stem().string());
      std::sort(plans.begin(), plans.end());
    } else {
      plans.emplace_back();  // its own
    }
    for (const std::string &plan : plans) {
      Country c = country;
      if (!plan.empty())
        take_plan(c, plan);
      const std::string key = std::to_string(static_cast<int>(c.source)) + c.area;
      if (markets.count(key) == 0)
        markets[key] = market_prices(c, first_night, last_night + 1);
      const bool euros = std::string(c.currency) == "EUR";
      if (!euros && rates.count(c.currency) == 0)
        rates[c.currency] = ecb_rates(c.currency, first_night, last_night + 1);
      const Rates &rate = rates[euros ? std::string() : c.currency];
      results.push_back(run(c, euros ? markets[key] : in_currency(markets[key], rate, first_night, last_night + 1),
                            rate, first_night, last_night));
    }
  }

  // The README's list of countries, each with a table of what charging cost a year on each plan, in the order of their
  // files, against the car's own schedule.
  for (const Country &country : countries) {
    // Non-breaking spaces keep each figure and its heading on one line, so only a plan's name wraps.
    const std::string currency = std::string("&nbsp;") + country.currency;
    std::vector<const Result *> mine;
    for (const Result &r : results)
      if (std::string(r.country.name) == country.name)
        mine.push_back(&r);
    const std::string level = std::string(country.confidence).substr(0, std::string(country.confidence).find(':'));
    const std::string reason = std::string(country.confidence).substr(level.size() + 1);
    // Worth it where a plan saves 15% or more and many homes pay the market price, nothing where none saves anything.
    long most = 0;
    for (const Result *r : mine)
      most = std::max(most, percent_less(r->board, r->schedule));
    const char *icon = most <= 0 ? "⚪" : most >= 15 && level != "Low" ? "🟢" : "🟡";
    // The empty paragraph leaves as much room above the table as below it.
    std::printf("<details>\n<summary>%s <b>%s</b></summary>\n<p></p>\n\n", icon, country.name);
    std::printf(
        "| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule "
        "| This&nbsp;board | Saved&nbsp;a&nbsp;year |\n"
        "| --- | ---: | ---: | ---: | ---: |\n");
    for (const Result *r : mine) {
      const long percent = percent_less(r->board, r->schedule);
      const std::string saved =
          percent > 0 ? whole(r->schedule - r->board) + currency + ",&nbsp;" + std::to_string(percent) + "%"
          : percent < 0
              ? whole(r->board - r->schedule) + currency + "&nbsp;more,&nbsp;" + std::to_string(-percent) + "%"
              : std::string("nothing");
      std::printf("| %s%s | %s | %s%s | %s%s | %s |\n", r->name.c_str(),
                  r->country.area_name.empty() ? "" : (" (" + r->country.area_name + ")").c_str(),
                  hhmm(r->schedule_from).c_str(), whole(r->schedule).c_str(), currency.c_str(), whole(r->board).c_str(),
                  currency.c_str(), saved.c_str());
    }
    std::printf("\n%s confidence:%s. [The research](plans/%s/AGENTS.md)\n\n</details>\n\n", level.c_str(),
                reason.c_str(), country.code);
  }
  return 0;
} catch (const std::exception &e) {  // a file it couldn't write or read
  std::fprintf(stderr, "%s\n", e.what());
  return 1;
}
