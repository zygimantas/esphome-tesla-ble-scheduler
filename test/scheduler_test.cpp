// Unit tests for charger.h and the headers it includes; CONTRIBUTING.md says how to build and run them.
#include "scheduler/charger.h"
#include "scheduler/settings.h"
#include "scheduler_test_tesla.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace esphome::scheduler;
using scheduler_test::FakeTesla;
using scheduler_test::plugged_in;

static int failures = 0;

static bool check(bool ok, const char *expr, int line) {
  if (!ok) {
    std::printf("FAIL %s:%d: %s\n", __FILE__, line, expr);
    ++failures;
  }
  return ok;
}

static void check_str(const std::string &actual, const std::string &expected, const char *expr, int line) {
  if (actual != expected) {
    std::printf("FAIL %s:%d: %s\n  got:      \"%s\"\n  expected: \"%s\"\n", __FILE__, line, expr, actual.c_str(),
                expected.c_str());
    ++failures;
  }
}

#define CHECK(cond) check((cond), #cond, __LINE__)
#define CHECK_STR(actual, expected) check_str((actual), (expected), #actual, __LINE__)
// Stops the test when the checks after it would index out of range.
#define REQUIRE(cond) \
  do {                \
    if (!CHECK(cond)) \
      return;         \
  } while (0)

// Thursday 2026-09-24T17:00:00Z, 20:00 in Vilnius (EEST).
constexpr int64_t SEP24_1700Z = 1790269200;
constexpr int64_t HOUR = 3600;
// CET midnight of 2026-09-24 and 2026-09-25 (CEST, UTC+2).
constexpr int64_t CET_SEP24 = SEP24_1700Z - 19 * HOUR;
constexpr int64_t CET_SEP25 = CET_SEP24 + DAY_SECONDS;
constexpr int64_t TROUGH = SEP24_1700Z + 5 * HOUR + 30 * 60;  // Friday 01:30 local, where the cheapest window starts

static std::string iso(int64_t t) {
  const int64_t day = floor_div(t, DAY_SECONDS), s = t - day * DAY_SECONDS;
  const CivilDate d = civil_from_days(day);
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%04d-%02u-%02uT%02d:%02d:%02dZ", static_cast<int>(d.year), d.month, d.day,
                static_cast<int>(s / HOUR), static_cast<int>(s / 60 % 60), static_cast<int>(s % 60));
  return buf;
}

static int64_t utc(const char *iso8601) {
  const auto t = parse_iso8601(iso8601);
  CHECK(t.has_value());
  return t.value_or(0);
}

// Hours since local midnight in Vilnius.
static double local_hours(int64_t t) {
  const int64_t local = t + eu_offset(t, VILNIUS_STANDARD_OFFSET);
  return static_cast<double>(local - floor_div(local, DAY_SECONDS) * DAY_SECONDS) / 3600.0;
}

static int local_hour(int64_t t) { return static_cast<int>(local_hours(t)); }

// EUR/MWh with a night trough bottoming out at 03:07:30 local, a solar dip and an evening peak.
static double typical_baltic_price(int64_t t) {
  const double hour = local_hours(t);
  if (hour >= 1 && hour < 5)
    return 15 + 8 * std::fabs(hour - 3.125);
  if (hour >= 11 && hour < 15)
    return 45;
  if (hour >= 17 && hour < 22)
    return 190;
  return 95;
}

// Adds a CET delivery day of typical prices, from a DayAheadPrices response like the Nord Pool data portal's.
static void add_day(PriceTable &prices, int64_t cet_midnight_utc) {
  std::string json = R"({"deliveryDateCET":"x","version":3,"multiAreaEntries":[)";
  for (int i = 0; i < 96; ++i) {
    const int64_t start = cet_midnight_utc + i * SLOT_SECONDS;
    char entry[256];
    std::snprintf(entry, sizeof(entry),
                  R"(%s{"deliveryStart":"%s","deliveryEnd":"%s","entryPerArea":{"LT":%.2f,"LV":1.0}})", i ? "," : "",
                  iso(start).c_str(), iso(start + SLOT_SECONDS).c_str(), typical_baltic_price(start));
    json += entry;
  }
  json += R"(],"areaStates":[{"state":"Final","areas":["LT"]}]})";
  CHECK(prices.add_nord_pool(json.data(), json.size(), "LT") == 96);
}

static PriceTable two_days() {
  PriceTable prices;
  add_day(prices, CET_SEP24);
  add_day(prices, CET_SEP25);
  return prices;
}

static Controller with_prices(PriceTable prices = two_days()) {
  Controller controller;
  controller.prices = std::move(prices);
  return controller;
}

// One price per slot in [from, to), by the time since from.
template <typename PriceAt>
static PriceTable prices_from(int64_t from, int64_t to, PriceAt price_at) {
  PriceTable prices;
  for (int64_t t = from; t < to; t += SLOT_SECONDS)
    prices.set(t, price_at(t - from));
  return prices;
}

static bool near(float a, float b, float tolerance = 1e-6f) { return std::fabs(a - b) < tolerance; }

// ---------------------------------------------------------------------------
// Calendar and Nord Pool prices
// ---------------------------------------------------------------------------

static void test_calendar() {
  const int64_t spring = utc("2026-03-29T01:00:00Z"), autumn = utc("2026-10-25T01:00:00Z");  // clocks go forward, back
  CHECK(!eu_summer_time(spring - 1));
  CHECK(eu_summer_time(spring));
  CHECK(eu_summer_time(autumn - 1));
  CHECK(!eu_summer_time(autumn));
  // 2027: October ends on a Sunday, the day clocks go back.
  CHECK(eu_summer_time(utc("2027-10-31T00:59:59Z")) && !eu_summer_time(utc("2027-10-31T01:00:00Z")));

  CHECK(floor_div(-1, 60) == -1 && floor_div(-60, 60) == -1 && floor_div(0, 60) == 0 && floor_div(59, 60) == 0);
  CHECK(weekday(0) == 4 && weekday(-20) == 5);  // Thursday 1970-01-01 and Friday 1969-12-12
  // Every day from 769 BC to AD 2517 follows the one before, and maps back to its day number.
  CHECK(days_from_civil(1970, 1, 1) == 0);
  int wrong = 0;
  CivilDate before = civil_from_days(-1000001);
  for (int64_t day = -1000000; day < 200000; ++day) {
    const CivilDate date = civil_from_days(day);
    const bool next_day = date.year == before.year && date.month == before.month && date.day == before.day + 1;
    const bool next_month = date.year == before.year && date.month == before.month + 1 && date.day == 1;
    const bool next_year = date.year == before.year + 1 && date.month == 1 && date.day == 1 && before.month == 12;
    wrong += !(next_day || next_month || next_year) || days_from_civil(date.year, date.month, date.day) != day;
    before = date;
  }
  CHECK(wrong == 0);

  CHECK_STR(format_hhmm(SEP24_1700Z, VILNIUS_STANDARD_OFFSET), "20:00");
  CHECK_STR(format_hhmm(SEP24_1700Z + 3 * HOUR + 45 * 60, VILNIUS_STANDARD_OFFSET), "23:45");
  std::string week;
  for (int day = 0; day < 7; ++day)
    week += format_day_hhmm(SEP24_1700Z + day * DAY_SECONDS, VILNIUS_STANDARD_OFFSET).substr(0, 4);
  CHECK_STR(week, "Thu Fri Sat Sun Mon Tue Wed ");

  CHECK(next_local_time(SEP24_1700Z, 7 * 60, VILNIUS_STANDARD_OFFSET) == SEP24_1700Z + 11 * HOUR);
  CHECK(next_local_time(SEP24_1700Z - 15 * HOUR, 7 * 60, VILNIUS_STANDARD_OFFSET) == SEP24_1700Z - 13 * HOUR);
  // At 07:00 the next 07:00 is the next day's; at 00:30 the next midnight too.
  CHECK(next_local_time(SEP24_1700Z + 11 * HOUR, 7 * 60, VILNIUS_STANDARD_OFFSET) == SEP24_1700Z + 35 * HOUR);
  CHECK(next_local_time(SEP24_1700Z + 4 * HOUR + 30 * 60, 0, VILNIUS_STANDARD_OFFSET) == SEP24_1700Z + 28 * HOUR);
  // Winter: 07:30 in Vilnius, so the next 07:00 is the next day.
  CHECK(next_local_time(utc("2026-12-01T05:30:00Z"), 7 * 60, VILNIUS_STANDARD_OFFSET) == utc("2026-12-02T05:00:00Z"));

  // Mon 00:00 in Vilnius, summer time.
  CHECK(local_to_utc(days_from_civil(2026, 9, 28), 0, VILNIUS_STANDARD_OFFSET) == utc("2026-09-27T21:00:00Z"));
  // The day before clocks go back.
  CHECK(local_to_utc(days_from_civil(2026, 10, 24), 7 * 60, VILNIUS_STANDARD_OFFSET) == utc("2026-10-24T04:00:00Z"));
  // The day they go back: midnight is still summer time, 07:00 winter time.
  CHECK(local_to_utc(days_from_civil(2026, 10, 25), 0, VILNIUS_STANDARD_OFFSET) == utc("2026-10-24T21:00:00Z"));
  CHECK(local_to_utc(days_from_civil(2026, 10, 25), 7 * 60, VILNIUS_STANDARD_OFFSET) == utc("2026-10-25T05:00:00Z"));
  // Its repeated 03:00 is the first one, still summer time.
  CHECK(local_to_utc(days_from_civil(2026, 10, 25), 3 * 60, VILNIUS_STANDARD_OFFSET) == utc("2026-10-25T00:00:00Z"));
  // The day they go forward: 03:00 doesn't exist, so it's 04:00 summer time.
  CHECK(local_to_utc(days_from_civil(2027, 3, 28), 3 * 60, VILNIUS_STANDARD_OFFSET) == utc("2027-03-28T01:00:00Z"));
  // The hour after each change: autumn 03:30 is the first one and 04:00 is winter time already; spring
  // 03:30 is skipped, so it's 04:30 summer time, and 04:00 is summer time.
  CHECK(local_to_utc(days_from_civil(2026, 10, 25), 3 * 60 + 30, VILNIUS_STANDARD_OFFSET) ==
        utc("2026-10-25T00:30:00Z"));
  CHECK(local_to_utc(days_from_civil(2026, 10, 25), 4 * 60, VILNIUS_STANDARD_OFFSET) == utc("2026-10-25T02:00:00Z"));
  CHECK(local_to_utc(days_from_civil(2027, 3, 28), 3 * 60 + 30, VILNIUS_STANDARD_OFFSET) ==
        utc("2027-03-28T01:30:00Z"));
  CHECK(local_to_utc(days_from_civil(2027, 3, 28), 4 * 60, VILNIUS_STANDARD_OFFSET) == utc("2027-03-28T01:00:00Z"));

  // Under a day ahead the time alone is unambiguous.
  CHECK_STR(format_when(SEP24_1700Z + 23 * HOUR, SEP24_1700Z, VILNIUS_STANDARD_OFFSET), "19:00");
  CHECK_STR(format_when(SEP24_1700Z + DAY_SECONDS, SEP24_1700Z, VILNIUS_STANDARD_OFFSET), "Fri 20:00");
  CHECK_STR(format_when(SEP24_1700Z + 25 * HOUR, SEP24_1700Z, VILNIUS_STANDARD_OFFSET), "Fri 21:00");
  CHECK_STR(format_when(SEP24_1700Z + 28 * HOUR, SEP24_1700Z, VILNIUS_STANDARD_OFFSET), "Sat 00:00");
  // On the day the clocks go forward, 23.5 real hours are 24.5 clock hours.
  CHECK_STR(format_when(utc("2027-03-28T13:30:00Z"), utc("2027-03-27T14:00:00Z"), VILNIUS_STANDARD_OFFSET),
            "Sun 16:30");
}

static void test_nord_pool_prices() {
  CHECK(parse_iso8601("2025-10-01T22:00:00Z") == 1759356000);
  CHECK(parse_iso8601("2025-10-01T22:00:00") == 1759356000);  // no offset: UTC
  CHECK(parse_iso8601("2025-10-01T22:00:30Z") == 1759356030);
  CHECK(parse_iso8601("2026-01-01T00:00:00Z") == 1767225600);
  for (const char *bad :
       {"not a date", "2025-10-01T22:00:00+02", "2025-10-01T22:00:00x", "2025-10-01T22:00:00.5x",
        "2025-00-01T22:00:00Z", "2025-13-01T22:00:00Z", "2025-10-00T22:00:00Z", "2025-10-32T22:00:00Z"})
    CHECK(!parse_iso8601(bad));
  CHECK(!parse_iso8601(nullptr));

  // 21:30Z is still Sep 24 in CET (23:30); 22:30Z is already Sep 25 (00:30).
  CHECK_STR(nord_pool_url("LT", "EUR", SEP24_1700Z + 4 * HOUR + 30 * 60, 0),
            "https://dataportal-api.nordpoolgroup.com/api/DayAheadPrices"
            "?market=DayAhead&date=2026-09-24&deliveryArea=LT&currency=EUR");
  CHECK(nord_pool_url("LT", "EUR", SEP24_1700Z + 5 * HOUR + 30 * 60, 0).find("date=2026-09-25") != std::string::npos);
  CHECK(nord_pool_url("LT", "EUR", SEP24_1700Z + 5 * HOUR + 30 * 60, 1).find("date=2026-09-26") != std::string::npos);
  CHECK(nord_pool_url("SE3", "SEK", SEP24_1700Z, 0).find("deliveryArea=SE3&currency=SEK") != std::string::npos);
  CHECK(end_of_delivery_day(SEP24_1700Z) == CET_SEP25);
  CHECK(end_of_next_delivery_day(SEP24_1700Z) == CET_SEP25 + DAY_SECONDS);

  PriceTable prices;
  const char *json =
      R"({"multiAreaEntries":[)"
      R"({"deliveryStart":"2025-09-30T22:00:00Z","deliveryEnd":"2025-09-30T22:15:00Z","entryPerArea":{"LT":-12.5}},)"
      R"({"deliveryStart":"2025-09-30T22:15:00Z","deliveryEnd":"2025-09-30T22:30:00Z","entryPerArea":{"EE":1.0}},)"
      R"({"deliveryStart":"later","deliveryEnd":"2025-09-30T22:45:00Z","entryPerArea":{"LT":1.0}},)"
      R"({"deliveryEnd":"2025-09-30T23:00:00Z","entryPerArea":{"LT":1.0}}]})";
  CHECK(prices.add_nord_pool(json, std::strlen(json), "LT") == 1);
  const int64_t t = utc("2025-09-30T22:00:00Z");
  const auto negative = prices.get(t);
  CHECK(negative && near(*negative, -0.0125f));
  CHECK(!prices.get(t + SLOT_SECONDS));
  CHECK(prices.add_nord_pool("{oops", 5, "LT") == -1);
  CHECK(prices.add_nord_pool("{}", 2, "LT") == 0);

  for (int64_t slot = t - DAY_SECONDS; slot < t - DAY_SECONDS + HOUR; slot += SLOT_SECONDS)
    prices.set(slot, 0.08f);
  CHECK(prices.known_until(t + 5 * 60) == t + SLOT_SECONDS);             // up to the gap after 22:00
  CHECK(prices.known_until(t + SLOT_SECONDS) == t + SLOT_SECONDS);       // no price: its own start
  CHECK(prices.known_until(t - DAY_SECONDS) == t - DAY_SECONDS + HOUR);  // an hour of four slots
  prices.set(t - DAY_SECONDS + SLOT_SECONDS, 0.25f);                     // a later download replaces a price
  prices.set(t - SLOT_SECONDS, 0.5f);                                    // and fills a gap
  CHECK(prices.get(t - DAY_SECONDS + SLOT_SECONDS) == 0.25f &&
        prices.known_until(t - DAY_SECONDS) == t - DAY_SECONDS + HOUR);
  CHECK(prices.known_until(t - SLOT_SECONDS) == t + SLOT_SECONDS);
  prices.drop_before(t);
  CHECK(!prices.get(t - DAY_SECONDS) && !prices.get(t - SLOT_SECONDS) && prices.get(t));
}

static void test_smard_prices() {
  // SMARD keeps a file a week, from Monday 00:00 German time. These are its real files' times.
  CHECK_STR(smard_url(262, SEP24_1700Z, 0),  // Thursday 24 September
            "https://www.smard.de/app/chart_data/262/DE/262_DE_quarterhour_1789941600000.json");
  const auto week = [](const char *now, int day_offset) {
    const std::string url = smard_url(262, utc(now), day_offset);
    return url.substr(url.rfind('_') + 1);
  };
  CHECK_STR(week("2026-09-24T17:00:00Z", 4), "1790546400000.json");  // Monday 28 September
  CHECK_STR(week("2026-10-04T21:30:00Z", 0), "1790546400000.json");  // Sunday 23:30 in Germany,
  CHECK_STR(week("2026-10-04T21:30:00Z", 1), "1791151200000.json");  // its next day,
  CHECK_STR(week("2026-10-04T22:30:00Z", 0), "1791151200000.json");  // and Monday 00:30
  CHECK_STR(week("2026-03-29T12:00:00Z", 0), "1774220400000.json");  // the weeks before and after summer time,
  CHECK_STR(week("2026-03-29T12:00:00Z", 1), "1774821600000.json");
  CHECK_STR(week("2025-10-26T12:00:00Z", 0), "1760911200000.json");  // and before and after winter time
  CHECK_STR(week("2025-10-26T12:00:00Z", 1), "1761519600000.json");

  PriceTable prices;
  const char *json = R"({"meta_data":{"version":1,"created":1791035097048},"series":[)"
                     R"([1790546400000,166.58],[1790547300000,-12.5],[1790548200000,null],["later",1.0]]})";
  CHECK(prices.add_smard(json, std::strlen(json)) == 2);
  const auto first = prices.get(1790546400);
  CHECK(first && near(*first, 0.16658f));
  const auto negative = prices.get(1790547300);
  CHECK(negative && near(*negative, -0.0125f));
  CHECK(!prices.get(1790548200));  // null: not out yet
  CHECK(prices.add_smard("{oops", 5) == -1);
  CHECK(prices.add_smard("{}", 2) == 0);
  PriceTable koruna;  // in the country's own currency, at the ECB's rate
  CHECK(koruna.add_smard(json, std::strlen(json), 25.0f) == 2);
  const auto converted = koruna.get(1790546400);
  CHECK(converted && near(*converted, 4.1645f));
}

static void test_ecb_rates() {
  const std::string xml =
      "<gesmes:Envelope><Cube><Cube time='2026-10-05'><Cube currency='USD' rate='1.1652'/>"
      "<Cube currency='CZK' rate='24.335'/><Cube currency='HUF' rate='0'/>"
      "<Cube currency='CHF' rate='soon'/></Cube></Cube></gesmes:Envelope>";
  const auto czk = ecb_rate(xml.data(), xml.size(), "CZK");
  CHECK(czk && near(*czk, 24.335f));
  CHECK(!ecb_rate(xml.data(), xml.size(), "PLN"));  // not in the file
  CHECK(!ecb_rate(xml.data(), xml.size(), "HUF"));  // not a rate
  CHECK(!ecb_rate(xml.data(), xml.size(), "CHF"));
  CHECK(std::string(ECB_RATES_URL) == "https://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml");
}

static void test_omie_prices() {
  // A file a CET day: Thursday 24 September until 23:59 in Spain, 00:59 in Vilnius.
  CHECK_STR(omie_url(SEP24_1700Z, 0),
            "https://www.omie.es/es/file-download?parents=marginalpdbc&filename=marginalpdbc_20260924.1");
  CHECK(omie_url(SEP24_1700Z + 4 * HOUR + 30 * 60, 0).find("_20260924.1") != std::string::npos);
  CHECK(omie_url(SEP24_1700Z + 5 * HOUR + 30 * 60, 0).find("_20260925.1") != std::string::npos);
  CHECK(omie_url(SEP24_1700Z, 1).find("_20260925.1") != std::string::npos);

  // Quarter-hour 81 of 30 September, 20:00 in Spain, split the two countries' prices; the autumn day has 100.
  const char *file =
      "MARGINALPDBC;\r\n"
      "2026;09;30;81;255;249.79;\r\n"
      "2026;09;30;82;bad;1;\r\n"
      "2025;10;26;100;-1.5;-2;\r\n"
      "*\r\n";
  PriceTable spain, portugal;
  CHECK(spain.add_omie(file, std::strlen(file), "ES") == 2);
  CHECK(portugal.add_omie(file, std::strlen(file), "PT") == 2);
  const int64_t quarter_81 = utc("2026-09-30T18:00:00Z");
  const auto es = spain.get(quarter_81), pt = portugal.get(quarter_81);
  CHECK(es && near(*es, 0.24979f));
  CHECK(pt && near(*pt, 0.255f));
  CHECK(!spain.get(quarter_81 + SLOT_SECONDS));
  const auto last = spain.get(utc("2025-10-26T22:45:00Z"));  // 23:45 CET, after the clocks went back
  CHECK(last && near(*last, -0.002f));
  CHECK(spain.add_omie("<!DOCTYPE html>", 15, "ES") == -1);
}

// ---------------------------------------------------------------------------
// Schedule
// ---------------------------------------------------------------------------

static void test_cheapest_slots() {
  const std::vector<float> prices = {1, 9, 1, 9, 1, 9, 1, 5};
  CHECK(cheapest_slots(prices, 4) == std::vector<int>({0, 2, 4, 6}));  // four separate slots
  CHECK(cheapest_slots(prices, 3) == std::vector<int>({0, 2, 4}));     // of equal prices, the earlier
  CHECK(cheapest_slots(prices, 5) == std::vector<int>({0, 2, 4, 6, 7}));
  CHECK(cheapest_slots(prices, 0).empty());
  CHECK(cheapest_slots(prices, 9).size() == prices.size());
}

// Scheduling from `now` to the next 07:00, the default Ready by.
static ScheduleRequest overnight(int64_t now, float soc = 40, float limit = 80) {
  ScheduleRequest request;
  request.now = now;
  request.deadline = next_local_time(now, 7 * 60, VILNIUS_STANDARD_OFFSET);
  request.soc = soc;
  request.limit = limit;
  return request;
}

// The schedule's only window in local time, "01:30-05:00".
static std::string only_window(const Schedule &schedule) {
  if (schedule.windows.size() != 1)
    return std::to_string(schedule.windows.size()) + " windows";
  return format_hhmm(schedule.windows[0].start, VILNIUS_STANDARD_OFFSET) + "-" +
         format_hhmm(schedule.windows[0].end, VILNIUS_STANDARD_OFFSET);
}

static void test_schedule_picks_the_night_trough() {
  const PriceTable prices = two_days();
  ScheduleRequest request = overnight(SEP24_1700Z);
  const Schedule schedule = make_schedule(prices, request);
  // 30 kWh at 11 kW x 90% = 12.1 -> 13 quarter-hours, plus one buffer slot.
  CHECK(schedule.valid && schedule.needed_slots == 14 && schedule.unpriced_slots == 0);
  CHECK_STR(only_window(schedule), "01:30-05:00");
  CHECK(make_schedule(prices, overnight(SEP24_1700Z, 47)).needed_slots == 11);  // 24.75 kWh: exactly 10 quarter-hours
  ScheduleRequest slow = request;
  slow.settings.charging_kw = 2.3f;  // 30 kWh takes 58 quarter-hours: all 44 to 07:00
  CHECK_STR(only_window(make_schedule(prices, slow)), "20:00-07:00");

  request.soc = 80;  // at the limit: nothing to charge
  const Schedule full = make_schedule(prices, request);
  CHECK(full.valid && full.needed_slots == 0 && full.windows.empty());
  request.soc = 79.95f;  // within 0.05 kWh of it: nothing either
  CHECK(make_schedule(prices, request).needed_slots == 0);
  request.soc = 79.9f;  // 0.075 kWh short: a quarter-hour and the buffer
  CHECK(make_schedule(prices, request).needed_slots == 2);
  CHECK(!make_schedule(PriceTable(), overnight(SEP24_1700Z + 60)).valid);  // no price for this quarter-hour
}

static void test_schedule_waits_for_prices_not_out_yet() {
  PriceTable prices;
  add_day(prices, CET_SEP24);
  const int64_t now = SEP24_1700Z - 7 * HOUR;  // 13:00 local: prices run to 01:00, tomorrow's aren't out
  // 01:00-07:00 has no prices yet but could do all 14 quarter-hours, so it buys nothing yet.
  const Schedule waiting = make_schedule(prices, overnight(now));
  CHECK(waiting.valid && waiting.windows.empty() && waiting.unpriced_slots == 24);
  // 10% to 100% needs 29: it buys the 5 that 01:00-07:00 can't do, in the midday dip.
  ScheduleRequest request = overnight(now, 10, 100);
  CHECK_STR(only_window(make_schedule(prices, request)), "13:00-14:15");
  request.settings.charging_kw = 2.3f;  // needs 132: all 48 quarter-hours with a price, to 01:00
  CHECK_STR(only_window(make_schedule(prices, request)), "13:00-01:00");
  // Once tomorrow's prices are out, the night trough.
  add_day(prices, CET_SEP25);
  CHECK_STR(only_window(make_schedule(prices, overnight(now))), "01:30-05:00");
}

// 20:00 Thursday: 10 ct at 01:00-02:00, 11 ct at 05:00, 50 ct otherwise, until 07:00 Friday.
static PriceTable two_cheap_spells() {
  return prices_from(SEP24_1700Z, SEP24_1700Z + 11 * HOUR, [](int64_t after) {
    return after >= 5 * HOUR && after < 6 * HOUR ? 0.10f : after == 9 * HOUR ? 0.11f : 0.50f;
  });
}

static void test_schedule_windows_and_prices() {
  ScheduleRequest request = overnight(SEP24_1700Z, 67);
  // 9.75 kWh into the battery at 2.475 kWh a slot: 4 slots and the buffer, 01:00-02:00 and 05:00.
  const Schedule schedule = make_schedule(two_cheap_spells(), request);
  CHECK(schedule.needed_slots == 5);
  REQUIRE(schedule.windows.size() == 2);
  const Window &w = schedule.windows[0];
  CHECK(w.start == SEP24_1700Z + 5 * HOUR && w.end == SEP24_1700Z + 6 * HOUR);
  CHECK(!schedule.contains(w.start - 1) && schedule.contains(w.start) && schedule.contains(w.end - 1) &&
        !schedule.contains(w.end) && !schedule.contains(w.end + 1));
  CHECK(near(schedule.windows[0].energy_kwh, 9.75f / 0.9f, 1e-3f));  // full within the first window
  CHECK(schedule.windows[1].energy_kwh < 1e-6f);                     // so 05:00 is spare
  CHECK(near(schedule.avg_price, 0.10f));
  const std::string first = std::to_string(SEP24_1700Z + 5 * HOUR) + "," + std::to_string(SEP24_1700Z + 6 * HOUR);
  const std::string spare =
      std::to_string(SEP24_1700Z + 9 * HOUR) + "," + std::to_string(SEP24_1700Z + 9 * HOUR + SLOT_SECONDS);
  CHECK_STR(format_windows(schedule, "EUR"), "EUR;" + first + ",0.100;" + spare + ",0.110,spare");

  // At 01:05 the current slot has 10 minutes left, so the buffer at 05:00 is needed after all.
  request.now = SEP24_1700Z + 5 * HOUR + 5 * 60;
  const Schedule late = make_schedule(two_cheap_spells(), request);
  REQUIRE(late.windows.size() == 2);
  const float stored_first = 11 * (10.0f / 60) * 0.9f + 3 * 2.475f;  // 9.075 kWh by 02:00
  CHECK(near(late.windows[0].energy_kwh, stored_first / 0.9f, 1e-3f));
  CHECK(near(late.windows[1].energy_kwh, (9.75f - stored_first) / 0.9f, 1e-3f));
  CHECK(near(late.avg_price, (stored_first * 0.10f + (9.75f - stored_first) * 0.11f) / 9.75f, 1e-5f));
  CHECK_STR(format_windows(late, "EUR"), "EUR;" + first + ",0.100;" + spare + ",0.110");

  // A window's price is for the energy the car takes there: all of 01:00, half of 01:15, none of the
  // spare 01:30 (0.10, 0.12 and 0.40 EUR).
  const PriceTable mixed = prices_from(SEP24_1700Z, SEP24_1700Z + 11 * HOUR, [](int64_t after) {
    return after == 5 * HOUR                      ? 0.10f
           : after == 5 * HOUR + SLOT_SECONDS     ? 0.12f
           : after == 5 * HOUR + 2 * SLOT_SECONDS ? 0.40f
                                                  : 0.50f;
  });
  const Schedule m = make_schedule(mixed, overnight(SEP24_1700Z, 80 - 1.5f * 2.475f / 75 * 100));  // 1.5 slots to go
  REQUIRE(m.windows.size() == 1);
  CHECK(m.windows[0].end - m.windows[0].start == 3 * SLOT_SECONDS);
  CHECK(near(m.windows[0].avg_price, (2.75f * 0.10f + 1.375f * 0.12f) / 4.125f, 1e-5f));
  // 0.45 kWh to go fits in 01:00: 01:00-01:30 costs 01:00's 10 ct, also when that's under a kWh.
  const Schedule small = make_schedule(mixed, overnight(SEP24_1700Z, 80 - 0.45f / 75 * 100));
  REQUIRE(small.windows.size() == 1);
  CHECK(small.windows[0].end - small.windows[0].start == 2 * SLOT_SECONDS);
  CHECK(near(small.windows[0].avg_price, 0.10f, 1e-5f) && near(small.avg_price, 0.10f, 1e-5f));

  request.soc = 80;
  CHECK_STR(format_windows(make_schedule(two_cheap_spells(), request), "EUR"), "EUR");  // nothing to charge
}

// ---------------------------------------------------------------------------
// Tariff: VAT, and the tariff's rates from a plan and the settings
// ---------------------------------------------------------------------------

// ESO's 2026 plans (prices in EUR/kWh with VAT), the four-zone one with Lithuania's public holidays
// of 2026.
static const char *const FOUR_ZONES = R"(# ESO's Standartinis schedule with four zones
currency: EUR
calendar:
  jan-dec:
    mon-fri: night 05:00 morning 07:00 day 17:00 evening 22:00 night
    sat-sun: night 07:00 day 22:00 night
exceptions:
  01-01: night 07:00 day 22:00 night
  02-16: night 07:00 day 22:00 night
  03-11: night 07:00 day 22:00 night
  04-06: night 07:00 day 22:00 night
  05-01: night 07:00 day 22:00 night
  06-24: night 07:00 day 22:00 night
  07-06: night 07:00 day 22:00 night
  08-15: night 07:00 day 22:00 night
  11-01: night 07:00 day 22:00 night
  11-02: night 07:00 day 22:00 night
  12-24: night 07:00 day 22:00 night
  12-25: night 07:00 day 22:00 night
  12-26: night 07:00 day 22:00 night
rates:
  night: 0.06292
  morning: 0.08349
  day: 0.10406
  evening: 0.14641
)";
static const char *const TWO_ZONES = R"(currency: EUR
clock: winter
calendar:
  jan-dec:
    mon-fri: night 07:00 day 23:00 night
    sat-sun: night
rates:
  night: 0.07139
  day: 0.12947
)";
static const char *const ONE_ZONE = R"(currency: EUR
calendar:
  jan-dec:
    mon-sun: flat
rates:
  flat: 0.11132
)";

// What's wrong with a plan and the settings' own tariff together, as the board puts them, or "".
static std::string tariff_error(const std::string &plan, const std::string &own = "", const char *currency = "EUR") {
  Tariff ignored;
  return make_tariff(plan, own, currency, ignored);
}

// The tariff of a plan and the settings' own tariff, with 21% VAT.
static Tariff tariff_of(const std::string &plan, const std::string &own = "") {
  Tariff tariff;
  CHECK_STR(make_tariff(plan, own, "EUR", tariff), "");
  tariff.vat = 0.21f;
  return tariff;
}
static Tariff four_zones() { return tariff_of(FOUR_ZONES); }
static Tariff two_zones() { return tariff_of(TWO_ZONES); }
static Tariff one_zone() { return tariff_of(ONE_ZONE); }

// The tariff's fee at a Vilnius date and clock time.
static float fee_at(const Tariff &tariff, int year, unsigned month, unsigned day, int hour, int minute = 0) {
  const int64_t t = local_to_utc(days_from_civil(year, month, day), hour * 60 + minute, VILNIUS_STANDARD_OFFSET);
  return tariff_fee(t, tariff, VILNIUS_STANDARD_OFFSET);
}

// A whole calendar of one week, with the rates that make it whole.
static std::string calendar_of(const char *week, const char *rates = "flat: 0.1") {
  return std::string("calendar:\n  jan-dec:\n") + week + "\nrates:\n  " + rates + "\n";
}

using Entries = std::vector<std::pair<std::string, std::string>>;

static void test_reads_tariff_settings() {
  TariffText settings;
  CHECK_STR(read_tariff("# a comment\r\n\ncurrency: EUR  \nclock: winter\ncalendar:\n  jan-dec:\n    # the days\n"
                        "    mon-sun:   flat 07:00 day\nexceptions:\n  12-25: flat\nrates:\n  flat: 0.1\n  day: 0.2\n",
                        settings),
            "");
  CHECK_STR(settings.currency, "EUR");
  CHECK_STR(settings.clock, "winter");
  REQUIRE(settings.calendar.size() == 1);
  CHECK_STR(settings.calendar[0].first, "jan-dec");
  CHECK((settings.calendar[0].second == Entries{{"mon-sun", "flat 07:00 day"}}));
  CHECK((settings.exceptions == Entries{{"12-25", "flat"}}));
  CHECK((settings.rates == Entries{{"flat", "0.1"}, {"day", "0.2"}}));
  TariffText none;
  CHECK_STR(read_tariff("", none) + read_tariff("\n  \n# only a comment\n", none), "");
  CHECK(none.calendar.empty() && none.rates.empty() && none.clock.empty());

  struct Case {
    const char *text;
    const char *error;
  };
  for (const Case &c : {
           Case{"calendar\n", "line 1 isn't a key and a value"},
           Case{"rates:\n  night:\t0.1\n", "line 2 isn't a key and a value"},
           Case{"calendar:\n  jan-dec\n", "line 2 isn't a key and a value"},
           Case{"calendar:\n  : x\n", "line 2 isn't a key and a value"},
           Case{"currency:X\n", "line 1 isn't a key and a value"},
           Case{"currency:\n", "line 1 doesn't belong there: currency"},
           Case{"calendar: x\n", "line 1 doesn't belong there: calendar"},
           Case{"\tcalendar:\n", "line 1 doesn't belong there: \tcalendar"},
           Case{"  calendar:\n", "line 1 doesn't belong there: calendar"},
           Case{"zones:\n", "line 1 doesn't belong there: zones"},
           Case{"  currency: EUR\n", "line 1 doesn't belong there: currency"},
           Case{"calendar:\n    mon-sun: flat\n", "line 2 doesn't belong there: mon-sun"},
           Case{"calendar:\n  jan-dec: flat\n", "line 2 doesn't belong there: jan-dec"},
           Case{"calendar:\n  jan-dec:\n   mon-sun: flat\n", "line 3 doesn't belong there: mon-sun"},
           Case{"calendar:\n  jan-dec:\n    mon-sun:\n", "line 3 doesn't belong there: mon-sun"},
           Case{"calendar:\n  jan-dec:\n      mon-sun: flat\n", "line 3 doesn't belong there: mon-sun"},
           Case{"exceptions:\n  12-25:\n", "line 2 doesn't belong there: 12-25"},
           Case{"calendar:\n  jan-dec:\nrates:\n    mon-sun: flat\n", "line 4 doesn't belong there: mon-sun"},
           Case{"calendar:\njan-dec:\n", "line 2 doesn't belong there: jan-dec"},
           Case{"rates:\nnight: 0.1\n", "line 2 doesn't belong there: night"},
           Case{"calendar:\n  jan-dec:\nclock: winter\n    mon-sun: flat\n", "line 4 doesn't belong there: mon-sun"},
       }) {
    TariffText ignored;
    CHECK_STR(read_tariff(c.text, ignored), c.error);
  }
  // A download cut off inside its last line doesn't read, though it could still make a tariff: here ESO's evening rate,
  // 0.14641, cut to 0.1.
  const std::string whole = FOUR_ZONES;
  TariffText cut;
  CHECK_STR(read_tariff(whole.substr(0, whole.find("4641\n")), cut), "the text ends inside a line, as if cut off");
}

static void test_plan_downloads() {
  // At the first chance, but not before the clock is set. Then daily after a try that brought a plan the board can
  // use, and hourly after any other, also when the one before brought a plan.
  CHECK(!plan_due(0, 0, false));
  CHECK(plan_due(SEP24_1700Z, 0, false));
  CHECK(!plan_due(SEP24_1700Z + HOUR - 1, SEP24_1700Z, false));
  CHECK(plan_due(SEP24_1700Z + HOUR, SEP24_1700Z, false));
  CHECK(!plan_due(SEP24_1700Z + DAY_SECONDS - 1, SEP24_1700Z, true));
  CHECK(plan_due(SEP24_1700Z + DAY_SECONDS, SEP24_1700Z, true));
}

static void test_makes_tariffs() {
  // Your rates replace the plan's, and add to them.
  const Tariff cheaper = tariff_of(FOUR_ZONES, "rates:\n  night: 0.05\n");
  CHECK(near(fee_at(cheaper, 2026, 9, 24, 3), 0.05f));
  CHECK(near(fee_at(cheaper, 2026, 9, 24, 12), 0.10406f));
  const Tariff peak = tariff_of(FOUR_ZONES, "exceptions:\n  12-28: peak\nrates:\n  peak: 0.2\n");
  CHECK(near(fee_at(peak, 2026, 12, 28, 3), 0.2f));
  // Your exceptions replace the plan's on the same date, and add others.
  const Tariff special = tariff_of(FOUR_ZONES, "exceptions:\n  12-31: night 07:00 day 22:00 night\n  12-25: evening\n");
  CHECK(near(fee_at(special, 2026, 12, 31, 18), 0.10406f));  // a Thursday, now a holiday
  CHECK(near(fee_at(special, 2026, 12, 25, 3), 0.14641f));
  CHECK(near(fee_at(special, 2026, 12, 24, 18), 0.10406f));  // the plan's holiday stays
  // Your calendar replaces the plan's whole calendar, and your clock its clock.
  const Tariff own_week = tariff_of(TWO_ZONES, "calendar:\n  jan-dec:\n    mon-sun: day\n");
  CHECK(near(fee_at(own_week, 2026, 9, 26, 3), 0.12947f));
  CHECK(near(fee_at(tariff_of(TWO_ZONES, "clock: local\n"), 2026, 9, 24, 7, 30), 0.12947f));
  CHECK(near(fee_at(tariff_of(TWO_ZONES, "calendar:\n  jan-dec:\n    mon-sun: night  07:00   day\n"), 2026, 9, 24, 8),
             0.12947f));
  // Without a plan, your settings are the whole tariff; a plan's prices are in its currency.
  CHECK(near(fee_at(tariff_of("", calendar_of("    mon-sun: flat", "flat: 0.2")), 2026, 9, 24, 3), 0.2f));
  CHECK_STR(tariff_error(FOUR_ZONES, "", "NOK"), "the plan's prices are in EUR, not NOK");
  CHECK_STR(tariff_error("currency: NOK\n" + calendar_of("    mon-sun: flat")),
            "the plan's prices are in NOK, not EUR");
  CHECK_STR(tariff_error("currency: EUR\ncalendar:\n  jan-dec:\n    mon-sun: flat\n", "", "NOK"),
            "calendar: jan-dec: mon-sun: rate flat has no price");
  CHECK_STR(tariff_error("calendar\n"), "line 1 isn't a key and a value");
  // A rate the plan renamed or dropped, which yours still sets.
  CHECK_STR(tariff_error(FOUR_ZONES, "rates:\n  nakts: 0.05\n"), "rate nakts isn't used on any day");

  std::string rates = "rates:";
  for (char rate = 'a'; rate <= 'z'; rate++)
    rates += std::string("\n  ") + rate + ": 0.1";
  std::string week = "    mon-sun: a";  // a new rate every quarter-hour from 00:15 to 06:15
  for (char rate = 'b'; rate <= 'z'; rate++) {
    char when[16];
    std::snprintf(when, sizeof(when), " %02d:%02d ", (rate - 'a') / 4, (rate - 'a') % 4 * 15);
    week += when + std::string(1, rate);
  }
  CHECK_STR(tariff_error("", "calendar:\n  jan-dec:\n" + week + "\n" + rates + "\n"), "");
  CHECK_STR(tariff_error("", "calendar:\n  jan-dec:\n" + week + "\n" + rates + "\n  extra: 0.1\n"),
            "rate extra: a rate is there twice, or there are more than 26");

  CHECK_STR(tariff_error("", ""), "");  // no plan and no calendar: no grid fees
  struct Case {
    std::string own;
    const char *error;
  };
  for (const Case &c : {
           Case{calendar_of("    mon-sun: flat") + "clock: abc\n", "clock is local or winter, not abc"},
           Case{calendar_of("    mon-sun: flat") + "clock: zone\n", "clock is local or winter, not zone"},
           Case{calendar_of("    mon-sun: flat", "flat: 1000000"), "rate flat: 1000000 isn't a price per kWh"},
           Case{calendar_of("    mon-sun: flat", "flat: x"), "rate flat: x isn't a price per kWh"},
           Case{calendar_of("    mon-sun: flat", "flat: 0.1x"), "rate flat: 0.1x isn't a price per kWh"},
           Case{calendar_of("    mon-sun: flat", "flat: -0.1"), "rate flat: -0.1 isn't a price per kWh"},
           Case{calendar_of("    mon-sun: flat", "flat: 1e9"), "rate flat: 1e9 isn't a price per kWh"},
           Case{calendar_of("    mon-sun: flat", "flat: nan"), "rate flat: nan isn't a price per kWh"},
           Case{"exceptions:\n  12-25: flat\nrates:\n  flat: 0.1\n", "the calendar needs every month"},
           Case{"calendar:\n  jan-jun:\n    mon-sun: flat\nrates:\n  flat: 0.1\n", "the calendar needs every month"},
           Case{"calendar:\n  jan-jun:\n    mon-sun: flat\n  jun-dec:\n    mon-sun: flat\nrates:\n  flat: 0.1\n",
                "calendar: jun-dec has a month that's in the calendar already"},
           Case{"calendar:\n  jan-dex:\n    mon-sun: flat\n",
                "calendar: jan-dex isn't a month or a range like nov-mar"},
           Case{"calendar:\n  january:\n    mon-sun: flat\n",
                "calendar: january isn't a month or a range like nov-mar"},
           Case{calendar_of("    mon-fri: flat"), "calendar: jan-dec needs every day of the week"},
           Case{calendar_of("    mon-sun: flat\n    sun: flat"),
                "calendar: jan-dec: sun has a day that's there already"},
           Case{calendar_of("    weekend: flat"), "calendar: jan-dec: weekend isn't a day or a range like mon-fri"},
           Case{calendar_of("    mon-: flat"), "calendar: jan-dec: mon- isn't a day or a range like mon-fri"},
           Case{calendar_of("    xyz-mon: flat"), "calendar: jan-dec: xyz-mon isn't a day or a range like mon-fri"},
           Case{calendar_of("    mon-sun: flat 07:00"),
                "calendar: jan-dec: mon-sun: \"flat 07:00\" isn't rates and times by turns, like night 07:00 day"},
           Case{calendar_of("    mon-sun: dark"), "calendar: jan-dec: mon-sun: rate dark has no price"},
           Case{calendar_of("    mon-sun: flat 07:00 dark"), "calendar: jan-dec: mon-sun: rate dark has no price"},
           Case{calendar_of("    mon-sun: flat 7:00 flat"),
                "calendar: jan-dec: mon-sun: 7:00 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 07:10 flat"),
                "calendar: jan-dec: mon-sun: 07:10 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 07:60 flat"),
                "calendar: jan-dec: mon-sun: 07:60 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 07-00 flat"),
                "calendar: jan-dec: mon-sun: 07-00 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 0x:00 flat"),
                "calendar: jan-dec: mon-sun: 0x:00 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 07:x0 flat"),
                "calendar: jan-dec: mon-sun: 07:x0 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 00:00 flat"),
                "calendar: jan-dec: mon-sun: 00:00 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 24:00 flat"),
                "calendar: jan-dec: mon-sun: 24:00 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 07:00 flat 07:00 flat"),
                "calendar: jan-dec: mon-sun: 07:00 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 07:00 flat 06:45 flat"),
                "calendar: jan-dec: mon-sun: 06:45 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 07:00 flat 08:00"),
                "calendar: jan-dec: mon-sun: \"flat 07:00 flat 08:00\" isn't rates and times by turns, like night "
                "07:00 day"},
           Case{calendar_of("    mon-sun: flat 0::00 flat"),
                "calendar: jan-dec: mon-sun: 0::00 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 1/:00 flat"),
                "calendar: jan-dec: mon-sun: 1/:00 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 07:0? flat"),
                "calendar: jan-dec: mon-sun: 07:0? isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 07:000 flat"),
                "calendar: jan-dec: mon-sun: 07:000 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 07x00 flat"),
                "calendar: jan-dec: mon-sun: 07x00 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat 25:00 flat"),
                "calendar: jan-dec: mon-sun: 25:00 isn't a later quarter-hour, like 07:00 or 22:15"},
           Case{calendar_of("    mon-sun: flat") + "exceptions:\n  01+01: flat\n",
                "exceptions: 01+01 isn't a date like 12-25, or it's there twice"},
           Case{calendar_of("    mon-sun: flat") + "exceptions:\n  13-01: flat\n",
                "exceptions: 13-01 isn't a date like 12-25, or it's there twice"},
           Case{calendar_of("    mon-sun: flat") + "exceptions:\n  00-10: flat\n",
                "exceptions: 00-10 isn't a date like 12-25, or it's there twice"},
           Case{calendar_of("    mon-sun: flat") + "exceptions:\n  01-00: flat\n",
                "exceptions: 01-00 isn't a date like 12-25, or it's there twice"},
           Case{calendar_of("    mon-sun: flat") + "exceptions:\n  1-01: flat\n",
                "exceptions: 1-01 isn't a date like 12-25, or it's there twice"},
           Case{calendar_of("    mon-sun: flat") + "exceptions:\n  01/01: flat\n",
                "exceptions: 01/01 isn't a date like 12-25, or it's there twice"},
           Case{calendar_of("    mon-sun: flat") + "exceptions:\n  01-011: flat\n",
                "exceptions: 01-011 isn't a date like 12-25, or it's there twice"},
           Case{calendar_of("    mon-sun: flat") + "exceptions:\n  12-25: dark\n",
                "exceptions: 12-25: rate dark has no price"},
           Case{calendar_of("    mon-sun: flat", "flat: 0.1\n  spare: 0.2"), "rate spare isn't used on any day"},
       }) {
    CHECK_STR(tariff_error("", c.own), c.error);
  }
  CHECK_STR(tariff_error("", calendar_of("    mon-sun: flat 23:45 flat")), "");
  // Prices of nothing and above 1, like Norway's in NOK, and exceptions in any order.
  CHECK_STR(tariff_error("", calendar_of("    mon-sun: flat 12:00 free", "flat: 12.5\n  free: 0")), "");
  CHECK_STR(tariff_error("", calendar_of("    mon-sun: flat") + "exceptions:\n  12-25: flat\n  01-01: flat\n"), "");
  // The last day of each month is a date, and the day after isn't.
  const int last_day[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  for (int month = 1; month <= 12; month++) {
    char last[16], after[16];
    std::snprintf(last, sizeof(last), "%02d-%02d", month, last_day[month - 1]);
    std::snprintf(after, sizeof(after), "%02d-%02d", month, last_day[month - 1] + 1);
    CHECK_STR(tariff_error("", calendar_of("    mon-sun: flat") + "exceptions:\n  " + last + ": flat\n"), "");
    CHECK_STR(tariff_error("", calendar_of("    mon-sun: flat") + "exceptions:\n  " + after + ": flat\n"),
              std::string("exceptions: ") + after + " isn't a date like 12-25, or it's there twice");
  }
  // A key twice in a plan: the settings file turns it away before, line by line.
  CHECK_STR(tariff_error("currency: EUR\n" + calendar_of("    mon-sun: flat", "flat: 0.1\n  flat: 0.2")),
            "rate flat: a rate is there twice, or there are more than 26");
  CHECK_STR(tariff_error("currency: EUR\n" + calendar_of("    mon-sun: flat") +
                         "exceptions:\n  12-25: flat\n  12-25: flat\n"),
            "exceptions: 12-25 isn't a date like 12-25, or it's there twice");
}

static void test_eso_plans() {
  const Tariff t = four_zones();
  const float night = 0.06292f, morning = 0.08349f, day = 0.10406f, evening = 0.14641f;
  // Thursday 2026-09-24, a workday.
  CHECK(near(fee_at(t, 2026, 9, 24, 4), night));
  CHECK(near(fee_at(t, 2026, 9, 24, 5), morning));
  CHECK(near(fee_at(t, 2026, 9, 24, 7), day));
  CHECK(near(fee_at(t, 2026, 9, 24, 16), day));
  CHECK(near(fee_at(t, 2026, 9, 24, 17), evening));
  CHECK(near(fee_at(t, 2026, 9, 24, 22), night));
  // Saturday 2026-09-26 and Sunday 2026-09-27: weekend hours.
  CHECK(near(fee_at(t, 2026, 9, 26, 6), night));
  CHECK(near(fee_at(t, 2026, 9, 26, 7), day));
  CHECK(near(fee_at(t, 2026, 9, 26, 18), day));
  CHECK(near(fee_at(t, 2026, 9, 26, 22), night));
  CHECK(near(fee_at(t, 2026, 9, 27, 18), day));
  // Weekday holidays use weekend hours: Christmas Eve (a Thursday) and Easter Monday.
  CHECK(near(fee_at(t, 2026, 12, 24, 6), night));
  CHECK(near(fee_at(t, 2026, 12, 24, 18), day));
  CHECK(near(fee_at(t, 2026, 4, 6, 18), day));
  CHECK(near(fee_at(t, 2026, 4, 7, 18), evening));   // the Tuesday after is a workday again
  CHECK(near(fee_at(t, 2026, 12, 1, 17), evening));  // winter time: the same clock hours
  // 02:00 local on Friday is night: spot plus VAT, plus the night fee.
  CHECK(near(total_price(0.10f, SEP24_1700Z + 6 * HOUR, t, VILNIUS_STANDARD_OFFSET), 0.10f * 1.21f + night));
  Tariff margin = t;
  margin.margin = 0.01f;  // the supplier's, with VAT like the rates
  const float with_margin = total_price(0.10f, SEP24_1700Z + 6 * HOUR, margin, VILNIUS_STANDARD_OFFSET);
  CHECK(near(with_margin, 0.10f * 1.21f + 0.01f + night));
  CHECK(tariff_fee(SEP24_1700Z, Tariff(), VILNIUS_STANDARD_OFFSET) == 0.0f);  // no tariff: spot prices only

  const Tariff two = two_zones();
  // Summer: the day rate, 07-23 in winter time, is 08-24 on the clock.
  CHECK(near(fee_at(two, 2026, 9, 24, 7, 30), 0.07139f));
  CHECK(near(fee_at(two, 2026, 9, 24, 8), 0.12947f));
  CHECK(near(fee_at(two, 2026, 9, 24, 23, 30), 0.12947f));
  CHECK(near(fee_at(two, 2026, 9, 25, 0, 30), 0.07139f));
  // Winter: 07-23.
  CHECK(near(fee_at(two, 2026, 12, 1, 6, 30), 0.07139f));
  CHECK(near(fee_at(two, 2026, 12, 1, 7), 0.12947f));
  CHECK(near(fee_at(two, 2026, 12, 1, 22, 30), 0.12947f));
  CHECK(near(fee_at(two, 2026, 12, 1, 23), 0.07139f));
  CHECK(near(fee_at(two, 2026, 9, 26, 12), 0.07139f));  // weekends are night all day
  // ESO's two-zone meters don't know public holidays: Christmas Eve (a Thursday) is a workday.
  CHECK(near(fee_at(two, 2026, 12, 24, 12), 0.12947f));

  const Tariff one = one_zone();
  for (int hour : {3, 6, 12, 18, 23}) {
    CHECK(near(fee_at(one, 2026, 9, 24, hour), 0.11132f));
    CHECK(near(fee_at(one, 2026, 9, 26, hour), 0.11132f));
  }
}

static void test_calendar_and_exceptions() {
  // A rate for each day of the week, a range that wraps past Sunday, and an exception: Monday 21 to Sunday 27
  // December 2026, with Christmas Eve, a Thursday, at its own rate.
  const Tariff week =
      tariff_of("",
                "calendar:\n  jan-dec:\n    tue: b\n    wed: c\n    thu: d\n    fri-mon: a\n"
                "exceptions:\n  12-24: e\nrates:\n  a: 0.01\n  b: 0.02\n  c: 0.03\n  d: 0.04\n  e: 0.05\n");
  const float expected[] = {0.01f, 0.02f, 0.03f, 0.05f, 0.01f, 0.01f, 0.01f};
  for (unsigned day = 21; day <= 27; ++day)
    CHECK(near(fee_at(week, 2026, 12, day, 12), expected[day - 21]));
  CHECK(near(fee_at(week, 2026, 12, 31, 12), 0.04f));  // the next Thursday, no exception

  // Seasons: weekdays and Saturdays dearer from 07:00 to 22:00 from November to March, low the rest of the year,
  // and the same on 24 December in any year.
  const Tariff seasons = tariff_of("",
                                   "calendar:\n  apr-oct:\n    mon-sun: low\n  nov-mar:\n"
                                   "    mon-sat: low 07:00 high 22:00 low\n    sun: low\n"
                                   "exceptions:\n  12-24: high\nrates:\n  low: 0.03\n  high: 0.08\n");
  CHECK(fee_at(seasons, 2027, 1, 15, 12) == 0.08f);   // a Friday in January
  CHECK(fee_at(seasons, 2027, 1, 15, 3) == 0.03f);    // its night
  CHECK(fee_at(seasons, 2027, 1, 16, 12) == 0.08f);   // Saturday
  CHECK(fee_at(seasons, 2027, 1, 17, 12) == 0.03f);   // Sunday
  CHECK(fee_at(seasons, 2027, 3, 31, 12) == 0.08f);   // the last day of winter
  CHECK(fee_at(seasons, 2027, 4, 1, 12) == 0.03f);    // the first of summer
  CHECK(fee_at(seasons, 2027, 10, 29, 12) == 0.03f);  // a Friday in October
  CHECK(fee_at(seasons, 2027, 11, 1, 12) == 0.08f);   // Monday 1 November
  CHECK(fee_at(seasons, 2027, 12, 15, 12) == 0.08f);  // a Wednesday in December
  CHECK(fee_at(seasons, 2027, 12, 24, 3) == 0.08f);   // the exception, all day
  CHECK(fee_at(seasons, 2028, 2, 29, 12) == 0.08f);   // a leap day, a Tuesday

  // Ranges that end and start in the middle of the year, at the months' names.
  const Tariff thirds = tariff_of("",
                                  "calendar:\n  jan-may:\n    mon-sun: a\n  jun-aug:\n    mon-sun: b\n  sep-dec:\n"
                                  "    mon-sun: c\nrates:\n  a: 0.01\n  b: 0.02\n  c: 0.03\n");
  CHECK(fee_at(thirds, 2026, 5, 31, 12) == 0.01f);
  CHECK(fee_at(thirds, 2026, 6, 1, 12) == 0.02f);
  CHECK(fee_at(thirds, 2026, 8, 31, 12) == 0.02f);
  CHECK(fee_at(thirds, 2026, 9, 1, 12) == 0.03f);

  // Quarter-hours, on UK time: Octopus Go's cheap 00:30 to 05:30, and a quarter-hour from 06:45.
  const Tariff go =
      tariff_of("", calendar_of("    mon-sun: n 00:30 c 05:30 n 06:45 c 07:00 n", "n: 0.245\n  c: 0.085"));
  const auto uk = [&](unsigned month, unsigned day, int minutes) {
    return tariff_fee(local_to_utc(days_from_civil(2026, month, day), minutes, 0), go, 0);
  };
  CHECK(uk(12, 1, 29) == 0.245f);
  CHECK(uk(12, 1, 30) == 0.085f);
  CHECK(uk(12, 1, 5 * 60 + 29) == 0.085f);
  CHECK(uk(12, 1, 5 * 60 + 30) == 0.245f);
  CHECK(uk(7, 1, 30) == 0.085f);  // in summer time too
  CHECK(uk(7, 1, 29) == 0.245f);
  CHECK(uk(12, 1, 6 * 60 + 44) == 0.245f);
  CHECK(uk(12, 1, 6 * 60 + 45) == 0.085f);
  CHECK(uk(12, 1, 7 * 60) == 0.245f);
  CHECK(uk(12, 1, 23 * 60 + 59) == 0.245f);
}

// The plans in plans/, by name like lt/eso-standartinis-4-zones, as the board has them built in.
static const Plans &repository_plans() {
  static std::vector<std::pair<std::string, std::string>> texts;
  static Plans plans;
  if (plans.empty()) {
    for (const auto &entry : std::filesystem::recursive_directory_iterator("plans")) {
      if (entry.path().extension() != ".yaml")
        continue;
      std::ifstream file(entry.path());
      std::stringstream text;
      text << file.rdbuf();
      texts.emplace_back(entry.path().lexically_relative("plans").replace_extension().string(), text.str());
    }
    std::sort(texts.begin(), texts.end());
    for (const auto &[name, text] : texts)
      plans.emplace_back(name, text);
  }
  return plans;
}

static void test_plans_in_the_repository() {
  // Each plan reads on the board in its own currency, three capital letters, and uses all its rates given as a
  // settings file's own too. A settings file can name it.
  for (const auto &[name, text] : repository_plans()) {
    const std::string plan(text), label = std::string(name) + ": ";
    TariffText read;
    Tariff tariff;
    CHECK_STR(label + read_tariff(plan, read), label);
    CHECK_STR(label + make_tariff(plan, plan, read.currency, tariff), label);
    CHECK(read.currency.size() == 3 &&
          std::all_of(read.currency.begin(), read.currency.end(), [](char c) { return c >= 'A' && c <= 'Z'; }));
    const std::string car = "tesla_battery_kwh: 75\ntesla_charging_kw: 11\ntesla_vin: 5YJ3E1EA0KF000000\n";
    CHECK_STR(label + (plan_title(text).empty() ? "no name for people in its first line" : ""), label);
    SettingsFile settings;
    CHECK_STR(label + read_settings(concat({"currency: ", read.currency, "\ntariff:\n  plan: ", name, "\n", car,
                                            "timezone: Europe/Vilnius\n"}),
                                    repository_plans(), settings),
              label);
  }
  CHECK(repository_plans().size() >= 8);
}

// ---------------------------------------------------------------------------
// Settings: the file uploaded on the board's page
// ---------------------------------------------------------------------------

// A board in Vilnius with ESO's plan, line by line.
static const char *const SETTINGS =
    "# Settings\n"
    "market:\n"
    "  area: LT\n"
    "  margin: 0.016\n"
    "  vat: 0.21\n"
    "ntfy_topic: my-topic_1\n"
    "tariff:\n"
    "  plan: lt/eso-standartinis-4-zones\n"
    "tesla_battery_kwh: 75\n"
    "tesla_charging_kw: 11\n"
    "tesla_vin: 5YJ3E1EA0KF000000\n"
    "timezone: Europe/Vilnius\n";

// SETTINGS with its line that starts with `line` replaced by `with`, which may be several lines or none.
static std::string settings_with(const std::string &line, const std::string &with) {
  std::string text = SETTINGS;
  const size_t at = text.find(line);
  return text.replace(at, text.find('\n', at) + 1 - at, with);
}

// What read_settings() says is wrong with `text`, or "".
static std::string settings_error(const std::string &text) {
  SettingsFile settings;
  return read_settings(text, repository_plans(), settings);
}

static void test_reads_the_settings_file() {
  SettingsFile s;
  CHECK_STR(read_settings(SETTINGS, repository_plans(), s), "");
  CHECK(s.area == &AREAS[15] && std::string(s.area->name) == "LT" && s.currency == "EUR");
  CHECK(near(s.vat, 0.21f) && near(s.margin, 0.016f));
  CHECK(s.ntfy_server == "https://ntfy.sh" && s.ntfy_topic == "my-topic_1");
  CHECK(s.plan == "lt/eso-standartinis-4-zones" && s.plan_text.rfind("# ESO", 0) == 0);
  CHECK(s.tariff == std::string(12, '\n'));
  CHECK(s.battery_kwh == 75.0f && s.charging_kw == 11.0f && s.vin == "5YJ3E1EA0KF000000");
  CHECK(s.standard_offset == 2 * 3600);

  // settings.example.yaml, with a VIN.
  std::ifstream file("settings.example.yaml");
  std::stringstream example;
  example << file.rdbuf();
  std::string text = example.str();
  REQUIRE(text.find("tesla_vin: REPLACEME\n") != std::string::npos);
  CHECK_STR(read_settings(text.replace(text.find("REPLACEME"), 9, "5YJ3E1EA0KF000000"), repository_plans(), s), "");

  // A fixed price: no market, a tariff of its own, any currency; quotes, comments after values and Windows line
  // endings; the last line without a line break.
  const std::string fixed =
      "currency: gbp\r\n"
      "ntfy_server: 'http://192.168.1.2:8080'\n"
      "ntfy_topic: \"\"\n"
      "tariff:\n"
      "  calendar:\n"
      "    # all year\n"
      "    jan-dec:\n"
      "      mon-sun: day 00:30 night 05:30 day  # Octopus Go\n"
      "\n"
      "  rates:\n"
      "    day: 0.245\n"
      "    night: 0.085\n"
      "tesla_battery_kwh: \"82\"\n"
      "tesla_charging_kw: 7.4\n"
      "tesla_vin: '5YJ3E1EA0KF000000'\n"
      "timezone: Europe/London";
  CHECK_STR(read_settings(fixed.substr(fixed.find('\n') + 1), repository_plans(), s), "");
  CHECK(s.currency == "EUR");  // without a market or a currency of its own
  CHECK_STR(read_settings(fixed, repository_plans(), s), "");
  CHECK(s.area == nullptr && s.currency == "GBP" && s.vat == 0.0f && s.margin == 0.0f && s.plan_text.empty());
  CHECK(s.ntfy_server == "http://192.168.1.2:8080" && s.ntfy_topic.empty() && s.standard_offset == 0);
  TariffText own;
  CHECK_STR(read_tariff(s.tariff, own), "");
  CHECK(own.calendar.size() == 1 && own.calendar[0].second[0].second == "day 00:30 night 05:30 day");
  CHECK(own.rates.size() == 2 && s.battery_kwh == 82.0f && near(s.charging_kw, 7.4f));

  // A fixed price with a plan, which the board adds to every quarter-hour as it does a margin.
  const std::string fixed_with_plan =
      "fixed_price: 0.15\ntariff:\n  plan: lt/eso-standartinis-2-zones\n"
      "tesla_battery_kwh: 75\ntesla_charging_kw: 11\n"
      "tesla_vin: 5YJ3E1EA0KF000000\ntimezone: Europe/Vilnius\n";
  CHECK_STR(read_settings(fixed_with_plan, repository_plans(), s), "");
  CHECK(s.area == nullptr && near(s.margin, 0.15f) && s.plan == "lt/eso-standartinis-2-zones");

  // A market area in small letters, its own currency, and a margin left out.
  CHECK_STR(read_settings(settings_with("  area:", "  area: no1\n"), repository_plans(), s),
            "tariff: the plan lt/eso-standartinis-4-zones is in EUR: set currency: EUR");
  CHECK(s.area == nullptr);  // unchanged after an error
  const std::string norway =
      "market:\n  area: no1\n  vat: 0.25\ntariff:\n  calendar:\n    jan-dec:\n      mon-sun: "
      "flat\n  rates:\n    flat: 0.5\ntesla_battery_kwh: 75\ntesla_charging_kw: 11\n"
      "tesla_vin: 5YJ3E1EA0KF000000\ntimezone: Europe/Oslo\n";
  CHECK_STR(read_settings(norway, repository_plans(), s), "");
  CHECK(std::string(s.area->name) == "NO1" && s.currency == "NOK" && s.margin == 0.0f && s.standard_offset == 3600);
  CHECK_STR(read_settings("currency: eur\n" + norway, repository_plans(), s), "");
  CHECK(s.currency == "EUR");
  CHECK_STR(settings_error("currency: GBP\n" + norway),
            "currency: Nord Pool's prices come in DKK, EUR, NOK, PLN, RON or SEK");
  // SMARD's prices in euros, or in Czechia, Hungary and Switzerland converted into their own currency
  const std::string czech =
      "currency: czk\nmarket:\n  area: CZ\n  vat: 0.21\ntariff:\n  calendar:\n    jan-dec:\n      mon-sun: "
      "flat\n  rates:\n    flat: 2.5\ntesla_battery_kwh: 75\ntesla_charging_kw: 11\n"
      "tesla_vin: 5YJ3E1EA0KF000000\ntimezone: Europe/Prague\n";
  CHECK_STR(read_settings(czech, repository_plans(), s), "");
  CHECK(std::string(s.area->name) == "CZ" && s.currency == "CZK");
  CHECK_STR(settings_error(settings_with("  area:", "  area: CH\n") + "currency: CHF\n"),
            "tariff: the plan lt/eso-standartinis-4-zones is in EUR: set currency: EUR");
  CHECK_STR(settings_error(settings_with("  area:", "  area: HU\n") + "currency: CZK\n"),
            "currency: SMARD's prices come in EUR, or HUF at the ECB's daily rate");
  CHECK_STR(settings_error(settings_with("  area:", "  area: SI\n") + "currency: HUF\n"),
            "currency: SMARD's prices come in EUR");
  CHECK_STR(settings_error(settings_with("  area:", "  area: ES\n") + "currency: NOK\n"),
            "currency: OMIE's prices come in EUR");
  CHECK_STR(settings_error(settings_with("  area:", "  area: HU\n")), "");
  CHECK_STR(settings_error(settings_with("  area:", "  area: PT\n")), "");
}

static void test_settings_form_options() {
  CHECK_STR(plan_title("# ESO Standartinis, four zones, prices with VAT: https://www.eso.lt\ncurrency: EUR\n"),
            "ESO Standartinis, four zones");
  CHECK_STR(plan_title("# A plan\n# prices with VAT\n"), "");
  CHECK_STR(plan_title("A plan, prices with VAT\n"), "");
  CHECK_STR(json_string("a \"b\" \\ c"), "\"a \\\"b\\\" \\\\ c\"");
  const Plans plans = {{"lt/one", "# One plan, prices with VAT: https://example.com\n"}, {"lt/two", "currency: EUR\n"}};
  const std::string options = settings_options(plans);
  CHECK(options.rfind("{\"areas\":[\"AT\",\"BE\",", 0) == 0);
  CHECK(options.find("\"SI\"],\"plans\":[[\"lt/one\",\"One plan\"],[\"lt/two\",\"lt/two\"]],\"time_zones\":[") !=
        std::string::npos);
  CHECK(options.find("\"Europe/Vilnius\",") != std::string::npos);
  CHECK(options.size() > 2 && options.compare(options.size() - 17, 17, "\"Europe/Zurich\"]}") == 0);
}

static void test_settings_file_errors() {
  CHECK_STR(settings_error(SETTINGS + std::string(MAX_SETTINGS_BYTES, '#')), "the file is longer than 4 kB");
  // Lines
  CHECK_STR(settings_error(settings_with("tesla_vin", "tesla_vin 5YJ3E1EA0KF000000\n")),
            "line 11 isn't a key and a value");
  CHECK_STR(settings_error(settings_with("tesla_vin", ": 5YJ3E1EA0KF000000\n")), "line 11 isn't a key and a value");
  CHECK_STR(settings_error(settings_with("tesla_vin", "tesla_vin:5YJ3E1EA0KF000000\n")),
            "line 11 isn't a key and a value");
  CHECK_STR(settings_error(settings_with("tesla_vin", "tesla_vin:\n  x: 5YJ3E1EA0KF000000\n")),
            "line 12 doesn't belong there: x");
  CHECK_STR(settings_error(settings_with("  vat", "  vat:\n")), "line 5 doesn't belong there: vat");
  CHECK_STR(settings_error(settings_with("  vat", "    vat: 0.21\n")), "line 5 doesn't belong there: vat");
  CHECK_STR(settings_error("  tesla_vin: 5YJ3E1EA0KF000000\n"), "line 1 doesn't belong there: tesla_vin");
  CHECK_STR(settings_error(settings_with("  plan", " plan: lt/eso-standartinis-4-zones\n")),
            "line 8 doesn't belong there: plan");
  CHECK_STR(settings_error(settings_with("timezone", "timezone: Europe/Riga\ntimezone: Europe/Riga\n")),
            "line 13 has timezone again");
  CHECK_STR(settings_error(settings_with("  vat", "  vat: 0.21\n  colour: red\n")),
            "line 6 has market: colour, which isn't a setting");
  CHECK_STR(settings_error(settings_with("tariff", "tariff: none\n")), "line 8 doesn't belong there: plan");
  CHECK_STR(settings_error(std::string(SETTINGS) + "colour: red\n"), "line 13 has colour, which isn't a setting");
  // Missing: with a market, the tariff can go; without one, it's the whole price; without either, there are no
  // prices yet.
  CHECK_STR(settings_error(settings_with("  plan", "")), "");
  std::string no_tariff = SETTINGS;
  no_tariff.erase(no_tariff.find("tariff:"), std::strlen("tariff:\n  plan: lt/eso-standartinis-4-zones\n"));
  CHECK_STR(settings_error(no_tariff), "");
  const std::string no_prices = no_tariff.substr(no_tariff.find("ntfy_topic"));
  CHECK_STR(settings_error(no_prices), "");
  CHECK_STR(settings_error(no_prices + "tariff:\n  rates:\n    flat: 0.24\n"),
            "without market:, tariff needs a plan or a calendar of its own");
  CHECK_STR(settings_error(settings_with("tesla_battery_kwh", "")),
            "tesla_battery_kwh must be the battery's size in kWh, like 75");
  CHECK_STR(settings_error(settings_with("  vat", "")), "market: vat must be the VAT as a fraction, like 0.21 for 21%");
  // Market
  for (const char *area : {"  area: GB\n", ""})
    CHECK_STR(settings_error(settings_with("  area", area)),
              "market: area must be one the board knows, like LT or SE3: see Countries and plans");
  CHECK_STR(settings_error(settings_with("  vat", "  vat: -0.1\n")),
            "market: vat must be the VAT as a fraction, like 0.21 for 21%");
  CHECK_STR(settings_error(settings_with("  vat", "  vat: 1\n")),
            "market: vat must be the VAT as a fraction, like 0.21 for 21%");
  CHECK_STR(settings_error(settings_with("  vat", "  vat: \"\"\n")),
            "market: vat must be the VAT as a fraction, like 0.21 for 21%");
  CHECK_STR(settings_error(settings_with("  margin", "  margin: 1.6 ct\n")),
            "market: margin must be a price per kWh, like 0.012");
  CHECK_STR(settings_error(settings_with("  margin", "  margin: 1e7\n")),
            "market: margin must be a price per kWh, like 0.012");
  // Fixed price: without a market, and a price
  CHECK_STR(settings_error("fixed_price: 0.15\n" + std::string(SETTINGS)),
            "fixed_price goes without market:; on top of a market price, use market: margin");
  for (const char *price : {"15 ct", "-0.1", "1e7"})
    CHECK_STR(settings_error(std::string("fixed_price: ") + price + "\n" + no_prices +
                             "tariff:\n  plan: lt/eso-standartinis-2-zones\n"),
              "fixed_price must be a price per kWh, like 0.15");
  // Currency
  for (const char *currency : {"EURO", "E1R", "E[R"})
    CHECK_STR(settings_error(std::string("currency: ") + currency + "\n" + SETTINGS),
              "currency must be a currency's three-letter code, like EUR");
  // Phone messages
  for (const char *server : {"ntfy.sh", "ntfy://ntfy.sh"})
    CHECK_STR(settings_error(settings_with("ntfy_topic", std::string("ntfy_server: ") + server + "\n")),
              "ntfy_server must be the server's address, like https://ntfy.sh");
  for (const std::string &topic : {std::string("https://ntfy.sh/mine"), std::string(65, 'a')})
    CHECK_STR(settings_error(settings_with("ntfy_topic", "ntfy_topic: " + topic + "\n")),
              "ntfy_topic must be the topic's name, not its address: up to 64 letters, digits, - and _");
  // Tariff
  const std::string no_plan = settings_error(settings_with("  plan", "  plan: lt/nope\n"));
  CHECK(no_plan.rfind("tariff: there's no plan lt/nope; there are lt/eso-efektyvus-1-zone, ", 0) == 0);
  const std::string last = ", lt/eso-standartinis-2-zones, lt/eso-standartinis-4-zones";
  CHECK(no_plan.size() > last.size() && no_plan.compare(no_plan.size() - last.size(), last.size(), last) == 0);
  CHECK(no_plan.find("pl/") == std::string::npos);
  CHECK_STR(settings_error(settings_with("  plan", "  plan: xx/nope\n")), "tariff: there's no plan xx/nope");
  CHECK_STR(settings_error(settings_with("  plan", "  plan: nope\n")), "tariff: there's no plan nope");
  CHECK(settings_error(settings_with("  plan", "  plan: lt\n"))
            .rfind("tariff: there's no plan lt; there are lt/eso-efektyvus-1-zone, ", 0) == 0);
  CHECK_STR(settings_error(settings_with("  plan", "  plan: lt/eso-standartinis-4-zones\n  rates:\n    nope: 1\n")),
            "tariff: rate nope isn't used on any day");
  CHECK_STR(settings_error(settings_with("  plan", "  plan: lt/eso-standartinis-4-zones\n  rates: 1\n")),
            "tariff: line 9 doesn't belong there: rates");
  // The car and the time zone
  for (const char *kwh : {"0", "-75", "inf", "75 kWh"})
    CHECK_STR(settings_error(settings_with("tesla_battery_kwh", std::string("tesla_battery_kwh: ") + kwh + "\n")),
              "tesla_battery_kwh must be the battery's size in kWh, like 75");
  for (const char *kw : {"0", "inf"})
    CHECK_STR(settings_error(settings_with("tesla_charging_kw", std::string("tesla_charging_kw: ") + kw + "\n")),
              "tesla_charging_kw must be the charging power in kW, like 11");
  for (const char *vin : {"5YJ3E1EA0KF00000", "5YJ3E1EA0KF00000-", "5YJ3E1EA0KF00000:", "5yj3e1ea0kf000000",
                          "5YJ3E1EA0KF00000I", "5YJ3E1EA0KF00000O", "5YJ3E1EA0KF00000Q"})
    CHECK_STR(settings_error(settings_with("tesla_vin", std::string("tesla_vin: ") + vin + "\n")),
              "tesla_vin must be the car's VIN: 17 capital letters and digits, none of them I, O or Q, on the car's "
              "screen under Controls, Software");
  CHECK_STR(settings_error(settings_with("timezone", "timezone: America/New_York\n")),
            "timezone must be one the board knows, like Europe/Vilnius");
  // Quotes only around a whole value
  CHECK_STR(settings_error(settings_with("tesla_vin", "tesla_vin: \"5YJ3E1EA0KF000000'\n")),
            "tesla_vin must be the car's VIN: 17 capital letters and digits, none of them I, O or Q, on the car's "
            "screen under Controls, Software");
  CHECK_STR(settings_error(settings_with("tesla_battery_kwh", "tesla_battery_kwh: \"\n")),
            "tesla_battery_kwh must be the battery's size in kWh, like 75");
}

static void test_schedule_counts_the_tariff_fee() {
  // 50 EUR/MWh all day, but 20 EUR/MWh in the workday evening (17-22). On spot price alone the evening
  // wins; with the evening fee (14.6 ct vs 6.3 ct at night) the night does.
  const PriceTable prices = prices_from(CET_SEP24, CET_SEP25 + DAY_SECONDS, [](int64_t after) {
    const int hour = local_hour(CET_SEP24 + after);
    return hour >= 17 && hour < 22 ? 0.020f : 0.050f;
  });
  ScheduleRequest request = overnight(SEP24_1700Z - 4 * HOUR, 70);  // Thursday 16:00 local
  const Schedule spot_only = make_schedule(prices, request);
  REQUIRE(!spot_only.windows.empty());
  for (const Window &w : spot_only.windows)
    for (int64_t s = w.start; s < w.end; s += SLOT_SECONDS)
      CHECK(local_hour(s) >= 17 && local_hour(s) < 22);

  request.tariff = four_zones();
  const Schedule with_fees = make_schedule(prices, request);
  REQUIRE(!with_fees.windows.empty());
  for (const Window &w : with_fees.windows)
    for (int64_t s = w.start; s < w.end; s += SLOT_SECONDS)
      CHECK(local_hour(s) >= 22 || local_hour(s) < 5);
  CHECK(near(with_fees.avg_price, 0.050f * 1.21f + 0.06292f, 1e-5f));
}

// ---------------------------------------------------------------------------
// Controller against a simulated car
// ---------------------------------------------------------------------------

struct Step {
  int64_t at;
  std::function<void(FakeTesla &)> run;
};

struct Run {
  std::vector<std::pair<int64_t, Command>> commands;
  std::vector<int64_t> charging_at;
  std::vector<std::string> statuses;
  std::vector<std::pair<int64_t, Notification>> messages;
  FakeTesla car;
};

static Run simulate(Controller &controller, FakeTesla car, int64_t from, int64_t to, std::vector<Step> steps,
                    Settings settings = {}) {
  Run run;
  size_t next_step = 0;
  for (int64_t now = from; now < to; now += 30) {
    while (next_step < steps.size() && steps[next_step].at <= now)
      steps[next_step++].run(car);
    const Decision d = controller.tick(car.state(now), settings);
    if (d.command != Command::NONE)
      run.commands.emplace_back(now, d.command);
    if (run.statuses.empty() || run.statuses.back() != d.status)
      run.statuses.push_back(d.status);
    if (d.notification)
      run.messages.emplace_back(now, *d.notification);
    if (d.command == Command::START_CHARGING)
      car.set_charging(true);
    if (d.command == Command::STOP_CHARGING)
      car.set_charging(false);
    if (car.charging)
      run.charging_at.push_back(now);
    car.advance(30);
  }
  run.car = car;
  return run;
}

static bool contains(const std::vector<std::string> &items, const std::string &item) {
  return std::find(items.begin(), items.end(), item) != items.end();
}

static void test_updates_wait_while_charging() {
  CarState car;
  Decision d;
  CHECK(!update_due(car, d, 0));  // no clock yet
  car.now = SEP24_1700Z;
  CHECK(update_due(car, d, 0));
  CHECK(!update_due(car, d, SEP24_1700Z - HOUR + 1) && update_due(car, d, SEP24_1700Z - HOUR));
  for (const char *state : {"Charging", "Starting"}) {
    car.charging_state = state;
    CHECK(!update_due(car, d, 0));
  }
  car.charging_state = "Stopped";
  CHECK(update_due(car, d, 0));
  d.command = Command::START_CHARGING;
  CHECK(!update_due(car, d, 0));
}

static void test_charges_only_in_the_cheap_window() {
  Controller controller = with_prices();
  const Run run = simulate(controller, FakeTesla(), SEP24_1700Z - HOUR, SEP24_1700Z + 12 * HOUR,
                           {{SEP24_1700Z, &FakeTesla::plug_in}});
  REQUIRE(!run.commands.empty());
  CHECK(run.commands.front().second == Command::STOP_CHARGING);
  CHECK(run.commands.front().first - SEP24_1700Z <= 60);  // plug-in auto start stopped fast
  for (int64_t t : run.charging_at)
    CHECK(t < SEP24_1700Z + 60 || (t >= TROUGH && t < SEP24_1700Z + 9 * HOUR));  // until 05:00 local
  CHECK(run.car.soc >= 79.9f);
  CHECK(run.commands.size() <= 4);
  CHECK(contains(run.statuses, "Charges at 01:30"));
  CHECK(contains(run.statuses, "Starting"));  // the window's first tick, before the car reports charging
  CHECK(contains(run.statuses, "Charging"));
  REQUIRE(run.messages.size() == 1);
  CHECK(run.messages[0].first == SEP24_1700Z + 2 * 60);  // once the schedule has settled
  // 01:30-05:00 local is the bottom of the night trough. The car needs 12.1 of its 14 slots, which
  // average 21.2 EUR/MWh: (28+26+24+22+20+18+16+16+18+20+22+24 + 0.12*26) / 12.12.
  CHECK_STR(run.messages[0].second.title, "Tesla schedule created");
  CHECK_STR(run.messages[0].second.message, "40 to 80% by Fri 07:00; avg 0.021 EUR/kWh over 1 window(s)");
}

static void test_start_from_the_car_holds_until_unplugged() {
  Controller controller = with_prices();
  const Run run = simulate(controller, plugged_in(), SEP24_1700Z, SEP24_1700Z + 3 * HOUR,
                           {{SEP24_1700Z + HOUR, [](FakeTesla &car) { car.charging = true; }},
                            {SEP24_1700Z + 2 * HOUR, &FakeTesla::unplug},
                            {SEP24_1700Z + 2 * HOUR + 10 * 60, &FakeTesla::plug_in}});
  CHECK(std::none_of(run.commands.begin(), run.commands.end(), [](const auto &c) {
    return c.second == Command::STOP_CHARGING && c.first >= SEP24_1700Z + HOUR && c.first < SEP24_1700Z + 2 * HOUR;
  }));  // no stop while held by the start from the car
  CHECK(contains(run.statuses, "Charging now"));
  REQUIRE(!run.commands.empty());
  CHECK(run.commands.back().second == Command::STOP_CHARGING);
  CHECK(run.commands.back().first >= SEP24_1700Z + 2 * HOUR + 10 * 60);  // scheduling again after re-plugging
  // The phone hears of the start at once, then of the schedule after the re-plugging.
  REQUIRE(run.messages.size() == 2);
  CHECK(run.messages[0].first == SEP24_1700Z + HOUR);
  CHECK_STR(run.messages[0].second.message,
            "Started from the car or the Tesla app: charging to 80% at any price until you unplug");
  CHECK(run.messages[1].first > SEP24_1700Z + 2 * HOUR + 10 * 60);

  // Once per hold: a stop and a start in between send nothing more.
  Controller held = with_prices();
  FakeTesla again = plugged_in(false);
  held.tick(again.state(SEP24_1700Z), Settings());
  again.charging = true;
  CHECK(held.tick(again.state(SEP24_1700Z + 5 * 60), Settings()).notification.has_value());
  again.charging = false;
  held.tick(again.state(SEP24_1700Z + 10 * 60), Settings());
  again.charging = true;
  CHECK(!held.tick(again.state(SEP24_1700Z + 15 * 60), Settings()).notification.has_value());

  // A plug-in message still pending (the schedule waits for tomorrow's prices) doesn't follow it as a second one.
  Controller pending;
  add_day(pending.prices, CET_SEP24);
  FakeTesla later;
  pending.tick(later.state(SEP24_1700Z - 60), Settings());
  later.plugged = true;
  for (int64_t now = SEP24_1700Z; now < SEP24_1700Z + 5 * 60; now += 30)
    CHECK(!pending.tick(later.state(now), Settings()).notification.has_value());
  later.charging = true;
  CHECK(pending.tick(later.state(SEP24_1700Z + 5 * 60), Settings()).notification.has_value());
  CHECK(!pending.tick(later.state(SEP24_1700Z + 5 * 60 + 30), Settings()).notification.has_value());
  CHECK(!pending.tick(later.state(SEP24_1700Z + 6 * 60), Settings()).notification.has_value());

  // Without a battery level (Stop charging restored after a restart) the message names no limit.
  Controller blind = with_prices();
  blind.restore_mode(2);
  FakeTesla unread = plugged_in(false);
  unread.battery_known = false;
  blind.tick(unread.state(SEP24_1700Z), Settings());
  unread.charging = true;
  const Decision started = blind.tick(unread.state(SEP24_1700Z + 4 * 60), Settings());
  CHECK_STR(started.notification ? started.notification->message : "",
            "Started from the car or the Tesla app: charging at any price until you unplug");

  // In a scheduled window it stays the schedule's: here the car ignores the three starts, then starts by itself.
  Controller scheduled = with_prices();
  FakeTesla stopped = plugged_in(false);
  for (int64_t now = TROUGH - 10 * 60; now < TROUGH + 10 * 60; now += 30)
    scheduled.tick(stopped.state(now), Settings());
  stopped.charging = true;
  const Decision in_window = scheduled.tick(stopped.state(TROUGH + 10 * 60), Settings());
  CHECK_STR(in_window.mode, "schedule");
  CHECK(!in_window.notification.has_value());  // and no message about it

  // So it does while the battery level is unknown, and the schedule stops it once it's known.
  Controller reading = with_prices();
  FakeTesla unknown = plugged_in(false);
  unknown.battery_known = false;
  reading.tick(unknown.state(SEP24_1700Z), Settings());
  unknown.charging = true;
  const Decision waiting = reading.tick(unknown.state(SEP24_1700Z + 5 * 60), Settings());
  CHECK_STR(waiting.mode, "wait");
  CHECK(!waiting.notification.has_value());
  unknown.battery_known = true;
  CHECK(reading.tick(unknown.state(SEP24_1700Z + 6 * 60), Settings()).command == Command::STOP_CHARGING);
}

static void test_reads_the_charging_state() {
  // Only "Charging" and "Starting" are charging: at 20:00, not a cheap time, those it stops.
  std::string stopped;
  for (const char *state : {"Calibrating", "Charging", "Complete", "No Power", "Starting", "Stopped"}) {
    CarState car = plugged_in().state(SEP24_1700Z);
    car.charging_state = state;
    if (with_prices().tick(car, Settings()).command == Command::STOP_CHARGING)
      stopped += std::string(state) + " ";
  }
  CHECK_STR(stopped, "Charging Starting ");

  // Unknown for a moment after Stop charging, while the car still charges: not a start from the car. With
  // "Unknown", esphome-tesla-ble reports the charger as unplugged too, which is no unplug.
  for (const char *unknown : {"", "Unknown"}) {
    const FakeTesla car = plugged_in();
    Controller controller = with_prices();
    controller.tick(car.state(TROUGH), Settings());
    controller.stop_charging();
    controller.tick(car.state(TROUGH + 5 * 60), Settings());
    CarState reading = car.state(TROUGH + 6 * 60);
    reading.charging_state = unknown;
    if (reading.charging_state == "Unknown")
      reading.plugged = false;
    controller.tick(reading, Settings());
    CHECK_STR(controller.tick(car.state(TROUGH + 7 * 60), Settings()).mode, "none");
  }

  // "Unknown" says nothing about charging: in a scheduled window, no start goes out while the car charges.
  const FakeTesla in_window = plugged_in();
  Controller scheduled = with_prices();
  scheduled.tick(in_window.state(TROUGH), Settings());
  CarState unknown_reading = in_window.state(TROUGH + 5 * 60);
  unknown_reading.charging_state = "Unknown";
  unknown_reading.plugged = false;
  CHECK(scheduled.tick(unknown_reading, Settings()).command == Command::NONE);

  // Charging before the plug state is known, after a restart, is not a start from the car either.
  CarState waking = plugged_in(false).state(SEP24_1700Z);
  waking.plugged.reset();
  Controller restarted = with_prices();
  restarted.tick(waking, Settings());
  waking.charging_state = "Charging";
  waking.now += 30;
  restarted.tick(waking, Settings());
  CHECK(restarted.tick(plugged_in().state(SEP24_1700Z + 60), Settings()).command == Command::STOP_CHARGING);

  // After a restart the first reading may be "Unknown", with the charger reported as unplugged: no plug state
  // yet, and the real reading that follows is no plug-in, so Stop charging still holds.
  Controller after_unknown = with_prices();
  after_unknown.restore_mode(2);
  CarState first = plugged_in(false).state(SEP24_1700Z);
  first.charging_state = "Unknown";
  first.plugged = false;
  CHECK_STR(after_unknown.tick(first, Settings()).status, "Waiting for car");
  CHECK_STR(after_unknown.tick(plugged_in(false).state(SEP24_1700Z + 30), Settings()).mode, "none");
}

static void test_tells_its_own_starts_from_the_cars() {
  // Plugged in at 20:00, stopped. Charging within 3 minutes is the car's own start on plug-in; later it's
  // a start from the car or the Tesla app, which holds until unplugged.
  const auto mode_when_charging_after = [](int64_t seconds) {
    Controller controller = with_prices();
    FakeTesla car = plugged_in(false);
    controller.tick(car.state(SEP24_1700Z), Settings());
    car.charging = true;
    return controller.tick(car.state(SEP24_1700Z + seconds), Settings()).mode;
  };
  CHECK_STR(mode_when_charging_after(179), "schedule");
  CHECK_STR(mode_when_charging_after(180), "now");

  // The board's own start may take 5 minutes to show: Start charging now, then Create schedule.
  const auto mode_when_started_after = [](int64_t seconds) {
    Controller controller = with_prices();
    FakeTesla car = plugged_in(false);
    controller.charge_now();
    controller.tick(car.state(SEP24_1700Z), Settings());
    controller.create_schedule();
    car.charging = true;
    return controller.tick(car.state(SEP24_1700Z + seconds), Settings()).mode;
  };
  CHECK_STR(mode_when_started_after(299), "schedule");
  CHECK_STR(mode_when_started_after(300), "now");

  // Charging again soon after the board's stop is the car's or the app's start.
  Controller controller = with_prices();
  FakeTesla car = plugged_in();
  controller.tick(car.state(SEP24_1700Z), Settings());  // stops the car's own start
  car.charging = false;
  controller.tick(car.state(SEP24_1700Z + 30), Settings());
  car.charging = true;
  CHECK_STR(controller.tick(car.state(SEP24_1700Z + 200), Settings()).mode, "now");

  // A higher limit resumes a finished charge by itself: within 3 minutes, that's the car's own start too,
  // also when the car says Complete a percent under the limit. Below it, with the limit unchanged, or
  // after Stop charging, a start is the car's or the app's.
  const auto mode_when_resumed_after = [](int64_t seconds, float soc = 80, bool complete = false,
                                          bool stopped = false) {
    Controller resumed = with_prices();
    FakeTesla full = plugged_in(false);
    full.soc = soc;
    full.complete = complete;
    resumed.tick(full.state(SEP24_1700Z - HOUR), Settings());
    if (stopped)
      resumed.stop_charging();
    full.limit = 90;
    resumed.tick(full.state(SEP24_1700Z), Settings());
    full.charging = true;
    return resumed.tick(full.state(SEP24_1700Z + seconds), Settings()).mode;
  };
  CHECK_STR(mode_when_resumed_after(179), "schedule");
  CHECK_STR(mode_when_resumed_after(180), "now");
  CHECK_STR(mode_when_resumed_after(60, 79), "now");
  CHECK_STR(mode_when_resumed_after(60, 79, true), "schedule");
  CHECK_STR(mode_when_resumed_after(60, 80, false, true), "now");
  Controller unchanged = with_prices();
  FakeTesla at_limit = plugged_in(false);
  at_limit.soc = 80;
  unchanged.tick(at_limit.state(SEP24_1700Z), Settings());
  unchanged.tick(at_limit.state(SEP24_1700Z + 5 * 60), Settings());  // past the plug-in window
  at_limit.charging = true;
  CHECK_STR(unchanged.tick(at_limit.state(SEP24_1700Z + 10 * 60), Settings()).mode, "now");
}

static void test_complete_under_the_limit_is_charged() {
  // The car says Complete at 79% with the limit at 80 and takes no start: no schedule slides through the
  // night sending starts, and the message says so.
  Controller controller = with_prices();
  FakeTesla car;
  car.soc = 79;
  car.complete = true;
  const Run run =
      simulate(controller, car, SEP24_1700Z - HOUR, SEP24_1700Z + 12 * HOUR, {{SEP24_1700Z, &FakeTesla::plug_in}});
  CHECK(run.commands.empty());
  CHECK(contains(run.statuses, "Charged"));
  for (const std::string &status : run.statuses)
    CHECK(status.rfind("Charges at", 0) != 0);  // no schedule on the page either
  REQUIRE(run.messages.size() == 1);
  CHECK_STR(run.messages[0].second.message, "Not needed: battery at limit");
}

static void test_reschedules_when_no_longer_complete() {
  // A higher limit shows a tick before the car leaves Complete. The schedule is made again when it does, so a
  // charge the car resumes in the cheapest quarter-hour, 03:00, isn't stopped.
  const int64_t three = SEP24_1700Z + 7 * HOUR;
  Controller controller = with_prices();
  CarState car = plugged_in(false).state(three - 10 * 60);
  car.soc = 80;
  car.charging_state = "Complete";
  controller.tick(car, Settings());
  car.now = three;
  car.limit = 90;
  CHECK_STR(controller.tick(car, Settings()).status, "Charged");
  car.now = three + 30;
  car.charging_state = "Charging";
  car.power_kw = 11;
  const Decision resumed = controller.tick(car, Settings());
  CHECK(resumed.command == Command::NONE);
  CHECK_STR(resumed.status, "Charging");
}

static void test_reschedules_when_charging_runs_slow() {
  // The schedule counts on 16.5 kW and the car takes 11: each quarter-hour's schedule makes up for it.
  Settings optimistic;
  optimistic.charging_kw = 16.5f;
  Controller controller = with_prices();
  const Run run = simulate(controller, FakeTesla(), SEP24_1700Z - 60, SEP24_1700Z + 11 * HOUR,
                           {{SEP24_1700Z, &FakeTesla::plug_in}}, optimistic);
  CHECK(run.car.soc >= 79.9f);
  CHECK(run.commands.size() <= 3);
}

static void test_no_start_for_a_windows_last_minutes() {
  // 20:00 is cheap and 20:15 dear: a schedule made at 20:14:30 sends no start that the next quarter-hour's
  // schedule would stop, and names its next window; a car already charging is left alone.
  const auto price_at = [](int64_t t) { return t < SLOT_SECONDS || (t >= 5 * HOUR && t < 6 * HOUR) ? 0.001f : 0.5f; };
  FakeTesla car = plugged_in(false);
  car.soc = 78;
  Controller stopped = with_prices(prices_from(SEP24_1700Z, SEP24_1700Z + 12 * HOUR, price_at));
  const Decision late = stopped.tick(car.state(SEP24_1700Z + 14 * 60 + 30), Settings());
  CHECK(late.command == Command::NONE);
  CHECK_STR(late.status, "Charges at 01:00");
  car.charging = true;
  Controller charging = with_prices(prices_from(SEP24_1700Z, SEP24_1700Z + 12 * HOUR, price_at));
  CHECK(charging.tick(car.state(SEP24_1700Z + 13 * 60 + 30), Settings()).command == Command::NONE);

  // A car that stops within half a percent of the limit is Charged, also in the window's last minutes.
  FakeTesla stops = plugged_in(false);
  stops.soc = 78;
  Controller full = with_prices(prices_from(SEP24_1700Z, SEP24_1700Z + 12 * HOUR, price_at));
  full.tick(stops.state(SEP24_1700Z), Settings());
  stops.soc = 79.6f;
  CHECK_STR(full.tick(stops.state(SEP24_1700Z + 14 * 60 + 30), Settings()).status, "Charged");
}

static void test_charges_as_usual_without_prices() {
  Controller controller;
  const Run run = simulate(controller, plugged_in(), SEP24_1700Z, SEP24_1700Z + HOUR, {});
  CHECK(run.commands.empty());
  CHECK(run.charging_at.size() == 120);
  CHECK(contains(run.statuses, "Charging (no prices)"));
}

static void test_waits_for_tomorrows_prices() {
  Controller controller;
  add_day(controller.prices, CET_SEP24);
  const int64_t noon = SEP24_1700Z - 8 * HOUR, published = SEP24_1700Z - 6 * HOUR + 5 * 60;  // 12:00 and 14:05 local
  const Run run = simulate(controller, FakeTesla(), noon - 60, SEP24_1700Z,
                           {{noon, &FakeTesla::plug_in}, {published, [&controller](FakeTesla &) {
                                                            add_day(controller.prices, CET_SEP25);
                                                            controller.reschedule();
                                                          }}});
  for (int64_t t : run.charging_at)
    CHECK(t < noon + 60);  // only the plug-in auto start
  CHECK(contains(run.statuses, "Waiting for prices") && contains(run.statuses, "Charges at 01:30"));
  REQUIRE(run.messages.size() == 1);
  CHECK(run.messages[0].first == published);  // the message waits for the schedule
}

static void test_fetch_prices_due() {
  Controller controller;
  const int64_t noon = SEP24_1700Z - 8 * HOUR;             // 12:00 local
  CHECK(!controller.fetch_prices_due(0));                  // no clock yet
  CHECK(controller.fetch_prices_due(noon));                // no prices: at once,
  CHECK(!controller.fetch_prices_due(noon + 5 * 60 - 1));  // then every 5 minutes
  CHECK(controller.fetch_prices_due(noon + 5 * 60));
  CHECK(controller.fetch_prices_due(noon + 15 * 60));
  add_day(controller.prices, CET_SEP24);                         // today's, to 01:00
  CHECK(!controller.fetch_prices_due(noon + 15 * 60 + HOUR));    // tomorrow's: not before 12:45 CET
  const int64_t publication = SEP24_1700Z - 6 * HOUR - 15 * 60;  // 12:45 CET: every 5 minutes
  CHECK(controller.fetch_prices_due(publication));
  CHECK(!controller.fetch_prices_due(publication + 4 * 60));
  CHECK(controller.fetch_prices_due(publication + 5 * 60));
  add_day(controller.prices, CET_SEP25);
  CHECK(!controller.fetch_prices_due(noon + 3 * HOUR));  // all in
  CHECK(!controller.fetch_prices_due(CET_SEP25 + 60));   // 01:00: the next delivery day,
  CHECK(!controller.fetch_prices_due(publication + 24 * HOUR - 60));
  CHECK(controller.fetch_prices_due(publication + 24 * HOUR));  // from 12:45 CET

  // A tick keeps the delivery day's prices, for its average, and drops the days before.
  Controller scheduling = with_prices();
  scheduling.tick(FakeTesla().state(SEP24_1700Z), Settings());
  CHECK(scheduling.prices.get(CET_SEP24).has_value());
  scheduling.tick(FakeTesla().state(CET_SEP25 + 60), Settings());
  CHECK(!scheduling.prices.get(CET_SEP25 - SLOT_SECONDS) && scheduling.prices.get(CET_SEP25));
}

static void test_grid_fees_alone() {
  // Without market prices, each quarter-hour costs its grid fee: two_zones()' night, 0.07139 EUR/kWh, is from 00:00 to
  // 08:00 in summer time. Plugged in at 20:00, the schedule waits for midnight.
  Controller controller;
  controller.without_market_prices();
  controller.set_tariff(two_zones());
  CHECK(!controller.fetch_prices_due(SEP24_1700Z));  // nothing to download, even before the first tick
  const Decision d = controller.tick(plugged_in(false).state(SEP24_1700Z), Settings());
  CHECK_STR(d.status, "Charges at 00:00");
  REQUIRE(!controller.schedule().windows.empty());
  CHECK(controller.schedule().windows.front().start == SEP24_1700Z + 4 * HOUR);
  CHECK(near(controller.schedule().avg_price, 0.07139f));
  // As far ahead as Nord Pool's prices go, and from the start of the delivery day, for its average.
  CHECK(controller.prices.known_until(SEP24_1700Z) == end_of_next_delivery_day(SEP24_1700Z));
  CHECK(controller.prices.get(CET_SEP24).has_value());
  CHECK(!controller.fetch_prices_due(SEP24_1700Z + 60));

  // One fee for every hour: no hour is cheaper, so it charges at once.
  Controller flat;
  flat.without_market_prices();
  flat.set_tariff(one_zone());
  flat.tick(plugged_in(false).state(SEP24_1700Z), Settings());
  REQUIRE(!flat.schedule().windows.empty());
  CHECK(flat.schedule().windows.front().start == SEP24_1700Z);
}

static void test_restart_waits_for_prices() {
  // The board restarts while the car is plugged in and stopped, waiting for a cheap slot.
  const FakeTesla car = plugged_in(false);
  Controller controller;
  const Decision starting = Controller().tick(CarState(), Settings());  // before the clock is set
  CHECK_STR(starting.status, "Starting up");
  CHECK_STR(starting.mode, "wait");
  CHECK_STR(Controller().tick(car.state(SEP24_1700Z), Settings()).mode, "wait");  // no schedule card yet
  const Run run = simulate(controller, car, SEP24_1700Z, SEP24_1700Z + 15 * 60, {});
  CHECK(contains(run.statuses, "Getting prices") && contains(run.statuses, "Charging (no prices)"));
  REQUIRE(run.commands.size() == 1);
  CHECK(run.commands[0].first == SEP24_1700Z + 10 * 60 &&
        run.commands[0].second == Command::START_CHARGING);  // only once the 10 minutes are up
  Controller restarted;
  restarted.tick(car.state(SEP24_1700Z), Settings());
  CHECK(restarted.tick(car.state(SEP24_1700Z + 10 * 60 - 5), Settings()).command == Command::NONE);
}

static void test_wakes_for_battery_level_then_charges() {
  Controller controller = with_prices();
  FakeTesla car = plugged_in(false);
  car.battery_known = false;
  const Run run = simulate(controller, car, SEP24_1700Z, SEP24_1700Z + HOUR, {});
  using Commands = std::vector<std::pair<int64_t, Command>>;
  Commands sent;
  for (const auto &[at, command] : run.commands)
    sent.emplace_back(at - SEP24_1700Z, command);
  CHECK(sent == Commands({{0, Command::WAKE},
                          {10 * 60, Command::WAKE},
                          {20 * 60, Command::WAKE},
                          {30 * 60, Command::START_CHARGING}}));
  CHECK_STR(run.statuses.back(), "Charging (battery unknown)");
  // Plugged in again, it reads the battery level afresh.
  FakeTesla again = run.car;
  again.unplug();
  controller.tick(again.state(SEP24_1700Z + HOUR), Settings());
  again.plug_in();
  CHECK_STR(controller.tick(again.state(SEP24_1700Z + HOUR + 60), Settings()).status, "Reading battery");

  FakeTesla awake = plugged_in();  // charging, so awake: no need to wake it
  awake.battery_known = false;
  CHECK(with_prices().tick(awake.state(SEP24_1700Z), Settings()).command == Command::NONE);

  // It needs both the battery level and the charge limit, and schedules as soon as it has both.
  const FakeTesla charging = plugged_in();
  CarState no_level = charging.state(SEP24_1700Z), no_limit = no_level;
  no_level.soc = NAN;
  no_limit.limit = NAN;
  CHECK_STR(with_prices().tick(no_limit, Settings()).status, "Reading battery");
  Controller levels = with_prices();
  CHECK_STR(levels.tick(no_level, Settings()).status, "Reading battery");
  CHECK(levels.tick(charging.state(SEP24_1700Z + 30), Settings()).command == Command::STOP_CHARGING);
  // When the car stops reporting them, the last ones count.
  CarState gone = charging.state(SEP24_1700Z + 60);
  gone.soc = gone.limit = NAN;
  CHECK_STR(levels.tick(gone, Settings()).status, "Charges at 01:30");
}

static void test_wakes_to_learn_the_plug_state() {
  // The board restarted while the car sleeps: no plug state or battery level, but the flap is open.
  CarState car;
  car.port_open = true;
  Controller controller = with_prices();
  std::vector<int64_t> wakes;
  for (int64_t now = SEP24_1700Z; now < SEP24_1700Z + HOUR; now += 5) {
    car.now = now;
    const Decision d = controller.tick(car, Settings());
    if (d.command == Command::WAKE)
      wakes.push_back(now - SEP24_1700Z);
    CHECK_STR(d.status, now - SEP24_1700Z < 30 * 60 ? "Checking the car" : "Waiting for car");
    CHECK_STR(d.mode, "wait");
  }
  CHECK(wakes == std::vector<int64_t>({0, 10 * 60, 20 * 60}));  // then it gives up

  CarState closed;  // with the flap closed it can't be plugged in: let it sleep
  closed.now = SEP24_1700Z;
  Controller asleep = with_prices();
  const Decision d = asleep.tick(closed, Settings());
  CHECK(d.command == Command::NONE);
  CHECK_STR(d.status, "Waiting for car");

  CarState unpaired = closed;  // nor does a car that has never reported to the board's key: it needs pairing,
  unpaired.paired = false;     // even before the clock is set
  Controller pairing = with_prices();
  for (const int64_t now : {int64_t{0}, SEP24_1700Z}) {
    unpaired.now = now;
    const Decision ask = pairing.tick(unpaired, Settings());
    CHECK(ask.command == Command::NONE);
    CHECK_STR(ask.status, "Not paired");
    CHECK_STR(ask.mode, "wait");
  }

  CarState shut;  // closing the flap stops the checks
  shut.port_open = true;
  shut.now = SEP24_1700Z;
  Controller closing = with_prices();
  CHECK(closing.tick(shut, Settings()).command == Command::WAKE);
  shut.port_open = false;
  shut.now += 10 * 60;
  const Decision after = closing.tick(shut, Settings());
  CHECK(after.command == Command::NONE);
  CHECK_STR(after.status, "Waiting for car");

  CarState opened;  // opening the flap later starts the checks then
  opened.now = SEP24_1700Z;
  Controller opening = with_prices();
  opening.tick(opened, Settings());
  opened.port_open = true;
  opened.now += HOUR;
  CHECK(opening.tick(opened, Settings()).command == Command::WAKE);
}

static void test_charge_now_ignores_the_schedule() {
  Controller controller = with_prices();
  const Run run = simulate(controller, plugged_in(), SEP24_1700Z, SEP24_1700Z + HOUR,
                           {{SEP24_1700Z + 10 * 60, [&controller](FakeTesla &) { controller.charge_now(); }}});
  REQUIRE(run.commands.size() == 2);
  CHECK(run.commands[0].second == Command::STOP_CHARGING && run.commands[1].second == Command::START_CHARGING &&
        run.commands[1].first == SEP24_1700Z + 10 * 60);
  REQUIRE(!run.charging_at.empty());
  CHECK(run.charging_at.back() == SEP24_1700Z + HOUR - 30);
}

static void test_full_within_half_a_percent() {
  // Start charging now within half a percent of the limit: nothing to start.
  FakeTesla car = plugged_in(false);
  car.soc = 79.5f;
  Controller full = with_prices();
  full.charge_now();
  const Decision d = full.tick(car.state(SEP24_1700Z), Settings());
  CHECK(d.command == Command::NONE);
  CHECK_STR(d.status, "Charged");
  car.charging = true;  // still charging there: it carries on
  CHECK_STR(full.tick(car.state(SEP24_1700Z + 30), Settings()).status, "Charging now");

  car.soc = 79.4f;
  car.charging = false;
  Controller nearly = with_prices();
  nearly.charge_now();
  CHECK(nearly.tick(car.state(SEP24_1700Z), Settings()).command == Command::START_CHARGING);
}

static void test_retries_are_rate_limited() {
  // The car ignores every start (for example, the charger has no power): one every 2 minutes, at most 3 a
  // quarter-hour, and 3 more after a button.
  Controller controller = with_prices();
  const FakeTesla car = plugged_in(false);
  std::vector<int64_t> starts;
  for (int64_t now = TROUGH; now < TROUGH + 2 * SLOT_SECONDS; now += 30) {
    if (now == TROUGH + 20 * 60)
      controller.charge_now();
    if (controller.tick(car.state(now), Settings()).command == Command::START_CHARGING)
      starts.push_back((now - TROUGH) / 60);
  }
  CHECK(starts == std::vector<int64_t>({0, 2, 4, 15, 17, 19, 20, 22, 24}));
  Controller once = with_prices();
  once.tick(car.state(TROUGH), Settings());
  CHECK(once.tick(car.state(TROUGH + 2 * 60 - 1), Settings()).command == Command::NONE);

  // The page says Starting until the car charges, and Can't start charging once the three starts of a
  // quarter-hour went unanswered, until the next quarter-hour tries again.
  Controller stuck = with_prices();
  CHECK_STR(stuck.tick(car.state(TROUGH), Settings()).status, "Starting");
  for (int64_t now = TROUGH + 30; now < TROUGH + 6 * 60; now += 30)
    stuck.tick(car.state(now), Settings());
  CHECK_STR(stuck.tick(car.state(TROUGH + 6 * 60 - 30), Settings()).status, "Starting");
  CHECK_STR(stuck.tick(car.state(TROUGH + 6 * 60), Settings()).status, "Can't start charging");
  for (int64_t now = TROUGH + 6 * 60 + 30; now < TROUGH + 15 * 60; now += 30)
    stuck.tick(car.state(now), Settings());
  CHECK_STR(stuck.tick(car.state(TROUGH + 15 * 60), Settings()).status, "Starting");
}

static void test_charger_without_power() {
  // The charger withholds power (an OCPP box waiting for approval, its own schedule): one start, so the car
  // charges as soon as power comes, then nothing for 10 minutes, and a stop when the window ends.
  FakeTesla car = plugged_in(false);
  car.no_power = true;
  Controller controller = with_prices();
  const Decision first = controller.tick(car.state(TROUGH), Settings());
  CHECK(first.command == Command::START_CHARGING);
  CHECK_STR(first.status, "Charger has no power");
  car.set_charging(true);  // the car keeps the request
  for (int64_t now = TROUGH + 30; now < TROUGH + 10 * 60; now += 30)
    CHECK(controller.tick(car.state(now), Settings()).command == Command::NONE);
  CHECK(controller.tick(car.state(TROUGH + 10 * 60), Settings()).command == Command::START_CHARGING);
  car.power_back();  // the charger supplies power: the car charges as asked
  const Decision charging = controller.tick(car.state(TROUGH + 11 * 60), Settings());
  CHECK(charging.command == Command::NONE);
  CHECK_STR(charging.status, "Charging");

  FakeTesla powerless = plugged_in(false);
  powerless.no_power = true;
  // A regular start, then the car reports No Power: it holds that request, so no second start for 10 minutes.
  FakeTesla late = plugged_in(false);
  Controller dark_after = with_prices();
  CHECK(dark_after.tick(late.state(TROUGH), Settings()).command == Command::START_CHARGING);
  late.no_power = true;
  late.set_charging(true);
  const Decision held_back = dark_after.tick(late.state(TROUGH + 30), Settings());
  CHECK_STR(held_back.status, "Charger has no power");
  CHECK(held_back.command == Command::NONE);
  for (int64_t now = TROUGH + 60; now < TROUGH + 10 * 60; now += 30)
    CHECK(dark_after.tick(late.state(now), Settings()).command == Command::NONE);
  CHECK(dark_after.tick(late.state(TROUGH + 10 * 60), Settings()).command == Command::START_CHARGING);

  // A button asks again at once: Stop charging, then Start charging now within the 10 minutes.
  Controller pressed = with_prices();
  pressed.tick(powerless.state(TROUGH), Settings());
  pressed.stop_charging();
  pressed.tick(powerless.state(TROUGH + 60), Settings());
  pressed.charge_now();
  CHECK(pressed.tick(powerless.state(TROUGH + 2 * 60), Settings()).command == Command::START_CHARGING);

  // An "Unknown" reading in between keeps the request: no start until the 10 minutes are up.
  Controller flicker = with_prices();
  CHECK(flicker.tick(powerless.state(TROUGH), Settings()).command == Command::START_CHARGING);
  CarState unknown_reading = powerless.state(TROUGH + 30);
  unknown_reading.charging_state = "Unknown";
  unknown_reading.plugged = false;
  CHECK(flicker.tick(unknown_reading, Settings()).command == Command::NONE);
  for (int64_t now = TROUGH + 60; now < TROUGH + 10 * 60; now += 30)
    CHECK(flicker.tick(powerless.state(now), Settings()).command == Command::NONE);
  CHECK(flicker.tick(powerless.state(TROUGH + 10 * 60), Settings()).command == Command::START_CHARGING);

  // Still without power once Ready by has passed and the schedule moved to the next night: a stop, so the car
  // doesn't start at a dear time when power comes.
  FakeTesla dark = plugged_in(false);
  dark.no_power = true;
  Controller ended = with_prices();
  ended.tick(dark.state(TROUGH), Settings());
  CHECK(ended.tick(dark.state(TROUGH + 6 * HOUR), Settings()).command == Command::STOP_CHARGING);
  CHECK(ended.tick(dark.state(TROUGH + 6 * HOUR + 30), Settings()).command == Command::NONE);
}

static void test_new_limit_reschedules_at_once() {
  FakeTesla car = plugged_in();
  Controller controller = with_prices();
  controller.tick(car.state(SEP24_1700Z), Settings());
  const int at_80 = controller.schedule().needed_slots;
  car.limit = 100;
  controller.tick(car.state(SEP24_1700Z + 60), Settings());        // same quarter-hour
  CHECK(at_80 == 14 && controller.schedule().needed_slots == 20);  // 40% to 100%: 45 kWh = 19 slots, plus the buffer
  car.limit = 50;
  controller.tick(car.state(SEP24_1700Z + 90), Settings());
  CHECK(controller.schedule().needed_slots == 5);  // 40% to 50%: 7.5 kWh = 4 slots, plus the buffer
}

static void test_one_off_ready_by() {
  const int64_t sat_midnight = local_to_utc(days_from_civil(2026, 9, 26), 0, VILNIUS_STANDARD_OFFSET);
  CHECK(sat_midnight == SEP24_1700Z + 28 * HOUR);
  Settings once;
  once.ready_by_once = sat_midnight;
  FakeTesla trip;
  trip.limit = 100;
  Controller controller = with_prices();
  const Run run =
      simulate(controller, trip, SEP24_1700Z - 60, SEP24_1700Z + 180, {{SEP24_1700Z, &FakeTesla::plug_in}}, once);
  // 20 quarter-hours: the night trough's 16, then 11:00-12:00 Friday, after the daily 07:00.
  const std::vector<Window> &windows = controller.schedule().windows;
  CHECK(windows.size() == 2 && windows[1].start == SEP24_1700Z + 15 * HOUR);
  REQUIRE(run.messages.size() == 1);
  CHECK_STR(run.messages[0].second.message, "40 to 100% by Sat 00:00; avg 0.026 EUR/kWh over 2 window(s)");

  const FakeTesla car = plugged_in();
  Settings stale;
  stale.ready_by_once = SEP24_1700Z;  // reached: back to the daily time
  Controller daily = with_prices();
  daily.tick(car.state(SEP24_1700Z), stale);
  CHECK_STR(only_window(daily.schedule()), "01:30-05:00");

  Settings far;
  far.ready_by_once = SEP24_1700Z + 30 * DAY_SECONDS;  // clamped to a week
  Controller week = with_prices();
  week.tick(car.state(SEP24_1700Z), far);
  CHECK(week.schedule().unpriced_slots == 7 * 96 - 29 * 4);  // a week, less the 29 hours with prices
}

static void test_schedules_with_the_grid_fees() {
  Controller controller = with_prices();
  controller.tick(FakeTesla().state(SEP24_1700Z), Settings());
  controller.set_tariff(four_zones());  // reschedules at once
  controller.tick(FakeTesla().state(SEP24_1700Z + 30), Settings());
  // The night trough (21.2 EUR/MWh for the energy the car takes, see test_charges_only_in_the_cheap_window) plus
  // VAT, plus the night fee.
  CHECK(near(controller.schedule().avg_price, 0.021215f * 1.21f + 0.06292f, 1e-4f));
}

static void test_windows_while_the_schedule_decides() {
  const FakeTesla car = plugged_in();
  Controller controller = with_prices();
  CHECK(!controller.tick(car.state(SEP24_1700Z), Settings()).windows.empty());

  Controller now = with_prices();
  now.charge_now();
  CHECK(now.tick(car.state(SEP24_1700Z), Settings()).windows.empty());  // charging regardless of the schedule

  const Decision unplugged = with_prices().tick(FakeTesla().state(SEP24_1700Z), Settings());
  CHECK(unplugged.windows.empty());
  CHECK_STR(unplugged.status, "Unplugged");
  CHECK_STR(unplugged.mode, "unplugged");
}

// ---------------------------------------------------------------------------
// The page's buttons
// ---------------------------------------------------------------------------

static void test_stop_charging_until_a_button_or_plug_in() {
  // Delete the schedule while it charges in a cheap slot: it stops at once and stays stopped.
  FakeTesla car = plugged_in();
  Controller controller = with_prices();
  CHECK_STR(controller.tick(car.state(TROUGH), Settings()).mode, "schedule");
  controller.stop_charging();
  Decision d = controller.tick(car.state(TROUGH + 30), Settings());
  CHECK(d.command == Command::STOP_CHARGING && d.windows.empty());
  CHECK_STR(d.status, "No schedule");
  CHECK_STR(d.mode, "none");
  car.charging = false;
  d = controller.tick(car.state(TROUGH + 15 * 60), Settings());  // the next cheap slot doesn't start it
  CHECK(d.command == Command::NONE);
  CHECK_STR(d.mode, "none");

  car.charging = true;  // started from the Tesla app: that's charging now
  d = controller.tick(car.state(TROUGH + 16 * 60), Settings());
  CHECK_STR(d.mode, "now");
  CHECK_STR(d.status, "Charging now");

  controller.stop_charging();  // Stop charging: at once, although the last command was recent
  d = controller.tick(car.state(TROUGH + 16 * 60 + 30), Settings());
  CHECK(d.command == Command::STOP_CHARGING);
  CHECK_STR(d.mode, "none");

  controller.create_schedule();  // Create schedule
  car.charging = false;
  CHECK_STR(controller.tick(car.state(TROUGH + 17 * 60), Settings()).mode, "schedule");

  controller.stop_charging();  // unplugging ends it, and the next plug-in schedules again
  controller.tick(car.state(TROUGH + 18 * 60), Settings());
  car.unplug();
  CHECK_STR(controller.tick(car.state(TROUGH + 19 * 60), Settings()).mode, "unplugged");
  car.plug_in();
  CHECK_STR(controller.tick(car.state(TROUGH + 20 * 60), Settings()).mode, "schedule");

  // Start charging now, then Stop charging half a minute later: the stop goes out at once.
  FakeTesla waiting = plugged_in(false);
  Controller quick = with_prices();
  quick.charge_now();
  CHECK(quick.tick(waiting.state(SEP24_1700Z), Settings()).command == Command::START_CHARGING);
  waiting.charging = true;
  quick.stop_charging();
  CHECK(quick.tick(waiting.state(SEP24_1700Z + 30), Settings()).command == Command::STOP_CHARGING);
}

static void test_buttons_skip_the_command_limits() {
  // Start, Stop, Start, Create schedule within one quarter-hour: every press reaches the car.
  FakeTesla car = plugged_in(false);
  Controller controller = with_prices();
  int64_t now = SEP24_1700Z;  // 20:00, not a cheap slot
  controller.tick(car.state(now), Settings());
  std::vector<Command> sent;
  for (const auto press :
       {&Controller::charge_now, &Controller::stop_charging, &Controller::charge_now, &Controller::create_schedule}) {
    (controller.*press)();
    sent.push_back(controller.tick(car.state(now += 30), Settings()).command);
    car.charging = !car.charging;
  }
  CHECK(sent == std::vector<Command>({Command::START_CHARGING, Command::STOP_CHARGING, Command::START_CHARGING,
                                      Command::STOP_CHARGING}));
}

static void test_buttons_hold_across_a_restart() {
  // Stopped, then the board restarts with the car still plugged in: it stays stopped, even in a cheap slot.
  FakeTesla car = plugged_in(false);
  Controller before = with_prices();
  before.tick(car.state(TROUGH - HOUR), Settings());
  before.stop_charging();
  before.tick(car.state(TROUGH - HOUR + 30), Settings());
  CHECK(before.held_mode() == 2);
  Controller after = with_prices();
  after.restore_mode(before.held_mode());
  Decision d = after.tick(car.state(TROUGH), Settings());
  CHECK(d.command == Command::NONE);
  CHECK_STR(d.mode, "none");
  car.unplug();  // a new plug-in schedules afresh
  after.tick(car.state(TROUGH + 60), Settings());
  car.plug_in();
  CHECK_STR(after.tick(car.state(TROUGH + 2 * 60), Settings()).mode, "schedule");

  // Kept from before a restart but unplugged meanwhile: the next plug-in schedules afresh too.
  Controller stale = with_prices();
  stale.restore_mode(2);
  FakeTesla away;
  stale.tick(away.state(TROUGH), Settings());
  away.plug_in();
  CHECK_STR(stale.tick(away.state(TROUGH + 60), Settings()).mode, "schedule");

  // The other persisted values: 0 follows the schedule, 1 charges now; anything else follows the schedule.
  CHECK(Controller().held_mode() == 0);
  Controller now;
  now.charge_now();
  CHECK(now.held_mode() == 1);
  for (int mode : {0, 1, 2, 7}) {
    Controller restored;
    restored.charge_now();
    restored.restore_mode(mode);
    CHECK(restored.held_mode() == (mode == 7 ? 0 : mode));
  }
}

static void test_create_schedule_cancels_charge_now() {
  const FakeTesla car = plugged_in();  // charging on its own at 20:00, an expensive time
  Controller controller = with_prices();
  controller.charge_now();
  CHECK_STR(controller.tick(car.state(SEP24_1700Z), Settings()).status, "Charging now");
  controller.create_schedule();
  const Decision d = controller.tick(car.state(SEP24_1700Z + 60), Settings());
  CHECK_STR(d.status, "Charges at 01:30");
  CHECK(d.command == Command::STOP_CHARGING && !d.windows.empty());
}

// ---------------------------------------------------------------------------
// Plug-in message
// ---------------------------------------------------------------------------

static void test_plug_in_message_after_two_minutes() {
  Controller controller = with_prices();
  FakeTesla car;
  controller.tick(car.state(SEP24_1700Z - 30), Settings());
  car.plug_in();
  controller.tick(car.state(SEP24_1700Z), Settings());
  CHECK(!controller.tick(car.state(SEP24_1700Z + 2 * 60 - 1), Settings()).notification);
  CHECK(controller.tick(car.state(SEP24_1700Z + 2 * 60), Settings()).notification.has_value());

  // Driving home: 50% at 20:00, 40% when plugged in at 20:05. The schedule starts from 40%.
  Controller driving = with_prices();
  car = FakeTesla();
  car.soc = 50;
  const Run run = simulate(driving, car, SEP24_1700Z, SEP24_1700Z + 10 * 60, {{SEP24_1700Z + 5 * 60, [](FakeTesla &c) {
                                                                                 c.soc = 40;
                                                                                 c.plug_in();
                                                                               }}});
  REQUIRE(run.messages.size() == 1);
  CHECK_STR(run.messages[0].second.message, "40 to 80% by Fri 07:00; avg 0.021 EUR/kWh over 1 window(s)");
}

static void test_plug_in_message_leaves_out_the_spare() {
  // 67% to 80% takes 01:00-02:00; the buffer at 05:00 is spare, so the message counts one window.
  Controller controller = with_prices(two_cheap_spells());
  FakeTesla car;
  car.soc = 67;
  const Run run = simulate(controller, car, SEP24_1700Z - 60, SEP24_1700Z + 180, {{SEP24_1700Z, &FakeTesla::plug_in}});
  REQUIRE(run.messages.size() == 1);
  CHECK_STR(run.messages[0].second.message, "67 to 80% by Fri 07:00; avg 0.100 EUR/kWh over 1 window(s)");

  // Plugged in at 01:05, with 10 minutes of the first slot left, the car needs 05:00 too.
  Controller late = with_prices(two_cheap_spells());
  const int64_t at = SEP24_1700Z + 5 * HOUR + 5 * 60;
  const Run later = simulate(late, car, at - 60, at + 180, {{at, &FakeTesla::plug_in}});
  REQUIRE(later.messages.size() == 1);
  CHECK(later.messages[0].second.message.find("over 2 window(s)") != std::string::npos);
}

// Plugs in at 20:00 and runs past the plug-in message.
static Run plug_in_at_8pm(Settings settings = {}, FakeTesla car = {}) {
  Controller controller = with_prices();
  return simulate(controller, car, SEP24_1700Z - 60, SEP24_1700Z + 180, {{SEP24_1700Z, &FakeTesla::plug_in}}, settings);
}

static void test_plug_in_in_cet() {
  Settings cet;
  cet.standard_offset = CET_STANDARD_OFFSET;
  cet.currency = "SEK";  // a Swedish user
  const Run run = plug_in_at_8pm(cet);
  CHECK(contains(run.statuses, "Charges at 00:30"));
  REQUIRE(run.messages.size() == 1);
  CHECK_STR(run.messages[0].second.message, "40 to 80% by Fri 07:00; avg 0.021 SEK/kWh over 1 window(s)");
}

static void test_plug_in_message_when_time_is_short() {
  // Ready by 22:00 leaves 8 quarter-hours for the 14 the car needs: it takes all 8.
  Settings soon;
  soon.ready_by_once = SEP24_1700Z + 2 * HOUR;
  const Run short_time = plug_in_at_8pm(soon);
  REQUIRE(short_time.messages.size() == 1);
  CHECK_STR(short_time.messages[0].second.message,
            "40 to 80% by Thu 22:00; avg 0.190 EUR/kWh over 1 window(s)\nNot enough time to reach the limit");

  soon.ready_by_once = SEP24_1700Z + 13 * SLOT_SECONDS;  // 23:15: 13 slots store the 12.1 the car takes
  const Run just = plug_in_at_8pm(soon);
  REQUIRE(just.messages.size() == 1);
  CHECK(just.messages[0].second.message.find("Not enough") == std::string::npos);

  soon.ready_by_once = SEP24_1700Z + 14 * SLOT_SECONDS;  // 23:30 leaves the 14
  const Run enough = plug_in_at_8pm(soon);
  REQUIRE(enough.messages.size() == 1);
  CHECK(enough.messages[0].second.message.find("Not enough") == std::string::npos);

  soon.ready_by_once = SEP24_1700Z + 10 * 60;  // 20:10, within the quarter-hour of the plug-in
  const Run no_time = plug_in_at_8pm(soon);
  REQUIRE(no_time.messages.size() == 1);
  CHECK_STR(no_time.messages[0].second.message, "No time left before Ready by");
  CHECK(contains(no_time.statuses, "Waiting"));
}

static void test_plug_in_message_at_the_limit() {
  FakeTesla full;
  full.soc = 80;
  const Run run = plug_in_at_8pm({}, full);
  REQUIRE(run.messages.size() == 1);
  CHECK_STR(run.messages[0].second.message, "Not needed: battery at limit");
  CHECK(contains(run.statuses, "Charged"));

  // The same without the car's Complete, which can arrive a tick after the battery level.
  Controller at_limit = with_prices();
  at_limit.tick(full.state(SEP24_1700Z), Settings());
  full.plugged = true;
  CarState stopped = full.state(SEP24_1700Z + 30);
  stopped.charging_state = "Stopped";
  at_limit.tick(stopped, Settings());
  stopped.now += 2 * 60;
  const Decision d = at_limit.tick(stopped, Settings());
  CHECK_STR(d.notification ? d.notification->message : "", "Not needed: battery at limit");
  full.plugged = false;

  // It doesn't wait for tomorrow's prices.
  Controller controller;
  add_day(controller.prices, CET_SEP24);
  const int64_t noon = SEP24_1700Z - 8 * HOUR;
  const Run waiting = simulate(controller, full, noon - 60, noon + 180, {{noon, &FakeTesla::plug_in}});
  REQUIRE(waiting.messages.size() == 1);
  CHECK(waiting.messages[0].first == noon + 2 * 60);
}

static void test_plug_in_message_waits_for_the_last_prices() {
  // Ready by 01:15, a quarter-hour after today's prices end: the message waits for tomorrow's.
  Controller controller;
  add_day(controller.prices, CET_SEP24);
  Settings settings;
  settings.ready_by_once = CET_SEP25 + SLOT_SECONDS;
  const int64_t noon = SEP24_1700Z - 8 * HOUR;
  CHECK(simulate(controller, FakeTesla(), noon - 60, noon + HOUR, {{noon, &FakeTesla::plug_in}}, settings)
            .messages.empty());
}

static void test_plug_in_message_when_charging_now() {
  // Start charging now doesn't wait for tomorrow's prices: the message comes as usual.
  Controller controller;
  add_day(controller.prices, CET_SEP24);
  const int64_t noon = SEP24_1700Z - 8 * HOUR;
  const Run run =
      simulate(controller, FakeTesla(), noon - 60, noon + HOUR,
               {{noon, &FakeTesla::plug_in}, {noon + 60, [&controller](FakeTesla &) { controller.charge_now(); }}});
  REQUIRE(run.messages.size() == 1);
  CHECK(run.messages[0].first == noon + 2 * 60);
  CHECK_STR(run.messages[0].second.message, "Charging now");
}

static void test_no_plug_in_message_after_stop_charging() {
  // Stop charging before the message, then Create schedule: no message for this plug-in.
  Controller controller = with_prices();
  const Run run = simulate(controller, FakeTesla(), SEP24_1700Z - 60, SEP24_1700Z + HOUR,
                           {{SEP24_1700Z, &FakeTesla::plug_in},
                            {SEP24_1700Z + 2 * 60, [&controller](FakeTesla &) { controller.stop_charging(); }},
                            {SEP24_1700Z + 5 * 60, [&controller](FakeTesla &) { controller.create_schedule(); }}});
  CHECK(run.messages.empty());
}

static void test_no_plug_in_message_after_a_restart() {
  Controller controller = with_prices();
  const FakeTesla car = plugged_in();  // already plugged in when the board starts
  CHECK(simulate(controller, car, SEP24_1700Z, SEP24_1700Z + HOUR, {}).messages.empty());

  Controller asleep = with_prices();  // also when the sleeping car reports it only later
  CarState unknown;
  unknown.now = SEP24_1700Z;
  asleep.tick(unknown, Settings());
  CHECK(simulate(asleep, car, SEP24_1700Z + 30, SEP24_1700Z + HOUR, {}).messages.empty());
}

static void test_one_plug_in_message_per_plug_in() {
  Controller controller = with_prices();
  const Run run = simulate(controller, FakeTesla(), SEP24_1700Z - HOUR, SEP24_1700Z + 2 * HOUR,
                           {{SEP24_1700Z - HOUR + 60, &FakeTesla::plug_in},
                            {SEP24_1700Z - HOUR + 90, &FakeTesla::unplug},  // unplugged before the schedule settled
                            {SEP24_1700Z, &FakeTesla::plug_in},
                            {SEP24_1700Z + HOUR, &FakeTesla::unplug},
                            {SEP24_1700Z + HOUR + 10 * 60, &FakeTesla::plug_in}});
  REQUIRE(run.messages.size() == 2);
  CHECK(run.messages[0].first == SEP24_1700Z + 2 * 60 && run.messages[1].first == SEP24_1700Z + HOUR + 12 * 60);
}

static void test_fallback_message_after_a_restart() {
  // The board restarts with the car plugged in and never gets prices: when it gives up and charges at any
  // price, the phone hears it, once.
  Controller controller;
  const Run run = simulate(controller, plugged_in(false), SEP24_1700Z, SEP24_1700Z + 15 * 60, {});
  REQUIRE(run.messages.size() == 1);
  CHECK(run.messages[0].first == SEP24_1700Z + 10 * 60);
  CHECK_STR(run.messages[0].second.message, "Charging (no prices)");

  // The same without a battery level, after the half hour of waking the car.
  FakeTesla unknown = plugged_in(false);
  unknown.battery_known = false;
  Controller waking = with_prices();
  const Run battery = simulate(waking, unknown, SEP24_1700Z, SEP24_1700Z + 35 * 60, {});
  REQUIRE(battery.messages.size() == 1);
  CHECK(battery.messages[0].first == SEP24_1700Z + 30 * 60);
  CHECK_STR(battery.messages[0].second.message, "Charging (battery unknown)");

  // A full car without prices is Charged, not charging at any price: the message waits for a higher limit.
  Controller full_controller;
  FakeTesla full = plugged_in(false);
  full.soc = 80;
  const Run charged = simulate(full_controller, full, SEP24_1700Z, SEP24_1700Z + 15 * 60, {});
  CHECK(contains(charged.statuses, "Charged"));
  CHECK(charged.messages.empty());
  full.limit = 90;
  const Run raised = simulate(full_controller, full, SEP24_1700Z + 15 * 60, SEP24_1700Z + 16 * 60, {});
  REQUIRE(raised.messages.size() == 1);
  CHECK_STR(raised.messages[0].second.message, "Charging (no prices)");
}

// Plugged in at 20:00 with 20%, a one-off Ready by at 00:00 and only today's prices, which end at 01:00.
static std::vector<std::pair<int64_t, Notification>> fallback_night(bool tomorrows_prices_come) {
  Controller controller;
  add_day(controller.prices, CET_SEP24);
  Settings tonight;
  tonight.ready_by_once = SEP24_1700Z + 4 * HOUR;
  FakeTesla car;
  car.soc = 20;
  std::vector<Step> steps{{SEP24_1700Z, &FakeTesla::plug_in}};
  if (tomorrows_prices_come)
    steps.push_back({SEP24_1700Z + 3 * HOUR, [&controller](FakeTesla &) {
                       add_day(controller.prices, CET_SEP25);
                       controller.reschedule();
                     }});
  return simulate(controller, car, SEP24_1700Z - 60, SEP24_1700Z + 5 * HOUR + 30 * 60, steps, tonight).messages;
}

static void test_fallback_message_mid_stay() {
  // When the prices run out at 01:00 with none for tomorrow, the board charges at any price and says so;
  // not when tomorrow's prices came first.
  const auto out = fallback_night(false);
  REQUIRE(out.size() == 3);
  CHECK(out[0].first == SEP24_1700Z + 2 * 60);
  CHECK(out[1].first == SEP24_1700Z + 4 * HOUR);  // 00:00, the one-off Ready by, with too little time to charge
  CHECK_STR(out[1].second.message, "Ready by passed at 73% of 80%");
  CHECK(out[2].first == CET_SEP25);  // 01:00 local, the first quarter-hour without a price
  CHECK_STR(out[2].second.message, "Charging (no prices)");
  CHECK(fallback_night(true).size() == 2);
}

static void test_message_when_ready_by_passes_short() {
  // The charger gives no power all night: at 07:00, Ready by, the phone hears it, and only then.
  const int64_t ready_by = SEP24_1700Z + 11 * HOUR;
  FakeTesla stuck;
  stuck.no_power = true;
  Controller controller = with_prices();
  const Run night =
      simulate(controller, stuck, SEP24_1700Z - 60, ready_by + 3 * HOUR, {{SEP24_1700Z, &FakeTesla::plug_in}});
  REQUIRE(night.messages.size() == 2);
  CHECK(night.messages[1].first == ready_by);
  CHECK_STR(night.messages[1].second.message, "Ready by passed at 40% of 80%");

  // None when the car reaches its limit, after Start charging now or Stop charging, or once unplugged.
  const auto messages = [&](FakeTesla car, Controller &c, std::vector<Step> steps) {
    steps.insert(steps.begin(), {SEP24_1700Z, &FakeTesla::plug_in});
    return simulate(c, car, SEP24_1700Z - 60, ready_by + HOUR, steps).messages.size();
  };
  Controller full = with_prices(), now = with_prices(), stopped = with_prices(), away = with_prices();
  CHECK(messages(FakeTesla(), full, {}) == 1);
  CHECK(messages(stuck, now, {{SEP24_1700Z + 5 * 60, [&now](FakeTesla &) { now.charge_now(); }}}) == 1);
  CHECK(messages(stuck, stopped, {{SEP24_1700Z + 5 * 60, [&stopped](FakeTesla &) { stopped.stop_charging(); }}}) == 1);
  CHECK(messages(stuck, away, {{ready_by - HOUR, &FakeTesla::unplug}}) == 1);

  // None before the car's battery level is known, or before the plug-in message has gone out.
  Controller unknown = with_prices();
  FakeTesla unread = plugged_in(false);
  unread.battery_known = false;
  unknown.tick(unread.state(ready_by - 30), Settings());
  CHECK(!unknown.tick(unread.state(ready_by), Settings()).notification);
  Controller late = with_prices();
  const Run plugged_late =
      simulate(late, stuck, ready_by - 120, ready_by + 10 * 60, {{ready_by - 60, &FakeTesla::plug_in}});
  for (const auto &[at, n] : plugged_late.messages)
    CHECK(n.message.rfind("Ready by passed", 0) != 0);

  // Moving Ready by later isn't passing it.
  Controller moved = with_prices();
  FakeTesla waiting = plugged_in(false);
  waiting.no_power = true;
  Settings settings;
  moved.tick(waiting.state(ready_by - 60), settings);
  settings.ready_by_minutes = 8 * 60;
  CHECK(!moved.tick(waiting.state(ready_by - 30), settings).notification);
  CHECK(!moved.tick(waiting.state(ready_by + 30), settings).notification);
  CHECK(moved.tick(waiting.state(ready_by + HOUR), settings).notification.has_value());

  // Nor does a start before the clock is set: the first deadline the board knows is still ahead.
  Controller booting = with_prices();
  booting.tick(waiting.state(0), Settings());
  CHECK(!booting.tick(waiting.state(SEP24_1700Z), Settings()).notification);
}

static void test_plug_in_message_without_prices() {
  Controller controller;
  const Run run =
      simulate(controller, FakeTesla(), SEP24_1700Z - 60, SEP24_1700Z + HOUR, {{SEP24_1700Z, &FakeTesla::plug_in}});
  REQUIRE(run.messages.size() == 1);
  CHECK_STR(run.messages[0].second.title, "ESPHome Tesla BLE Scheduler");
  CHECK_STR(run.messages[0].second.message, "Charging (no prices)");
}

static void test_plug_in_message_when_the_battery_level_stays_unknown() {
  Controller controller = with_prices();
  FakeTesla car;
  car.battery_known = false;
  const Run run = simulate(controller, car, SEP24_1700Z - 60, SEP24_1700Z + HOUR, {{SEP24_1700Z, &FakeTesla::plug_in}});
  REQUIRE(run.messages.size() == 1);
  CHECK(run.messages[0].first == SEP24_1700Z + 30 * 60);
  CHECK_STR(run.messages[0].second.message, "Charging (battery unknown)");

  Controller late = with_prices();  // after the 30 minutes, also between ticks
  late.tick(car.state(SEP24_1700Z - 30), Settings());
  car.plug_in();
  late.tick(car.state(SEP24_1700Z), Settings());
  CHECK(!late.tick(car.state(SEP24_1700Z + 30 * 60 - 1), Settings()).notification);
  CHECK(late.tick(car.state(SEP24_1700Z + 30 * 60 + 1), Settings()).notification.has_value());
}

// ---------------------------------------------------------------------------
// Savings
// ---------------------------------------------------------------------------

// A car plugged in at 40% with an 80% limit, reporting `state` and drawing `kw`.
static CarState drawing(int64_t now, float kw, const char *state = "Charging") {
  CarState car = plugged_in(false).state(now);
  car.charging_state = state;
  car.power_kw = kw;
  return car;
}

// A plug-in the board sees: the car unplugged a minute before `car`, unlike the plug state it first reads after a
// restart.
static Decision plug_in(Controller &controller, const CarState &car) {
  CarState away = car;
  away.now -= 60;
  away.plugged = false;
  away.charging_state = "Disconnected";
  controller.tick(away, Settings());
  return controller.tick(car, Settings());
}

// format_savings() with the same figures for the last 30 days and the last 365.
static std::string twice(const std::string &figures, const char *currency = "EUR") {
  return currency + (";" + figures + ";" + figures);
}

// Spot prices at `price` for the delivery days from that of `from` to that of `to`.
static PriceTable flat_prices(float price, int64_t from, int64_t to) {
  return prices_from(start_of_delivery_day(from), end_of_delivery_day(to), [price](int64_t) { return price; });
}

static const SavingsDay &savings_on(const Controller &controller, int year, unsigned month, unsigned day) {
  return controller.savings.days[ring_index(days_from_civil(year, month, day))];
}

static void test_savings_count_each_quarter_hour() {
  // At 36 kW, 0.01 kWh a second. Spot prices only: 0.50 EUR/kWh, but 0 in the quarter-hour before 20:00 and 1.00 in
  // the one from 20:00, so the day's average stays 0.50.
  Controller controller;
  controller.prices = prices_from(CET_SEP24, CET_SEP25, [](int64_t after) {
    const int64_t t = CET_SEP24 + after;
    return t == SEP24_1700Z - SLOT_SECONDS ? 0.0f : t == SEP24_1700Z ? 1.0f : 0.5f;
  });
  CHECK_STR(plug_in(controller, drawing(SEP24_1700Z - 45, 0, "Stopped")).savings, twice("0,0,0,0"));
  controller.tick(drawing(SEP24_1700Z - 15, 36), Settings());                     // 30 s at 0
  const Decision d = controller.tick(drawing(SEP24_1700Z + 15, 36), Settings());  // 15 s at 0, 15 s at 1.00
  // 0.6 kWh for 0.15 EUR, 0.30 at the day's average, and 0.15 at once: it charged from the start.
  CHECK_STR(d.savings, twice("600,15,30,15"));

  // Below zero, the car earns.
  Controller paid = with_prices(flat_prices(-0.5f, SEP24_1700Z, SEP24_1700Z));
  paid.tick(drawing(SEP24_1700Z, 0, "Stopped"), Settings());
  CHECK_STR(paid.tick(drawing(SEP24_1700Z + 10, 36), Settings()).savings, twice("100,-5,-5,-5"));

  // The ring wraps around for any day number.
  CHECK(ring_index(-1) == SAVINGS_DAYS - 1 && ring_index(SAVINGS_DAYS) == 0);
}

static void test_savings_by_local_day() {
  // Charging over midnight on New Year's Eve, in winter time: what it took before midnight counts on 31 December 2026,
  // the rest on 1 January 2027.
  const int64_t new_year = local_to_utc(days_from_civil(2027, 1, 1), 0, VILNIUS_STANDARD_OFFSET);
  Controller controller = with_prices(flat_prices(0.5f, new_year, new_year));
  controller.tick(drawing(new_year - 20, 0, "Stopped"), Settings());
  const Decision d = controller.tick(drawing(new_year + 10, 36), Settings());
  const SavingsDay &december = savings_on(controller, 2026, 12, 31), &january = savings_on(controller, 2027, 1, 1);
  CHECK(december.wh == 200 && december.paid == 10 && december.average == 10 && december.at_once == 10);
  CHECK(january.wh == 100 && january.paid == 5 && january.average == 5 && january.at_once == 5);
  CHECK_STR(d.savings, twice("300,15,15,15"));

  // The same in summer time, when midnight is 21:00 UTC.
  const int64_t friday = local_to_utc(days_from_civil(2026, 9, 25), 0, VILNIUS_STANDARD_OFFSET);
  Controller summer = with_prices(flat_prices(0.5f, friday, friday));
  summer.tick(drawing(friday - 20, 0, "Stopped"), Settings());
  CHECK_STR(summer.tick(drawing(friday + 10, 36), Settings()).savings, twice("300,15,15,15"));
  CHECK(savings_on(summer, 2026, 9, 24).wh == 200 && savings_on(summer, 2026, 9, 25).wh == 100);
}

static void test_savings_last_30_and_365_days() {
  // 0.1 kWh at 0.50 EUR/kWh on Thursday 24 September 2026, then the figures as the days go by.
  Controller controller = with_prices(flat_prices(0.5f, SEP24_1700Z, SEP24_1700Z));
  controller.tick(drawing(SEP24_1700Z, 0, "Stopped"), Settings());
  controller.tick(drawing(SEP24_1700Z + 10, 36), Settings());
  const auto days_later = [&controller](int64_t days) {
    return controller.tick(drawing(SEP24_1700Z + days * DAY_SECONDS, 0, "Stopped"), Settings()).savings;
  };
  const std::string start = "EUR", some = ";100,5,5,5", none = ";0,0,0,0";
  CHECK_STR(days_later(0), start + some + some);
  CHECK_STR(days_later(29), start + some + some);  // Friday 23 October, the 30th day
  CHECK_STR(days_later(30), start + none + some);
  CHECK_STR(days_later(364), start + none + some);  // Thursday 23 September 2027, the 365th
  CHECK_STR(days_later(365), start + none + none);
  CHECK_STR(days_later(365 + 1000), start + none + none);
}

static void test_savings_without_a_price() {
  // No prices at all, here over the end of a quarter-hour: the energy counts, and no money.
  Controller none;
  none.tick(drawing(SEP24_1700Z - 20, 0, "Stopped"), Settings());
  CHECK_STR(none.tick(drawing(SEP24_1700Z + 10, 36), Settings()).savings, twice("300,0,0,0"));
  // A quarter-hour without a price in a day with some counts at the day's average: it neither saves nor costs.
  Controller gap = with_prices(prices_from(CET_SEP24, SEP24_1700Z, [](int64_t) { return 0.5f; }));  // until 20:00
  gap.tick(drawing(SEP24_1700Z, 0, "Stopped"), Settings());
  CHECK_STR(gap.tick(drawing(SEP24_1700Z + 10, 36), Settings()).savings, twice("100,5,5,5"));
}

static void test_savings_average_of_the_delivery_day() {
  // Nord Pool's delivery day runs 01:00 to 01:00 in Vilnius. Thursday's: spot 0.10 EUR/kWh, and 1.00 in its last hour.
  // Friday's: 5.00. Saturday's: 9.00 from its second quarter-hour. None counts in another day's average. With 21% VAT
  // and two_zones()' fees by winter time, each day has 8 night hours at 0.07139 and 16 day hours at 0.12947.
  const int64_t saturday = CET_SEP25 + DAY_SECONDS;
  Controller controller;
  for (int64_t t = CET_SEP24; t < saturday + DAY_SECONDS; t += SLOT_SECONDS)
    if (t != saturday)
      controller.prices.set(t, t > saturday ? 9.0f : t >= CET_SEP25 ? 5.0f : t >= CET_SEP25 - HOUR ? 1.0f : 0.1f);
  controller.set_tariff(two_zones());
  plug_in(controller, drawing(SEP24_1700Z - 60, 0, "Stopped"));     // at the same price as 20:00
  for (int64_t eight : {SEP24_1700Z, SEP24_1700Z + DAY_SECONDS}) {  // 1 kWh at 20:00, in a day hour, each day
    controller.tick(drawing(eight, 0, "Stopped"), Settings());
    controller.tick(drawing(eight + 50, 36), Settings());
    controller.tick(drawing(eight + 100, 36), Settings());
  }
  // Thursday: 0.10 * 1.21 + 0.12947 = 0.25047 paid; the day's average (23 * 0.10 + 1.00) / 24 * 1.21 + (8 * 0.07139 +
  // 16 * 0.12947) / 24 = 0.16638 + 0.11011 = 0.27649. Friday: 5.00 * 1.21 + 0.12947 = 6.17947 paid, 6.05 + 0.11011 =
  // 6.16011 on average. At once both at Thursday's 20:00 price, as the car never got full.
  CHECK_STR(controller.tick(drawing(SEP24_1700Z + DAY_SECONDS + 130, 0, "Stopped"), Settings()).savings,
            twice("2000,643,644,50"));
}

// The figures of the last 365 days.
static SavingsDay savings_total(const Savings &savings) {
  SavingsDay all;
  for (const SavingsDay &day : savings.days)
    all.add(day);
  return all;
}

static void test_savings_after_a_night() {
  // Plugged in at 20:00 at 40% and charged to 80% in the night trough (test_charges_only_in_the_cheap_window): the 30
  // kWh the battery takes are 33.3 from the grid, plus half a minute of the car's own start at 190 EUR/MWh. The
  // trough averages 21.2 EUR/MWh, and both delivery days 94.48 (typical_baltic_price()). At once from 20:00, at 11 kW,
  // it would have charged two hours at 190 EUR/MWh and the rest at 95.
  Controller controller = with_prices();
  simulate(controller, FakeTesla(), SEP24_1700Z - HOUR, SEP24_1700Z + 12 * HOUR, {{SEP24_1700Z, &FakeTesla::plug_in}});
  const SavingsDay all = savings_total(controller.savings);
  const float kwh = static_cast<float>(all.wh) / 1000.0f, start_kwh = 11.0f * 30 / 3600;
  CHECK(near(kwh, 30.0f / EFFICIENCY + start_kwh, 0.1f));
  CHECK(near(static_cast<float>(all.paid) / 100.0f, (kwh - start_kwh) * 0.0212f + start_kwh * 0.19f, 0.01f));
  CHECK(near(static_cast<float>(all.average) / 100.0f, kwh * 0.09448f, 0.01f));
  CHECK(near(static_cast<float>(all.at_once) / 100.0f, 22 * 0.19f + (kwh - 22) * 0.095f, 0.01f));
}

static void test_savings_at_once_with_prices_out_later() {
  // Plugged in at 20:00 Thursday, then charged at 36 kW (9 kWh a quarter-hour) for 45 minutes from 03:00 Friday, at
  // 0.10 EUR/kWh. At once it would have charged 20:00 to 20:45: at 0.20, in a quarter-hour that never got a price
  // (so at Friday's average, 0.10), and at 0.40, a price out only after the plug-in.
  const int64_t eight = SEP24_1700Z, three = SEP24_1700Z + 7 * HOUR;
  Controller controller;
  for (int64_t t = CET_SEP24; t < CET_SEP25 + DAY_SECONDS; t += SLOT_SECONDS)
    if (t != eight + SLOT_SECONDS && t != eight + 2 * SLOT_SECONDS)
      controller.prices.set(t, t < CET_SEP25 ? 0.2f : 0.1f);
  plug_in(controller, drawing(eight, 0, "Stopped"));
  controller.prices.set(eight + 2 * SLOT_SECONDS, 0.4f);
  controller.tick(drawing(eight + 20 * 60, 0, "Stopped"), Settings());
  controller.prices.set(eight, 0.9f);  // a price it has is kept
  controller.tick(drawing(three, 0, "Stopped"), Settings());
  for (int64_t t = three + 30; t <= three + 45 * 60; t += 30)
    controller.tick(drawing(t, 36), Settings());
  // 27 kWh for 2.70 EUR, the same at Friday's average; at once 9 * 0.20 + 9 * 0.10 + 9 * 0.40 = 6.30.
  CHECK_STR(controller.tick(drawing(three + 46 * 60, 0, "Stopped"), Settings()).savings, twice("27000,270,270,630"));
}

static void test_savings_at_once_for_a_day_at_most() {
  // Plugged in at the start of Thursday's delivery day, at 1.00 EUR/kWh, then charged at 2 kW for a day and half an
  // hour from the start of Friday's, at 0.10, into Saturday's, at 0.30. At once covers a day of charging; the rest
  // counts at its day's average.
  const int64_t saturday = CET_SEP25 + DAY_SECONDS;
  Controller controller = with_prices(prices_from(CET_SEP24, saturday + DAY_SECONDS, [](int64_t after) {
    return after < DAY_SECONDS ? 1.0f : after < 2 * DAY_SECONDS ? 0.1f : 0.3f;
  }));
  plug_in(controller, drawing(CET_SEP24, 0, "Stopped"));
  controller.tick(drawing(CET_SEP25, 0, "Stopped"), Settings());
  for (int64_t t = CET_SEP25 + 60; t <= saturday + 30 * 60; t += 60)
    controller.tick(drawing(t, 2), Settings());
  // 49 kWh: 48 * 0.10 + 1 * 0.30 = 5.10 paid and at the days' averages; at once 48 * 1.00 + 1 * 0.30 = 48.30.
  const SavingsDay all = savings_total(controller.savings);
  CHECK(near(static_cast<float>(all.wh), 49000, 2));
  CHECK(near(static_cast<float>(all.paid), 510, 1.5f));
  CHECK(near(static_cast<float>(all.average), 510, 1.5f));
  CHECK(near(static_cast<float>(all.at_once), 4830, 1.5f));
}

static void test_savings_at_once_after_a_restart() {
  // After a restart with the car plugged in, when it was plugged in is unknown: charging at once counts at the day's
  // average until the car is plugged in again. 0.10 EUR/kWh, but 0.30 from 20:00 and 0.50 from 20:30 for a quarter-hour
  // each: the day's average is (94 * 0.10 + 0.30 + 0.50) / 96 = 0.10625.
  Controller controller = with_prices(prices_from(CET_SEP24, CET_SEP25, [](int64_t after) {
    const int64_t t = CET_SEP24 + after;
    return t == SEP24_1700Z ? 0.3f : t == SEP24_1700Z + 2 * SLOT_SECONDS ? 0.5f : 0.1f;
  }));
  CarState unknown = drawing(SEP24_1700Z - 60, 0, "");  // the car hasn't reported since the restart
  unknown.plugged.reset();
  controller.tick(unknown, Settings());
  controller.tick(drawing(SEP24_1700Z, 0, "Stopped"), Settings());
  for (int64_t t = SEP24_1700Z + 30; t <= SEP24_1700Z + SLOT_SECONDS; t += 30)
    controller.tick(drawing(t, 36), Settings());
  // 9 kWh from 20:00 for 2.70 EUR, and 0.96 at the day's average, also at once.
  const SavingsDay &thursday = savings_on(controller, 2026, 9, 24);
  CHECK(thursday.paid == 270 && thursday.average == 96 && thursday.at_once == 96);
  // Plugged in again at 20:30, then 9 kWh from 20:45 for 0.90, 0.96 at the day's average, and at once from 20:30 4.50.
  plug_in(controller, drawing(SEP24_1700Z + 2 * SLOT_SECONDS, 0, "Stopped"));
  controller.tick(drawing(SEP24_1700Z + 3 * SLOT_SECONDS, 0, "Stopped"), Settings());
  for (int64_t t = SEP24_1700Z + 3 * SLOT_SECONDS + 30; t <= SEP24_1700Z + 4 * SLOT_SECONDS; t += 30)
    controller.tick(drawing(t, 36), Settings());
  CHECK(thursday.paid == 360 && thursday.average == 191 && thursday.at_once == 546);
}

static void test_savings_at_once_starts_when_charging_is_needed() {
  // Plugged in at 20:00 and charged at once for a quarter-hour at 0.30 EUR/kWh, to the limit. By 06:00 Friday the car
  // has drifted below it, at 0.20; the board charges at 07:00, at 0.10. At once would have charged from 06:00.
  const int64_t eight = SEP24_1700Z, six = SEP24_1700Z + 10 * HOUR, seven = six + HOUR;
  Controller controller = with_prices(prices_from(CET_SEP24, CET_SEP25 + DAY_SECONDS, [=](int64_t after) {
    const int64_t t = CET_SEP24 + after;
    return t == eight ? 0.3f : t == six ? 0.2f : 0.1f;
  }));
  const auto at = [](int64_t now, float soc, float kw, const char *state) {
    CarState car = drawing(now, kw, state);
    car.soc = soc;
    return car;
  };
  plug_in(controller, at(eight, 79, 0, "Stopped"));
  for (int64_t t = eight + 30; t <= eight + 15 * 60; t += 30)
    controller.tick(at(t, 79, 36, "Charging"), Settings());
  controller.tick(at(eight + 16 * 60, 80, 0, "Complete"), Settings());
  controller.tick(at(six, 79, 0, "Stopped"), Settings());
  controller.tick(at(seven, 79, 0, "Stopped"), Settings());
  for (int64_t t = seven + 30; t <= seven + 15 * 60; t += 30)
    controller.tick(at(t, 79, 36, "Charging"), Settings());
  // 9 kWh each day: 2.70 EUR on Thursday, paid and at once; 0.90 on Friday, and 1.80 at once.
  const SavingsDay &thursday = savings_on(controller, 2026, 9, 24), &friday = savings_on(controller, 2026, 9, 25);
  CHECK(thursday.wh == 9000 && thursday.paid == 270 && thursday.at_once == 270);
  CHECK(friday.wh == 9000 && friday.paid == 90 && friday.at_once == 180);
}

static void test_savings_count_only_what_the_car_draws() {
  // 0.10 EUR/kWh from 20:00, 0.30 from 20:15. Plugged in at 20:00.
  Controller controller = with_prices(prices_from(CET_SEP24, CET_SEP25, [](int64_t after) {
    return CET_SEP24 + after < SEP24_1700Z + SLOT_SECONDS ? 0.1f : 0.3f;
  }));
  plug_in(controller, drawing(SEP24_1700Z, 0, "Stopped"));
  // Nothing while the car isn't reported charging, draws nothing or its power is unknown, nor does charging at once
  // move on.
  controller.tick(drawing(SEP24_1700Z + 30, 11, "Starting"), Settings());
  controller.tick(drawing(SEP24_1700Z + 60, 11, "Unknown"), Settings());
  controller.tick(drawing(SEP24_1700Z + 90, 11, ""), Settings());
  controller.tick(drawing(SEP24_1700Z + 120, 0), Settings());
  controller.tick(drawing(SEP24_1700Z + 150, NAN), Settings());
  CHECK(savings_on(controller, 2026, 9, 24).wh == 0);
  // 20:15 to 20:30 at 36 kW: 9 kWh for 2.70 EUR, at once in the first quarter-hour for 0.90.
  controller.tick(drawing(SEP24_1700Z + SLOT_SECONDS, 0, "Stopped"), Settings());
  for (int64_t t = SEP24_1700Z + SLOT_SECONDS + 30; t <= SEP24_1700Z + 2 * SLOT_SECONDS; t += 30)
    controller.tick(drawing(t, 36), Settings());
  const SavingsDay &thursday = savings_on(controller, 2026, 9, 24);
  CHECK(thursday.wh == 9000 && thursday.paid == 270 && thursday.at_once == 90);
  // A tick ten minutes late counts the last minute: the car's power is from the last few seconds.
  controller.tick(drawing(SEP24_1700Z + 2 * SLOT_SECONDS + 10 * 60, 36), Settings());
  CHECK(thursday.wh == 9600 && thursday.paid == 288);
  // The car reports whole kW: 1 kW counts.
  controller.tick(drawing(SEP24_1700Z + 2 * SLOT_SECONDS + 10 * 60 + 36, 1), Settings());
  CHECK(thursday.wh == 9610);
}

static void test_savings_with_a_clock_that_goes_back() {
  // Thursday's and Friday's prices at 0.50 EUR/kWh; 0.1 kWh on Friday, then the clock goes back a day.
  const int64_t friday = SEP24_1700Z + DAY_SECONDS;
  Controller controller = with_prices(flat_prices(0.5f, SEP24_1700Z, friday));
  controller.tick(drawing(friday, 0, "Stopped"), Settings());
  controller.tick(drawing(friday + 10, 36), Settings());
  // Nothing counts for the time before the last tick, the figures are written, and the days stay as they were. Then
  // 0.1 kWh on Thursday, whose prices are gone: no money.
  CHECK(controller.tick(drawing(SEP24_1700Z + 20, 36), Settings()).save_savings);
  CHECK_STR(controller.tick(drawing(SEP24_1700Z + 30, 36), Settings()).savings, twice("200,5,5,5"));
  CHECK(savings_on(controller, 2026, 9, 24).wh == 100 && savings_on(controller, 2026, 9, 25).wh == 100);
}

static void test_savings_saved_once_a_quarter_hour() {
  // Written at once when counting starts, then while the car charges on its first tick in each quarter-hour, and
  // once it stops.
  Controller controller = with_prices();
  CHECK(controller.tick(drawing(SEP24_1700Z, 0, "Stopped"), Settings()).save_savings);
  CHECK(!controller.tick(drawing(SEP24_1700Z + 30, 0, "Stopped"), Settings()).save_savings);
  CHECK(!controller.tick(drawing(SEP24_1700Z + 60, 11), Settings()).save_savings);  // the quarter-hour just written
  CHECK(!controller.tick(drawing(SEP24_1700Z + 90, 11), Settings()).save_savings);
  CHECK(controller.tick(drawing(SEP24_1700Z + SLOT_SECONDS, 11), Settings()).save_savings);
  CHECK(!controller.tick(drawing(SEP24_1700Z + SLOT_SECONDS + 30, 11), Settings()).save_savings);
  CHECK(controller.tick(drawing(SEP24_1700Z + SLOT_SECONDS + 60, 0, "Complete"), Settings()).save_savings);
  CHECK(!controller.tick(drawing(SEP24_1700Z + 2 * SLOT_SECONDS, 0, "Complete"), Settings()).save_savings);
}

static void test_savings_restart_currency_and_reset() {
  // Nothing before the clock is set.
  Controller controller = with_prices(flat_prices(0.5f, SEP24_1700Z, SEP24_1700Z + DAY_SECONDS));
  CHECK(controller.tick(drawing(0, 0, "Stopped"), Settings()).savings.empty());
  controller.tick(drawing(SEP24_1700Z, 0, "Stopped"), Settings());
  controller.tick(drawing(SEP24_1700Z + 10, 36), Settings());

  // After a restart the board loads what it saved and adds to it.
  Controller restarted = with_prices(controller.prices);
  restarted.savings = controller.savings;
  const Decision loaded = restarted.tick(drawing(SEP24_1700Z + 60, 0, "Stopped"), Settings());
  CHECK_STR(loaded.savings, twice("100,5,5,5"));
  CHECK(!loaded.save_savings);
  CHECK_STR(restarted.tick(drawing(SEP24_1700Z + 70, 36), Settings()).savings, twice("200,10,10,10"));

  // Another currency starts afresh.
  Settings sek;
  sek.currency = "SEK";
  const int64_t friday = SEP24_1700Z + DAY_SECONDS;
  const Decision swedish = restarted.tick(drawing(friday, 0, "Stopped"), sek);
  CHECK_STR(swedish.savings, twice("0,0,0,0", "SEK"));
  CHECK(swedish.save_savings);
  CHECK_STR(restarted.tick(drawing(friday + 10, 36), sek).savings, twice("100,5,5,5", "SEK"));

  // So does Reset savings, on the next tick, and what the car takes after it counts alone; also the days before.
  restarted.reset_savings();
  const Decision reset = restarted.tick(drawing(friday + 20, 0, "Stopped"), sek);
  CHECK_STR(reset.savings, twice("0,0,0,0", "SEK"));
  CHECK(reset.save_savings);
  CHECK_STR(restarted.tick(drawing(friday + 30, 36), sek).savings, twice("100,5,5,5", "SEK"));
  restarted.reset_savings();
  CHECK_STR(restarted.tick(drawing(friday + DAY_SECONDS, 0, "Stopped"), sek).savings, twice("0,0,0,0", "SEK"));
  // And back to euros.
  const Decision euros = restarted.tick(drawing(friday + DAY_SECONDS + 10, 0, "Stopped"), Settings());
  CHECK_STR(euros.savings, twice("0,0,0,0"));
  CHECK(euros.save_savings);

  // Large figures load without losing a cent.
  Controller loaded_large = with_prices(flat_prices(0.5f, SEP24_1700Z, SEP24_1700Z));
  std::snprintf(loaded_large.savings.currency, sizeof(loaded_large.savings.currency), "EUR");
  loaded_large.savings.day = static_cast<int32_t>(days_from_civil(2026, 9, 24));
  loaded_large.savings.days[ring_index(loaded_large.savings.day)] = {100000, 5000, 6000, 7000};
  loaded_large.tick(drawing(SEP24_1700Z, 0, "Stopped"), Settings());
  CHECK_STR(loaded_large.tick(drawing(SEP24_1700Z + 10, 36), Settings()).savings, twice("100100,5005,6005,7005"));
}

// NOLINTNEXTLINE(bugprone-exception-escape): an exception ends the run, failing it as it should
int main() {
  test_calendar();
  test_nord_pool_prices();
  test_smard_prices();
  test_ecb_rates();
  test_omie_prices();
  test_cheapest_slots();
  test_schedule_picks_the_night_trough();
  test_schedule_waits_for_prices_not_out_yet();
  test_schedule_windows_and_prices();
  test_reads_tariff_settings();
  test_plan_downloads();
  test_makes_tariffs();
  test_eso_plans();
  test_calendar_and_exceptions();
  test_plans_in_the_repository();
  test_reads_the_settings_file();
  test_settings_file_errors();
  test_settings_form_options();
  test_schedule_counts_the_tariff_fee();
  test_updates_wait_while_charging();
  test_charges_only_in_the_cheap_window();
  test_start_from_the_car_holds_until_unplugged();
  test_reads_the_charging_state();
  test_tells_its_own_starts_from_the_cars();
  test_complete_under_the_limit_is_charged();
  test_reschedules_when_no_longer_complete();
  test_reschedules_when_charging_runs_slow();
  test_no_start_for_a_windows_last_minutes();
  test_charges_as_usual_without_prices();
  test_waits_for_tomorrows_prices();
  test_fetch_prices_due();
  test_grid_fees_alone();
  test_restart_waits_for_prices();
  test_wakes_for_battery_level_then_charges();
  test_wakes_to_learn_the_plug_state();
  test_charge_now_ignores_the_schedule();
  test_full_within_half_a_percent();
  test_retries_are_rate_limited();
  test_charger_without_power();
  test_new_limit_reschedules_at_once();
  test_one_off_ready_by();
  test_schedules_with_the_grid_fees();
  test_windows_while_the_schedule_decides();
  test_stop_charging_until_a_button_or_plug_in();
  test_buttons_skip_the_command_limits();
  test_buttons_hold_across_a_restart();
  test_create_schedule_cancels_charge_now();
  test_plug_in_message_after_two_minutes();
  test_plug_in_message_leaves_out_the_spare();
  test_plug_in_in_cet();
  test_plug_in_message_when_time_is_short();
  test_plug_in_message_at_the_limit();
  test_plug_in_message_waits_for_the_last_prices();
  test_plug_in_message_when_charging_now();
  test_no_plug_in_message_after_stop_charging();
  test_no_plug_in_message_after_a_restart();
  test_one_plug_in_message_per_plug_in();
  test_fallback_message_after_a_restart();
  test_fallback_message_mid_stay();
  test_message_when_ready_by_passes_short();
  test_plug_in_message_without_prices();
  test_plug_in_message_when_the_battery_level_stays_unknown();
  test_savings_count_each_quarter_hour();
  test_savings_by_local_day();
  test_savings_last_30_and_365_days();
  test_savings_without_a_price();
  test_savings_average_of_the_delivery_day();
  test_savings_after_a_night();
  test_savings_at_once_with_prices_out_later();
  test_savings_at_once_for_a_day_at_most();
  test_savings_at_once_after_a_restart();
  test_savings_at_once_starts_when_charging_is_needed();
  test_savings_count_only_what_the_car_draws();
  test_savings_with_a_clock_that_goes_back();
  test_savings_saved_once_a_quarter_hour();
  test_savings_restart_currency_and_reset();
  if (failures == 0)
    std::printf("All tests passed\n");
  return failures == 0 ? 0 : 1;
}
