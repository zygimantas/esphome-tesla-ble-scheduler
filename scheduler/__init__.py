"""Charges a Tesla in the cheapest quarter-hours of the day-ahead market before Ready by.

charger.h decides; scheduler_component.h connects it to ESPHome. The settings are a file uploaded on the board's page,
which settings.h reads and checks. This file builds in the plans of plans/ and creates the web page's entities.
"""

from pathlib import Path

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, datetime, text_sensor, time
from esphome.components.http_request import CONF_HTTP_REQUEST_ID, HttpRequestComponent
from esphome.const import CONF_DISABLED_BY_DEFAULT, CONF_ID, CONF_NAME, CONF_TIME_ID

DEPENDENCIES = ["http_request", "network", "time"]
AUTO_LOAD = ["button", "datetime", "json", "text_sensor"]

# The plans the settings can name, from this release; boards download them from GitHub every day.
PLANS = Path(__file__).resolve().parent.parent / "plans"

scheduler_ns = cg.esphome_ns.namespace("scheduler")
SchedulerComponent = scheduler_ns.class_("SchedulerComponent", cg.PollingComponent)
ReadyBy = scheduler_ns.class_("ReadyBy", datetime.TimeEntity)
ReadyByOnce = scheduler_ns.class_("ReadyByOnce", datetime.DateTimeEntity)
ActionButton = scheduler_ns.class_("ActionButton", button.Button)
Action = scheduler_ns.enum("Action", is_class=True)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SchedulerComponent),
        cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
        cv.GenerateID(CONF_HTTP_REQUEST_ID): cv.use_id(HttpRequestComponent),
    }
).extend(cv.polling_component_schema("30s"))


# The web page finds these entities by name, and the board keeps Ready by and Ready by once under theirs, so the
# names are fixed.
def _entity(cls, key, name, **extra):
    return {CONF_ID: cv.declare_id(cls)(f"scheduler_{key}"), CONF_NAME: name, CONF_DISABLED_BY_DEFAULT: False, **extra}


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_clock(await cg.get_variable(config[CONF_TIME_ID])))
    cg.add(var.set_http(await cg.get_variable(config[CONF_HTTP_REQUEST_ID])))
    for path in sorted(PLANS.glob("*/*.yaml")):
        cg.add(var.add_plan(str(path.relative_to(PLANS).with_suffix("")), path.read_text(encoding="utf-8")))
    cg.add(var.load_settings())

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
