"""Charges a Tesla in the cheapest Nord Pool quarter-hours before Ready by.

charging.h plans and decides; charging_component.h connects it to ESPHome. This file checks the settings, the grid's
rates and the price list they start from (format in docs/grid-fees.md) when you build, and creates the web page's
entities.
"""

import re
from pathlib import Path

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, datetime, text_sensor, time
from esphome.components.http_request import CONF_HTTP_REQUEST_ID, HttpRequestComponent
from esphome.const import CONF_AREA, CONF_DISABLED_BY_DEFAULT, CONF_ID, CONF_NAME, CONF_TIME_ID

DEPENDENCIES = ["http_request", "network", "time"]
AUTO_LOAD = ["button", "datetime", "json", "text_sensor"]

CONF_BATTERY_KWH = "battery_kwh"
CONF_CALENDAR = "calendar"
CONF_CHARGING_KW = "charging_kw"
CONF_CLOCK = "clock"
CONF_CURRENCY = "currency"
CONF_EXCEPTIONS = "exceptions"
CONF_GRID = "grid"
CONF_MARKET = "market"
CONF_NTFY_SERVER = "ntfy_server"
CONF_NTFY_TOPIC = "ntfy_topic"
CONF_PRICELIST = "pricelist"
CONF_RATES = "rates"
CONF_VAT = "vat"
CONF_VIN = "vin"

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

# The price lists, which the build reads from this release and the board downloads from GitHub every day.
PRICE_LISTS = Path(__file__).resolve().parent.parent / "pricelists"

# The names of the days and months, in charging.h's order, from Sunday.
DAY_NAMES = ["sun", "mon", "tue", "wed", "thu", "fri", "sat"]
MONTH_NAMES = ["jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"]
# Words YAML reads as true, false or nothing.
NOT_NAMES = ["on", "off", "yes", "no", "true", "false", "null"]

charging_ns = cg.esphome_ns.namespace("charging")
ChargingComponent = charging_ns.class_("ChargingComponent", cg.PollingComponent)
ReadyBy = charging_ns.class_("ReadyBy", datetime.TimeEntity)
ReadyByOnce = charging_ns.class_("ReadyByOnce", datetime.DateTimeEntity)
ActionButton = charging_ns.class_("ActionButton", button.Button)
Action = charging_ns.enum("Action", is_class=True)


def _named(key, names):
    """The indexes `key` names among `names`: one name, or a range like fri-mon or nov-mar, which may wrap. None if
    it's neither."""
    first, dash, last = str(key).partition("-")
    last = last if dash else first
    if first not in names or last not in names:
        return None
    i, j = names.index(first), names.index(last)
    return [(i + k) % len(names) for k in range((j - i) % len(names) + 1)]


def _words(line):
    """A line's words, split at spaces as the board splits them."""
    return [word for word in line.split(" ") if word]


def _rate(value):
    """A rate's name: a word like night or p1."""
    if isinstance(value, bool) or value is None or str(value).lower() in NOT_NAMES:
        raise cv.Invalid(f"YAML reads {', '.join(NOT_NAMES)} as true, false or nothing, so they can't be rate names")
    if not isinstance(value, str) or not re.fullmatch(r"[^\W\d_][\w-]*", value):
        raise cv.Invalid(f'"{value}" isn\'t a rate name: a word like night or p1')
    return value


def _line(value):
    """A day's rates: the rate from midnight, then each time it changes, on a quarter-hour, and the rate from then."""
    words = _words(value) if isinstance(value, str) else [value]
    if len(words) % 2 == 0:
        raise cv.Invalid(f'"{value}" isn\'t rates and times by turns, like night 07:00 day 23:00 night')
    for rate in words[::2]:
        _rate(rate)
    minutes = 0
    for when in words[1::2]:
        m = re.fullmatch(r"(\d\d):(00|15|30|45)", when)
        if not m or not minutes < int(m[1]) * 60 + int(m[2]) < 24 * 60:
            raise cv.Invalid(f"{when} isn't a later quarter-hour, like 07:00 or 22:15")
        minutes = int(m[1]) * 60 + int(m[2])
    return value


def _each_once(names, what, example):
    """A table keyed like mon-fri or nov-mar, naming each of `names` once."""

    def validate(config):
        seen = []
        for key in config:
            found = _named(key, names)
            if found is None:
                raise cv.Invalid(f'"{key}" isn\'t a {what} or a range like {example}', [key])
            if set(found) & set(seen):
                raise cv.Invalid(f"{key} has a {what} that's there already", [key])
            seen += found
        if len(seen) < len(names):
            raise cv.Invalid(f"needs every {what}, like {example}")
        return config

    return validate


def _date(value):
    """A date like 12-25. 02-29 passes: it exists in leap years."""
    value = cv.string_strict(value)
    m = re.fullmatch(r"(\d\d)-(\d\d)", value)
    if (
        not m
        or not 1 <= int(m[1]) <= 12
        or not 1 <= int(m[2]) <= (31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31)[int(m[1]) - 1]
    ):
        raise cv.Invalid(f'"{value}" isn\'t a date like 12-25')
    return value


def _keys(check, values):
    """A table whose keys pass `check`, with its message, and whose values pass `values`."""

    def validate(config):
        config = cv.Schema({cv.valid: values})(config)
        for key in config:
            try:
                check(key)
            except cv.Invalid as error:
                raise cv.Invalid(error.msg, [key]) from error
        return config

    return validate


def _currency_code(value):
    value = cv.string_strict(value).upper()
    if not re.fullmatch(r"[A-Z]{3}", value):
        raise cv.Invalid("must be a currency's three-letter code, like EUR")
    return value


def _check_rates(settings, own):
    """Every rate the days use has a price, and every rate in `own` is used."""
    lines = [rates for week in settings[CONF_CALENDAR].values() for rates in week.values()]
    used = {rate for line in [*lines, *settings.get(CONF_EXCEPTIONS, {}).values()] for rate in _words(line)[::2]}
    if missing := sorted(used - set(settings.get(CONF_RATES, {}))):
        raise cv.Invalid(f"rate {missing[0]} needs a price per kWh, with VAT", [CONF_RATES])
    if unused := sorted(set(own) - used):
        raise cv.Invalid(f"rate {unused[0]} isn't used on any day", [CONF_RATES, unused[0]])


# The weeks by month, and the tables both price lists and config.yaml have.
CALENDAR = cv.All(
    cv.Schema(
        {cv.string_strict: cv.All(cv.Schema({cv.string_strict: _line}), _each_once(DAY_NAMES, "day", "mon-fri"))}
    ),
    _each_once(MONTH_NAMES, "month", "jan-dec"),
)
TABLES = {
    cv.Optional(CONF_CLOCK): cv.one_of("local", "winter"),
    cv.Optional(CONF_EXCEPTIONS): _keys(_date, _line),
    cv.Optional(CONF_RATES): _keys(_rate, cv.float_range(min=0.0)),
}
PRICE_LIST_SCHEMA = cv.Schema(
    {cv.Required(CONF_CALENDAR): CALENDAR, **TABLES, cv.Required(CONF_CURRENCY): _currency_code}
)


def _price_list_name(value):
    """A price list's name, like lt/eso-standartinis-4-zones."""
    value = cv.string_strict(value)
    if not re.fullmatch(r"[a-z]{2}/[a-z0-9-]+", value) or not (PRICE_LISTS / f"{value}.yaml").is_file():
        names = sorted(str(path.relative_to(PRICE_LISTS).with_suffix("")) for path in PRICE_LISTS.glob("*/*.yaml"))
        raise cv.Invalid(f"there's no price list {value}; there are {', '.join(names)}")
    return value


def _read(text):
    """Grid settings as the board reads them (read_grid() in charging.h), into tables with each key once."""
    settings, section, week = {}, None, None
    for number, line in enumerate(text.split("\n"), 1):
        line = line.rstrip(" \r")
        body = line.lstrip(" ")
        indent = len(line) - len(body)
        key, colon, rest = body.partition(":")
        value = rest.lstrip(" ")
        table = None
        if not body or body.startswith("#"):
            continue
        if not colon or not key or rest[:1] not in ("", " "):
            pass
        elif indent == 0 and not value and key in (CONF_CALENDAR, CONF_EXCEPTIONS, CONF_RATES):
            table, section, value = settings, key, {}
        elif indent == 0 and value and key in (CONF_CLOCK, CONF_CURRENCY):
            table, section = settings, None
        elif indent == 2 and not value and section == CONF_CALENDAR:
            table, week, value = settings[section], key, {}
        elif indent == 4 and value and section == CONF_CALENDAR and week is not None:
            table = settings[section][week]
        elif indent == 2 and value and section in (CONF_EXCEPTIONS, CONF_RATES):
            table = settings[section]
        if table is None or key in table:
            raise cv.Invalid(f"line {number} isn't a key and a value where it can be, or its key is there already")
        table[key] = value
    return settings


def _price_list(name):
    """A price list's settings, checked as the build checks config.yaml's, with every rate used."""
    try:
        settings = PRICE_LIST_SCHEMA(_read((PRICE_LISTS / f"{name}.yaml").read_text(encoding="utf-8")))
        _check_rates(settings, settings.get(CONF_RATES, {}))
    except cv.Invalid as error:
        raise cv.Invalid(f"price list {name}: {error}") from error
    return settings


def _with_price_list(config):
    """config.yaml's grid settings over the price list's, as make_grid() in charging.h puts them together."""
    settings = _price_list(config[CONF_PRICELIST]) if CONF_PRICELIST in config else {}
    if CONF_CALENDAR in config:
        settings[CONF_CALENDAR] = config[CONF_CALENDAR]
    for table in (CONF_EXCEPTIONS, CONF_RATES):
        settings[table] = {**settings.get(table, {}), **config.get(table, {})}
    if CONF_CALENDAR not in settings:
        raise cv.Invalid("needs a pricelist, or a calendar of its own")
    _check_rates(settings, config.get(CONF_RATES, {}))
    return config


GRID_SCHEMA = cv.All(
    cv.Schema({cv.Optional(CONF_CALENDAR): CALENDAR, **TABLES, cv.Optional(CONF_PRICELIST): _price_list_name}),
    _with_price_list,
)

MARKET_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_AREA): cv.one_of(*AREAS, upper=True),
        cv.Required(CONF_VAT): cv.float_range(min=0.0, max=1.0, max_included=False),
    }
)


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


def _currency(config):
    """The market area's currency unless set, otherwise euros. Without a market, any currency. A price list's must be
    the same."""
    market = config.get(CONF_MARKET)
    area = market[CONF_AREA] if market else ""
    config.setdefault(CONF_CURRENCY, CURRENCIES.get(area[:2], "EUR"))
    if market and config[CONF_CURRENCY] not in NORD_POOL_CURRENCIES:
        raise cv.Invalid(f"Nord Pool's prices come in {', '.join(NORD_POOL_CURRENCIES)}", [CONF_CURRENCY])
    name = config[CONF_GRID].get(CONF_PRICELIST)
    if name and (currency := _price_list(name)[CONF_CURRENCY]) != config[CONF_CURRENCY]:
        raise cv.Invalid(f"the price list {name} is in {currency}: set currency: {currency}", [CONF_CURRENCY])
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ChargingComponent),
            cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
            cv.GenerateID(CONF_HTTP_REQUEST_ID): cv.use_id(HttpRequestComponent),
            cv.Required(CONF_BATTERY_KWH): cv.positive_not_null_float,
            cv.Required(CONF_CHARGING_KW): cv.positive_not_null_float,
            cv.Optional(CONF_CURRENCY): _currency_code,
            cv.Required(CONF_GRID): GRID_SCHEMA,
            cv.Optional(CONF_MARKET): MARKET_SCHEMA,
            cv.Optional(CONF_NTFY_SERVER, default="https://ntfy.sh"): cv.url,
            cv.Optional(CONF_NTFY_TOPIC, default=""): cv.string,
            cv.Required(CONF_VIN): _vin,
        }
    ).extend(cv.polling_component_schema("30s")),
    _currency,
)


def _write(grid):
    """config.yaml's grid settings as the board reads them (read_grid() in charging.h)."""
    lines = []
    if CONF_CALENDAR in grid:
        lines.append(f"{CONF_CALENDAR}:")
        for months, week in grid[CONF_CALENDAR].items():
            lines += [f"  {months}:", *(f"    {days}: {rates}" for days, rates in week.items())]
    if CONF_CLOCK in grid:
        lines.append(f"{CONF_CLOCK}: {grid[CONF_CLOCK]}")
    for table in (CONF_EXCEPTIONS, CONF_RATES):
        if table in grid:
            lines += [f"{table}:", *(f"  {key}: {value}" for key, value in grid[table].items())]
    return "\n".join(lines)


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
    grid = config[CONF_GRID]
    name = grid.get(CONF_PRICELIST, "")
    text = (PRICE_LISTS / f"{name}.yaml").read_text(encoding="utf-8") if name else ""
    cg.add(var.set_grid(market[CONF_VAT] if market else 0.0, name, text, _write(grid)))

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
