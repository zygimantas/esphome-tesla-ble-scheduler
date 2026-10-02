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
CONF_NORDPOOL = "nordpool"
CONF_PRICES = "prices"
CONF_VAT = "vat"
CONF_CURRENCY = "currency"
CONF_NTFY_SERVER = "ntfy_server"
CONF_NTFY_TOPIC = "ntfy_topic"
CONF_VIN = "vin"
CONF_GRID = "grid"
CONF_HOLIDAYS = "holidays"
CONF_CLOCK = "clock"
CONF_WORKDAY = "workday"
CONF_WEEKEND = "weekend"
CONF_HOLIDAY = "holiday"
CONF_FEES = "fees"
CONF_WINTER = "winter"
CONF_FROM = "from"
CONF_TO = "to"

# Where electricity is bought: a country's code, or the price area where a country has several. DE is Germany and
# Luxembourg. Nord Pool names an area as here unless NORD_POOL_AREAS says otherwise, and prices are in euros unless
# CURRENCIES has the country.
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
    "LV",
    "NL",
    "NO1",
    "NO2",
    "NO3",
    "NO4",
    "NO5",
    "PL",
    "SE1",
    "SE2",
    "SE3",
    "SE4",
]
NORD_POOL_AREAS = {"DE": "GER"}
CURRENCIES = {"DK": "DKK", "NO": "NOK", "SE": "SEK"}

charging_ns = cg.esphome_ns.namespace("charging")
ChargingComponent = charging_ns.class_("ChargingComponent", cg.PollingComponent)
ReadyBy = charging_ns.class_("ReadyBy", datetime.TimeEntity)
ReadyByOnce = charging_ns.class_("ReadyByOnce", datetime.DateTimeEntity)
ActionButton = charging_ns.class_("ActionButton", button.Button)
Action = charging_ns.enum("Action", is_class=True)
Grid = charging_ns.struct("Grid")


def _hours(value):
    value = cv.string_strict(value)
    if not re.fullmatch(r"[a-z]{24}", value):
        raise cv.Invalid("must be 24 lowercase letters, one zone per hour from 00:00 to 23:00")
    return value


def _month_day(value):
    """Parses "12-25" to 1225 (month * 100 + day). "02-29" passes: it exists in leap years."""
    value = cv.string_strict(value)
    m = re.fullmatch(r"(\d\d)-(\d\d)", value)
    month, day = (int(m[1]), int(m[2])) if m else (0, 0)
    if 1 <= month <= 12 and 1 <= day <= (31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31)[month - 1]:
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


def _currency(config):
    """The area's currency, unless set."""
    config.setdefault(CONF_CURRENCY, CURRENCIES.get(config[CONF_AREA][:2], "EUR"))
    return config


def _validate_grid(config):
    days = [CONF_WORKDAY, CONF_WEEKEND]
    if config[CONF_HOLIDAYS]:
        if CONF_HOLIDAY not in config[CONF_HOURS]:
            raise cv.Invalid("needs the zone of each hour on public holidays", path=[CONF_HOURS, CONF_HOLIDAY])
        days.append(CONF_HOLIDAY)
    winter = config[CONF_HOURS].get(CONF_WINTER, {})
    zones = set("".join(config[CONF_HOURS].get(day, "") + winter.get(day, "") for day in days))
    if missing := sorted(zones - set(config[CONF_FEES])):
        raise cv.Invalid(f'zone "{missing[0]}" needs a fee per kWh incl. VAT', path=[CONF_FEES])
    if unused := sorted(set(config[CONF_FEES]) - zones):
        raise cv.Invalid(f'zone "{unused[0]}" isn\'t used in any hour', path=[CONF_FEES, unused[0]])
    return config


GRID_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Optional(CONF_HOLIDAYS, default=[]): cv.ensure_list(_holiday),
            cv.Optional(CONF_CLOCK, default="local"): cv.one_of("local", "winter"),
            cv.Required(CONF_HOURS): cv.Schema(
                {
                    cv.Required(CONF_WORKDAY): _hours,
                    cv.Required(CONF_WEEKEND): _hours,
                    cv.Optional(CONF_HOLIDAY): _hours,
                    cv.Optional(CONF_WINTER): cv.All(
                        cv.Schema(
                            {
                                cv.Required(CONF_FROM): _month_day,
                                cv.Required(CONF_TO): _month_day,
                                cv.Optional(CONF_WORKDAY): _hours,
                                cv.Optional(CONF_WEEKEND): _hours,
                                cv.Optional(CONF_HOLIDAY): _hours,
                            }
                        ),
                        cv.has_at_least_one_key(CONF_WORKDAY, CONF_WEEKEND, CONF_HOLIDAY),
                    ),
                }
            ),
            cv.Required(CONF_FEES): cv.Schema({cv.string_strict: cv.float_range(min=0.0)}),
        }
    ),
    _validate_grid,
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ChargingComponent),
        cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
        cv.GenerateID(CONF_HTTP_REQUEST_ID): cv.use_id(HttpRequestComponent),
        cv.Required(CONF_BATTERY_KWH): cv.positive_not_null_float,
        cv.Required(CONF_CHARGING_KW): cv.positive_not_null_float,
        cv.Optional(CONF_NORDPOOL): cv.invalid("is called prices: now, and GER is DE"),
        cv.Required(CONF_PRICES): cv.All(
            cv.Schema(
                {
                    cv.Required(CONF_AREA): cv.one_of(*AREAS, upper=True),
                    cv.Optional(CONF_CURRENCY): cv.one_of("DKK", "EUR", "NOK", "SEK", upper=True),
                    cv.Required(CONF_VAT): cv.float_range(min=0.0, max=1.0, max_included=False),
                }
            ),
            _currency,
        ),
        cv.Optional(CONF_NTFY_SERVER, default="https://ntfy.sh"): cv.url,
        cv.Optional(CONF_NTFY_TOPIC, default=""): cv.string,
        cv.Required(CONF_VIN): _vin,
        cv.Required(CONF_GRID): GRID_SCHEMA,
    }
).extend(cv.polling_component_schema("30s"))


def _grid(config, vat):
    hours = config[CONF_HOURS]
    holidays = config[CONF_HOLIDAYS]
    winter = hours.get(CONF_WINTER, {})
    return cg.StructInitializer(
        Grid,
        ("vat", vat),
        ("winter_clock", config[CONF_CLOCK] == "winter"),
        ("workday", hours[CONF_WORKDAY]),
        ("weekend", hours[CONF_WEEKEND]),
        ("holiday", hours.get(CONF_HOLIDAY, "")),
        ("fee", [config[CONF_FEES].get(zone, 0.0) for zone in ascii_lowercase]),
        ("holidays", [h for h in holidays if not isinstance(h, tuple)]),
        ("after_easter", [h[1] for h in holidays if isinstance(h, tuple)]),
        ("winter_from", winter.get(CONF_FROM, 0)),
        ("winter_to", winter.get(CONF_TO, 0)),
        ("winter_workday", winter.get(CONF_WORKDAY, "")),
        ("winter_weekend", winter.get(CONF_WEEKEND, "")),
        ("winter_holiday", winter.get(CONF_HOLIDAY, "")),
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
    prices = config[CONF_PRICES]
    cg.add(var.set_nord_pool_area(NORD_POOL_AREAS.get(prices[CONF_AREA], prices[CONF_AREA])))
    cg.add(var.set_currency(prices[CONF_CURRENCY]))
    cg.add(var.set_battery_kwh(config[CONF_BATTERY_KWH]))
    cg.add(var.set_charging_kw(config[CONF_CHARGING_KW]))
    cg.add(var.set_ntfy(config[CONF_NTFY_SERVER], config[CONF_NTFY_TOPIC]))
    cg.add(var.set_grid(_grid(config[CONF_GRID], prices[CONF_VAT])))

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
