"""Charges a Tesla in the cheapest Nord Pool quarter-hours before Ready by.

charger.h decides; scheduler_component.h connects it to ESPHome. This file checks the settings when you build,
grid.py the grid: block and its plan, and creates the web page's entities.
"""

import re

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, datetime, text_sensor, time
from esphome.components.http_request import CONF_HTTP_REQUEST_ID, HttpRequestComponent
from esphome.const import CONF_AREA, CONF_DISABLED_BY_DEFAULT, CONF_ID, CONF_NAME, CONF_TIME_ID

from .grid import CONF_CURRENCY, CONF_PLAN, GRID_SCHEMA, PLANS, plan, write_grid

DEPENDENCIES = ["http_request", "network", "time"]
AUTO_LOAD = ["button", "datetime", "json", "text_sensor"]

CONF_BATTERY_KWH = "battery_kwh"
CONF_CHARGING_KW = "charging_kw"
CONF_GRID = "grid"
CONF_MARKET = "market"
CONF_NTFY_SERVER = "ntfy_server"
CONF_NTFY_TOPIC = "ntfy_topic"
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

scheduler_ns = cg.esphome_ns.namespace("scheduler")
SchedulerComponent = scheduler_ns.class_("SchedulerComponent", cg.PollingComponent)
ReadyBy = scheduler_ns.class_("ReadyBy", datetime.TimeEntity)
ReadyByOnce = scheduler_ns.class_("ReadyByOnce", datetime.DateTimeEntity)
ActionButton = scheduler_ns.class_("ActionButton", button.Button)
Action = scheduler_ns.enum("Action", is_class=True)


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


def _currency_code(value):
    value = cv.string_strict(value).upper()
    if not re.fullmatch(r"[A-Z]{3}", value):
        raise cv.Invalid("must be a currency's three-letter code, like EUR")
    return value


def _currency(config):
    """The market area's currency unless set, otherwise euros. Without a market, any currency. A plan's must be
    the same."""
    market = config.get(CONF_MARKET)
    area = market[CONF_AREA] if market else ""
    config.setdefault(CONF_CURRENCY, CURRENCIES.get(area[:2], "EUR"))
    if market and config[CONF_CURRENCY] not in NORD_POOL_CURRENCIES:
        raise cv.Invalid(f"Nord Pool's prices come in {', '.join(NORD_POOL_CURRENCIES)}", [CONF_CURRENCY])
    name = config[CONF_GRID].get(CONF_PLAN)
    if name and (currency := plan(name)[CONF_CURRENCY]) != config[CONF_CURRENCY]:
        raise cv.Invalid(f"the plan {name} is in {currency}: set currency: {currency}", [CONF_CURRENCY])
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SchedulerComponent),
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


# The web page finds these entities by name, and the board keeps Ready by and Ready by once under theirs, so the
# names are fixed.
def _entity(cls, key, name, **extra):
    return {CONF_ID: cv.declare_id(cls)(f"scheduler_{key}"), CONF_NAME: name, CONF_DISABLED_BY_DEFAULT: False, **extra}


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
    name = grid.get(CONF_PLAN, "")
    text = (PLANS / f"{name}.yaml").read_text(encoding="utf-8") if name else ""
    cg.add(var.set_grid(market[CONF_VAT] if market else 0.0, name, text, write_grid(grid)))

    ready_by = await datetime.new_datetime(_entity(ReadyBy, "ready_by", "Ready by", type="TIME"))
    await cg.register_parented(ready_by, var)
    cg.add(var.set_ready_by(ready_by))
    ready_by_once = await datetime.new_datetime(_entity(ReadyByOnce, "ready_by_once", "Ready by once", type="DATETIME"))
    await cg.register_parented(ready_by_once, var)
    cg.add(var.set_ready_by_once(ready_by_once))

    for key, name, action in (
        ("create_schedule", "Create schedule", Action.CREATE_SCHEDULE),
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
