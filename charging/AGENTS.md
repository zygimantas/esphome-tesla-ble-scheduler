# charging/

- ESPHome compiles every C and C++ file in this folder into the firmware, so tests, tools and scratch code live elsewhere. Other files, like this one, are fine.
- charging.h is built as plain C++17 by the unit tests, without ESPHome, and as gnu++20 in the firmware. Keep it C++17 and free of ESPHome headers. The firmware's generated main.cpp builds Grid with designated initializers, a C++20 feature.
- make_plan() relies on 0.0f / 0.0f giving NaN for a plan without energy: never build with -ffast-math or -ffinite-math-only.
- Entity names are an interface. The page finds the board's entities by name (web/web_ui.js), this component finds the Tesla's by name, and Ready by and Ready by once are saved under ESPHome's template time and datetime keys combined with their names. Renaming an entity, or an ESPHome release that changes those keys, silently loses the saved values: check after upgrading ESPHome.
- esphome-tesla-ble, at the pinned `tesla_ble_ref`: the "Charger" binary sensor keeps its state once the car has reported it (only the asleep and user-present sensors go back to unknown); the battery level and charge limit keep their last values; the charging states are "Disconnected", "No Power", "Starting", "Charging", "Complete", "Stopped", "Calibrating" and "Unknown". The controller relies on all three.
- Controller::observe_() runs before the plan is made, so it judges a start from the car against the previous tick's plan. On the first tick after a restart there's none, and in_plan_() is true.
