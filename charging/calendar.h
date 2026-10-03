#pragma once
// Dates and times without libc's time zones: the proleptic Gregorian calendar in UTC, EU summer time, and local
// times as the page and the phone messages show them. Plain C++17, with nothing from ESPHome, like charging.h.

#include <cstdint>
#include <cstdio>
#include <string>

namespace esphome::charging {

// A slot is one quarter-hour, Nord Pool's price period.
constexpr int64_t SLOT_SECONDS = 15 * 60;
constexpr int64_t DAY_SECONDS = 24 * 3600;
// The default winter offset from UTC; the YAML sets the board's own from its clock's time zone.
constexpr int32_t VILNIUS_STANDARD_OFFSET = 2 * 3600;


// a / b rounded down, for b > 0.
inline int64_t floor_div(int64_t a, int64_t b) { return a / b - (a % b < 0); }

// 0 = Sunday; day 0, 1970-01-01, was a Thursday.
inline int weekday(int64_t day) { return static_cast<int>((day % 7 + 11) % 7); }

inline int64_t floor_to_slot(int64_t t) { return floor_div(t, SLOT_SECONDS) * SLOT_SECONDS; }

// Howard Hinnant's days_from_civil / civil_from_days.
inline int64_t days_from_civil(int64_t y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int64_t era = floor_div(y, 400);
  const auto yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + static_cast<int64_t>(doe) - 719468;
}

struct CivilDate {
  int64_t year;
  unsigned month;
  unsigned day;
};

inline CivilDate civil_from_days(int64_t z) {
  z += 719468;
  const int64_t era = floor_div(z, 146097);
  const auto doe = static_cast<unsigned>(z - era * 146097);
  const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const unsigned mp = (5 * doy + 2) / 153;
  const unsigned d = doy - (153 * mp + 2) / 5 + 1;
  const unsigned m = mp < 10 ? mp + 3 : mp - 9;
  return {static_cast<int64_t>(yoe) + era * 400 + (m <= 2), m, d};
}

// Day number (days since 1970-01-01) of the last Sunday of a 31-day month.
inline int64_t last_sunday_of(int64_t year, unsigned month) {
  const int64_t last_day = days_from_civil(year, month, 31);
  return last_day - weekday(last_day);
}

// EU summer time: last Sunday of March 01:00 UTC to last Sunday of October 01:00 UTC.
inline bool eu_summer_time(int64_t utc) {
  const int64_t year = civil_from_days(floor_div(utc, DAY_SECONDS)).year;
  const int64_t start = last_sunday_of(year, 3) * DAY_SECONDS + 3600;
  const int64_t end = last_sunday_of(year, 10) * DAY_SECONDS + 3600;
  return utc >= start && utc < end;
}

inline int32_t eu_offset(int64_t utc, int32_t standard_offset) {
  return standard_offset + (eu_summer_time(utc) ? 3600 : 0);
}

// The local calendar day (days since 1970-01-01) of a moment.
inline int64_t local_day_of(int64_t utc, int32_t standard_offset) {
  return floor_div(utc + eu_offset(utc, standard_offset), DAY_SECONDS);
}

inline std::string format_hhmm(int64_t utc, int32_t standard_offset) {
  const int64_t local = utc + eu_offset(utc, standard_offset);
  const int64_t minute_of_day = floor_div(local, 60) - floor_div(local, DAY_SECONDS) * 1440;
  char buf[8];
  std::snprintf(buf, sizeof(buf), "%02d:%02d", static_cast<int>(minute_of_day / 60),
                static_cast<int>(minute_of_day % 60));
  return buf;
}

// The moment (UTC) that is `minutes` after local midnight on local calendar day `local_day`. A time the clocks
// skip in spring moves an hour on; one they repeat in autumn is the first.
inline int64_t local_to_utc(int64_t local_day, int minutes, int32_t standard_offset) {
  const int64_t local = local_day * DAY_SECONDS + static_cast<int64_t>(minutes) * 60;
  return local - eu_offset(local - standard_offset - 3600, standard_offset);
}

// The next moment (UTC) that is `minutes` after local midnight, strictly after `now`.
inline int64_t next_local_time(int64_t now, int minutes, int32_t standard_offset) {
  const int64_t today = local_day_of(now, standard_offset);
  const int64_t utc = local_to_utc(today, minutes, standard_offset);
  return utc > now ? utc : local_to_utc(today + 1, minutes, standard_offset);
}

// "Mon 00:00" in local time.
inline std::string format_day_hhmm(int64_t utc, int32_t standard_offset) {
  static constexpr const char *WEEKDAYS[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  return std::string(WEEKDAYS[weekday(local_day_of(utc, standard_offset))]) + " " + format_hhmm(utc, standard_offset);
}

// "23:00" for a moment less than 24 clock hours after `now` (the next 23:00 is unambiguous), else "Mon 00:00".
inline std::string format_when(int64_t utc, int64_t now, int32_t standard_offset) {
  return utc + eu_offset(utc, standard_offset) < now + eu_offset(now, standard_offset) + DAY_SECONDS
             ? format_hhmm(utc, standard_offset)
             : format_day_hhmm(utc, standard_offset);
}

}  // namespace esphome::charging
