"""Charges a Tesla in the cheapest Nord Pool quarter-hours before Ready by.

charging.h plans and decides; charging_component.h connects it to ESPHome. This file checks the settings and the
grid fees (format in docs/grid-fees.md) when you build, and creates the web page's entities.
"""

import re
from string import ascii_lowercase

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, datetime, text_sensor, time
from esphome.components.http_request import CONF_HTTP_REQUEST_ID, HttpRequestComponent
from esphome.const import CONF_AREA, CONF_DISABLED_BY_DEFAULT, CONF_HOURS, CONF_ID, CONF_NAME, CONF_TIME_ID

DEPENDENCIES = ["http_request", "network", "time"]
AUTO_LOAD = ["button", "datetime", "json", "text_sensor"]

CONF_BATTERY_KWH = "battery_kwh"
CONF_CHARGING_KW = "charging_kw"
CONF_MARKET = "market"
CONF_PRICES = "prices"
CONF_VAT = "vat"
CONF_CURRENCY = "currency"
CONF_NTFY_SERVER = "ntfy_server"
CONF_NTFY_TOPIC = "ntfy_topic"
CONF_VIN = "vin"
CONF_GRID = "grid"
CONF_HOLIDAYS = "holidays"
CONF_CLOCK = "clock"
CONF_HOLIDAY = "holiday"
CONF_SEASONS = "seasons"
CONF_FROM = "from"
CONF_TO = "to"

# Where electricity is bought: a country's code, or the price area where a country has several. DE and LU are the
# area Germany and Luxembourg share. Nord Pool names an area as here unless NORD_POOL_AREAS says otherwise, and has
# its prices in NORD_POOL_CURRENCIES, in euros unless CURRENCIES has the country.
AREAS = [
    "AT",
    "BE",
    "BG",
    "DE",
    "DK1",
    "DK2",
    "EE",
    "FI",
    "FR",
    "HR",
    "LT",
    "LU",
    "LV",
    "NL",
    "NO1",
    "NO2",
    "NO3",
    "NO4",
    "NO5",
    "PL",
    "RO",
    "SE1",
    "SE2",
    "SE3",
    "SE4",
]
NORD_POOL_AREAS = {"DE": "GER", "LU": "GER", "RO": "TEL"}
NORD_POOL_CURRENCIES = ["DKK", "EUR", "NOK", "PLN", "RON", "SEK"]
CURRENCIES = {"DK": "DKK", "NO": "NOK", "PL": "PLN", "RO": "RON", "SE": "SEK"}

charging_ns = cg.esphome_ns.namespace("charging")
ChargingComponent = charging_ns.class_("ChargingComponent", cg.PollingComponent)
ReadyBy = charging_ns.class_("ReadyBy", datetime.TimeEntity)
ReadyByOnce = charging_ns.class_("ReadyByOnce", datetime.DateTimeEntity)
ActionButton = charging_ns.class_("ActionButton", button.Button)
Action = charging_ns.enum("Action", is_class=True)
Grid = charging_ns.struct("Grid")
Season = charging_ns.struct("Season")

# The keys of hours: days, ranges of days in the week from Monday, like mon-fri, and holiday. charging.h numbers the
# days from Sunday, 0, with public holidays after Saturday, at 7.
WEEK = ["mon", "tue", "wed", "thu", "fri", "sat", "sun"]
DAY_INDEX = {"sun": 0, "mon": 1, "tue": 2, "wed": 3, "thu": 4, "fri": 5, "sat": 6, CONF_HOLIDAY: 7}
MONTH_DAYS = (31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31)


def _zones(value):
    value = cv.string_strict(value)
    if not re.fullmatch(r"[a-z]{24}|[a-z]{48}|[a-z]{96}", value):
        raise cv.Invalid(
            "must be 24, 48 or 96 lowercase letters: the zone of each hour, half-hour or quarter-hour from 00:00"
        )
    return value


def _days(key):
    """The days a key of hours covers, as charging.h numbers them: a day (mon), a range (mon-fri) or holiday. None for
    anything else."""
    if key in DAY_INDEX:
        return [DAY_INDEX[key]]
    first, _, last = key.partition("-")
    if first in WEEK and last in WEEK and WEEK.index(first) < WEEK.index(last):
        return [DAY_INDEX[day] for day in WEEK[WEEK.index(first) : WEEK.index(last) + 1]]
    return None


def _hours(every_day):
    """The zones of the days the keys cover, each day once; all year, every day of the week."""

    def validate(config):
        config = cv.Schema({cv.string_strict: _zones})(config)
        if not config:
            raise cv.Invalid("needs the zones of a day")
        names = {index: name for name, index in DAY_INDEX.items()}
        covered = {}
        for key in config:
            days = _days(key)
            if days is None:
                raise cv.Invalid(f'"{key}" isn\'t a day like mon, a range like mon-fri, or holiday', [key])
            for day in days:
                if day in covered:
                    raise cv.Invalid(f'{names[day]} is in "{covered[day]}" already', [key])
                covered[day] = key
        if every_day and (missing := [day for day in WEEK if DAY_INDEX[day] not in covered]):
            raise cv.Invalid(f"needs the zones of {', '.join(missing)} too")
        return config

    return validate


def _month_day(value):
    """Parses "12-25" to 1225 (month * 100 + day). "02-29" passes: it exists in leap years."""
    value = cv.string_strict(value)
    m = re.fullmatch(r"(\d\d)-(\d\d)", value)
    month, day = (int(m[1]), int(m[2])) if m else (0, 0)
    if 1 <= month <= 12 and 1 <= day <= MONTH_DAYS[month - 1]:
        return month * 100 + day
    raise cv.Invalid(f'"{value}" isn\'t a date such as "12-25"')


def _holiday(value):
    """A public holiday: a date as _month_day(), or "easter", "easter+1" or "easter-2" as ("easter", days after
    Easter Sunday)."""
    value = cv.string_strict(value)
    if m := re.fullmatch(r"easter([+-]\d{1,2})?", value):
        return ("easter", int(m[1] or 0))
    if re.fullmatch(r"\d\d-\d\d", value):
        return _month_day(value)
    raise cv.Invalid(f'"{value}" isn\'t a date such as "12-25", "easter" or "easter+1"')


def _vin(value):
    """The car's VIN, checked here because the Tesla component takes any string and a wrong one leaves the board
    waiting for the car forever. Lower case is wrong too: the Bluetooth name is made from the text as typed."""
    value = cv.string_strict(value)
    if not re.fullmatch(r"[A-HJ-NPR-Z0-9]{17}", value):
        raise cv.Invalid(
            "must be the car's VIN: 17 capital letters and digits, none of them I, O or Q, "
            "on the car's screen under Controls, Software"
        )
    return value


def _currency_code(value):
    value = cv.string_strict(value).upper()
    if not re.fullmatch(r"[A-Z]{3}", value):
        raise cv.Invalid("must be a currency's three-letter code, like EUR")
    return value


def _currency(config):
    """The market area's currency unless set, otherwise euros. Without a market, any currency."""
    market = config.get(CONF_MARKET)
    area = market[CONF_AREA] if market else ""
    config.setdefault(CONF_CURRENCY, CURRENCIES.get(area[:2], "EUR"))
    if market and config[CONF_CURRENCY] not in NORD_POOL_CURRENCIES:
        raise cv.Invalid(f"Nord Pool's prices come in {', '.join(NORD_POOL_CURRENCIES)}", [CONF_CURRENCY])
    return config


def _season_dates(season):
    """The dates of a season, as month * 100 + day."""
    first, last = season[CONF_FROM], season[CONF_TO]
    dates = {month * 100 + day for month in range(1, 13) for day in range(1, MONTH_DAYS[month - 1] + 1)}
    return {date for date in dates if (first <= date <= last if first <= last else date >= first or date <= last)}


def _validate_grid(config):
    seasons = config[CONF_SEASONS]
    for i, season in enumerate(seasons):
        for other in seasons[:i]:
            if _season_dates(season) & _season_dates(other):
                start = f"{other[CONF_FROM] // 100:02d}-{other[CONF_FROM] % 100:02d}"
                raise cv.Invalid(f"overlaps the season from {start}", [CONF_SEASONS, i])
    tables = [config[CONF_HOURS], *(season[CONF_HOURS] for season in seasons)]
    zones = set("".join(zones for table in tables for zones in table.values()))
    if missing := sorted(zones - set(config[CONF_PRICES])):
        raise cv.Invalid(f'zone "{missing[0]}" needs a price per kWh incl. VAT', path=[CONF_PRICES])
    if unused := sorted(set(config[CONF_PRICES]) - zones):
        raise cv.Invalid(f'zone "{unused[0]}" isn\'t used in any hour', path=[CONF_PRICES, unused[0]])
    return config


GRID_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Optional(CONF_CLOCK, default="local"): cv.one_of("local", "winter"),
            cv.Optional(CONF_HOLIDAYS, default=[]): cv.ensure_list(_holiday),
            cv.Required(CONF_HOURS): _hours(every_day=True),
            cv.Required(CONF_PRICES): cv.Schema({cv.string_strict: cv.float_range(min=0.0)}),
            cv.Optional(CONF_SEASONS, default=[]): cv.ensure_list(
                cv.Schema(
                    {
                        cv.Required(CONF_FROM): _month_day,
                        cv.Required(CONF_TO): _month_day,
                        cv.Required(CONF_HOURS): _hours(every_day=False),
                    }
                )
            ),
        }
    ),
    _validate_grid,
)

MARKET_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_AREA): cv.one_of(*AREAS, upper=True),
        cv.Required(CONF_VAT): cv.float_range(min=0.0, max=1.0, max_included=False),
    }
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ChargingComponent),
            cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
            cv.GenerateID(CONF_HTTP_REQUEST_ID): cv.use_id(HttpRequestComponent),
            cv.Required(CONF_BATTERY_KWH): cv.positive_not_null_float,
            cv.Required(CONF_CHARGING_KW): cv.positive_not_null_float,
            cv.Optional(CONF_CURRENCY): _currency_code,
            cv.Optional(CONF_MARKET): MARKET_SCHEMA,
            cv.Optional(CONF_NTFY_SERVER, default="https://ntfy.sh"): cv.url,
            cv.Optional(CONF_NTFY_TOPIC, default=""): cv.string,
            cv.Required(CONF_VIN): _vin,
            cv.Required(CONF_GRID): GRID_SCHEMA,
        }
    ).extend(cv.polling_component_schema("30s")),
    _currency,
)


def _table(hours):
    """A Hours of charging.h: the zones by day from Sunday, then holidays, empty where hours doesn't say."""
    table = [""] * 8
    for key, zones in hours.items():
        for day in _days(key):
            table[day] = zones
    return table


def _grid(config, vat):
    holidays = config[CONF_HOLIDAYS]
    return cg.StructInitializer(
        Grid,
        ("vat", vat),
        ("winter_clock", config[CONF_CLOCK] == "winter"),
        ("hours", _table(config[CONF_HOURS])),
        (
            "seasons",
            [
                cg.StructInitializer(
                    Season, ("from", season[CONF_FROM]), ("to", season[CONF_TO]), ("hours", _table(season[CONF_HOURS]))
                )
                for season in config[CONF_SEASONS]
            ],
        ),
        ("fee", [config[CONF_PRICES].get(zone, 0.0) for zone in ascii_lowercase]),
        ("holidays", [h for h in holidays if not isinstance(h, tuple)]),
        ("after_easter", [h[1] for h in holidays if isinstance(h, tuple)]),
    )


# The web page finds these entities by name, and the board keeps Ready by and Ready by once under theirs, so the
# names are fixed.
def _entity(cls, key, name, **extra):
    return {CONF_ID: cv.declare_id(cls)(f"charging_{key}"), CONF_NAME: name, CONF_DISABLED_BY_DEFAULT: False, **extra}


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_clock(await cg.get_variable(config[CONF_TIME_ID])))
    cg.add(var.set_http(await cg.get_variable(config[CONF_HTTP_REQUEST_ID])))
    market = config.get(CONF_MARKET)
    if market:
        cg.add(var.set_nord_pool_area(NORD_POOL_AREAS.get(market[CONF_AREA], market[CONF_AREA])))
    cg.add(var.set_currency(config[CONF_CURRENCY]))
    cg.add(var.set_battery_kwh(config[CONF_BATTERY_KWH]))
    cg.add(var.set_charging_kw(config[CONF_CHARGING_KW]))
    cg.add(var.set_ntfy(config[CONF_NTFY_SERVER], config[CONF_NTFY_TOPIC]))
    cg.add(var.set_grid(_grid(config[CONF_GRID], market[CONF_VAT] if market else 0.0)))

    ready_by = await datetime.new_datetime(_entity(ReadyBy, "ready_by", "Ready by", type="TIME"))
    await cg.register_parented(ready_by, var)
    cg.add(var.set_ready_by(ready_by))
    ready_by_once = await datetime.new_datetime(_entity(ReadyByOnce, "ready_by_once", "Ready by once", type="DATETIME"))
    await cg.register_parented(ready_by_once, var)
    cg.add(var.set_ready_by_once(ready_by_once))

    for key, name, action in (
        ("create_plan", "Create charging plan", Action.CREATE_PLAN),
        ("charge_now", "Start charging now", Action.CHARGE_NOW),
        ("stop_charging", "Stop charging", Action.STOP_CHARGING),
        ("reset_savings", "Reset savings", Action.RESET_SAVINGS),
    ):
        action_button = await button.new_button(_entity(ActionButton, key, name))
        await cg.register_parented(action_button, var)
        cg.add(action_button.set_action(action))

    for key, name, setter in (
        ("status", "Charging status", var.set_status),
        ("mode", "Charging mode", var.set_mode),
        ("windows", "Charge windows", var.set_windows),
        ("prices_until", "Prices until", var.set_prices_until),
        ("savings", "Savings", var.set_savings),
    ):
        cg.add(setter(await text_sensor.new_text_sensor(_entity(text_sensor.TextSensor, key, name))))
