<div align="center">

<img src="docs/images/logo.svg" width="88" height="88" alt="">

# ESPHome Tesla BLE Scheduler

**Charge your Tesla in the cheapest hours of the night, automatically.**<br>
A small ESP32 board by the car follows the electricity market and controls charging over Bluetooth.

[![Latest release](https://img.shields.io/github/v/release/zygimantas/esphome-tesla-ble-scheduler)](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest)
[![CI](https://github.com/zygimantas/esphome-tesla-ble-scheduler/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/zygimantas/esphome-tesla-ble-scheduler/actions/workflows/ci.yml)
[![License](https://img.shields.io/github/license/zygimantas/esphome-tesla-ble-scheduler)](LICENSE)

[Install](#install) · [How it works](#how-it-works) · [Countries](plans/README.md) · [User guide](docs/guide.md) · [Contributing](CONTRIBUTING.md)

<img src="docs/images/hero.png" width="760" alt="The board's page on a phone: tonight's schedule, one window from 00:45 to 05:30, and its settings">

</div>

## Why

Day-ahead electricity prices change every quarter-hour, and the cheapest hours of a night often cost a fraction of the evening peak. A Tesla's own schedule knows times, not prices, and the tools that follow prices usually need a cloud service, your Tesla account, Home Assistant or a new charger.

This board needs none of them. When you plug in, it picks the cheapest quarter-hours that still get the car to its charge limit by the time you leave, counting your grid fees and VAT.

- **Runs at home:** no cloud, no Tesla account, no subscription, no Home Assistant.
- **Can only charge:** its key can't unlock the doors or drive the car.
- **Works in 26 European countries,** most with their grid operators' plans built in.
- **Updates from its own page,** with releases signed by the project.

## What it saves

Every night for a year, from October 2025, when the market prices went to quarter-hours, to September 2026, a 75 kWh Tesla that drives 100 km a day comes home at 18:00 with 60% and needs 80% by 07:00, charged at 11 kW on a dynamic contract. Each kWh costs what the bill charges for it at 2026's prices: the market price, the grid plan's fee for that quarter-hour, taxes, levies, the supplier's margin and VAT.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/images/savings-dark.svg">
  <img src="docs/images/savings-light.svg" width="720" alt="A year, October 2025 to September 2026, driving 100 km a day and charging from 60% to 80% every night. Germany, Westnetz Modul 3: 2,760 EUR charging at once, 1,452 EUR with the car's own schedule from 00:00 and 1,365 EUR with this board. Lithuania, ESO Standartinis with four zones: 1,869 EUR at once, 1,385 EUR from 22:00 and 994 EUR with this board.">
</picture>

That's 51% less than charging at once in Germany and 47% in Lithuania, and 6% and 28% less than the car's own schedule, as the board finds the cheap quarter-hours wherever they fall each night. No fixed start does as well: the best one, which shows only afterwards, from 03:00 in Germany and 03:15 in Lithuania, still costs 1% and 7% more.

The costs come from the published prices: the market's, from SMARD in Germany and Nord Pool in Lithuania, the plans' in [plans/](plans/), and the taxes, levies and a typical margin, with their sources in [test/savings_example.cpp](test/savings_example.cpp), which reruns it all with the board's own scheduler.

## How it works

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/images/how-it-works-dark.svg">
  <img src="docs/images/how-it-works-light.svg" width="760" alt="Day-ahead prices and your grid plan go into the board next to the car, which starts and stops your Tesla over Bluetooth, and your phone reaches the board's page over Wi-Fi">
</picture>

1. **Prices:** the board downloads tomorrow's prices once they're out, around 13:00 CET, and adds VAT and your grid plan's fees to each quarter-hour.
2. **Schedule:** when you plug in, it picks the cheapest quarter-hours that reach your charge limit by **Ready by**, with a spare one in case charging runs slow.
3. **Charging:** it starts and stops charging over Bluetooth, and sends the schedule to your phone through the ntfy app, if you like.

## What you need

- A Tesla Model 3, Model Y, Cybertruck, or Model S/X from 2021 on.
- An ESP32-S3-DevKitC-1 (N16R8) board, within Bluetooth range of the car and on your Wi-Fi.
- A USB-C cable for your computer, and a USB-C phone charger near the car to power the board.
- A home charger that charges whenever the car asks: no schedule, auto-lock or app approval (OCPP) of its own.
- A price that follows the day-ahead market, often called spot or exchange price, in Austria, Belgium, Bulgaria, Croatia, Czechia, Denmark, Estonia, Finland, France, Germany, Hungary, Ireland, Italy (the north), Latvia, Lithuania, Luxembourg, the Netherlands, Norway, Poland, Portugal, Romania, Slovakia, Slovenia, Spain, Sweden or Switzerland. [Countries and plans](plans/README.md) has the details, and how to use a fixed price instead.
- Chrome or Edge on a computer, for the first install.

## Install

1. **At your computer:** download [the firmware](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest/download/esphome-tesla-ble-scheduler.bin), plug the board's USB-C port labelled **COM** into the computer, and install the firmware with [ESPHome Web](https://web.esphome.io) in Chrome or Edge. Then give the board your Wi-Fi.
2. **On the board's page,** which ESPHome Web then opens: choose your country, grid plan and contract, and scan the QR code with your phone.
3. **At the car:** plug the board into a phone charger near the car, sit in the car with your key card, and finish the setup on your phone.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/images/setup-dark.gif">
  <img src="docs/images/setup-light.gif" width="800" alt="The whole setup: at a computer, ESPHome Web installs the firmware and sets up the Wi-Fi, then the board's page at tesla.local takes the prices and shows a QR code; the phone in the car scans it, opens the page and goes through the VIN, the charging key and the end of the setup, then shows the first schedule once the car is plugged in">
</picture>

From then on, just plug in: the car is charged by **Ready by**. The [user guide](docs/guide.md) explains the page, phone messages and what to do when something is off.

## Questions

<details>
<summary><b>Can I still charge right away?</b></summary>

Yes. **Start charging now** on the board's page charges to the limit at any price, and charging you start from the car or the Tesla app goes ahead: the board leaves it alone until you unplug.

</details>

<details>
<summary><b>What if the board or the prices are missing?</b></summary>

The car charges as usual. Without prices or a battery level, the board lets the car charge at any price, and tells your phone if messages are on. With the board unplugged, the car is on its own.

</details>

<details>
<summary><b>What can the board's key do?</b></summary>

Only charging. The board adds its own key, like a phone key, with Tesla's Charging Manager role: it can start and stop charging and set the charge limit, but it can't unlock the doors or drive. You can remove it in the car under **Controls** → **Locks**.

</details>

<details>
<summary><b>What leaves my home?</b></summary>

No account, and nothing goes through Tesla's servers. The board downloads market prices, grid plans and, when you open its page, news of a release. Only phone messages go out, if you turn them on: the schedule, through the ntfy app's server, under a long random topic. The page has no password: anyone on your Wi-Fi can open it.

</details>

## For developers

The scheduler is plain C++17 that runs on a computer as well as on the board.

- **Tested:** 85 unit tests cover every line and branch of the scheduler, mutation testing kills 92% of its mutants, and a simulation runs the board's logic with a simulated Tesla.
- **Built on ESPHome:** an external component in `scheduler/`, with [esphome-tesla-ble](https://github.com/yoziru/esphome-tesla-ble) for the car, and a page of plain JavaScript and CSS that the board serves.
- **Checked in CI:** clang-tidy, CodeQL, SonarCloud and the page's tests in a headless browser, and every release signed for the boards' updates.
- **Open to your country:** a grid operator's plan is a short YAML file in `plans/`, which every board downloads within a day.

[CONTRIBUTING.md](CONTRIBUTING.md) has the checks to run and how releases are made.

## Support

If it saves you money, star the repository so others find it, or [buy the developer a coffee](https://buymeacoffee.com/zygimantas_berziunas). Ideas and problems are welcome as [issues](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues).

## Credits and license

The Bluetooth link to the car is [esphome-tesla-ble](https://github.com/yoziru/esphome-tesla-ble), which implements Tesla's [vehicle-command](https://github.com/teslamotors/vehicle-command) protocol. Prices come from Nord Pool's data portal, SMARD (Bundesnetzagentur | SMARD.de, CC BY 4.0), OMIE (OMI-Polo Español, S.A.), OKTE (OKTE, a.s.) and SEMOpx (the market of Ireland and Northern Ireland). This project isn't affiliated with Tesla, Nord Pool, the Bundesnetzagentur, OMIE, OKTE, SEMOpx or any grid operator. Use it at your own risk.

Licensed under the [GNU Affero General Public License v3.0](LICENSE).
