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

## Example of yearly savings

Worked out on the market prices of every night from October 2025, when they went to quarter-hours, to September 2026, for one driver, the same in every country:

- **The car:** a 75 kWh Tesla that drives 20,000 km a year, about 55 km a day, at 15 kWh per 100 km.
- **Every night:** plugged in from 18:00 to 07:00, charging at 11 kW.
- **The driver already saves:** the Tesla's own scheduler starts charging at the same time every night, when cheap hours usually begin.
- **The contract:** a price that changes every hour with the market, called dynamic or spot, or the usual price where homes can't have one.
- **Full prices:** each kWh costs what the bill would say, with grid fees, taxes and VAT.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/images/savings-dark.svg">
  <img src="docs/images/savings-light.svg" width="720" alt="October 2025 to September 2026: 20,000 km a year, about 55 km a day. Sweden, Vattenfall Tidstariff (SE3), in kronor: 7,543 SEK charging at once, 6,197 SEK with the car's own schedule from 22:00 and 4,974 SEK with this board. Lithuania, ESO Standartinis with four zones: 1,001 EUR at once, 786 EUR from 22:00 and 536 EUR with this board. Spain, 2.0TD: 744 EUR at once, 616 EUR from 00:00 and 476 EUR with this board.">
</picture>

That's 20% less than the car's own schedule in Sweden, 32% in Lithuania and 23% in Spain, as the board finds the cheap quarter-hours wherever they fall each night. No fixed start does as well: the board still costs 5%, 8% and 12% less than the best one, which shows only afterwards, from 03:30 in Sweden, 03:45 in Lithuania and 04:00 in Spain. A driver whose car charges as soon as it's plugged in would save more: 34%, 46% and 36%.

### Every country

Where homes pay the market price, charging with the board cost 5% to 33% less than charging scheduled for the night. Much of that comes from charging later in the night, which a schedule can do too: set to the start time that turned out cheapest over the year, it came within 2% of the board in most of these countries, though no one knows that time in advance. Where homes pay regulated or fixed prices, a night schedule does as well as the board. In many countries few homes pay the market price yet, as Confidence says. All these are shares of what charging costs, not of the whole bill.

What the board would save you in a year, if your car now charges on a schedule for the night: in Lithuania, for example, 250 EUR, 32% less than a schedule from 22:00.

| Country | Saved a year against a night schedule | Confidence |
| --- | --- | --- |
| [Estonia](plans/ee/AGENTS.md) | 212 EUR, 33% | High: exchange-price packages are common |
| [Lithuania](plans/lt/AGENTS.md) | 250 EUR, 32% | Medium: a few percent of homes pay each quarter-hour's price, most a fixed one |
| [Czechia](plans/cz/AGENTS.md) | 5,578 CZK, 30% | Low: few homes pay the market price |
| [Latvia](plans/lv/AGENTS.md) | 142 EUR, 25% | High: one home in six pays the market price |
| [France](plans/fr/AGENTS.md) | 171 EUR, 23% | Low: two small suppliers bill the market price, and off-peak hours differ by address |
| [Spain](plans/es/AGENTS.md) | 140 EUR, 23% | High: over a quarter of homes pay each hour's price, most on the regulated PVPC |
| [Finland](plans/fi/AGENTS.md) | 99 EUR, 22% | High: a third of homes pay the market price |
| [Sweden](plans/se/AGENTS.md) | 1,223 SEK, 20% | High: one contract in seven pays each quarter-hour's or hour's price, most the month's average |
| [Slovenia](plans/si/AGENTS.md) | 120 EUR, 17% | Low: 0.3% of homes pay the market price |
| [Portugal](plans/pt/AGENTS.md) | 112 EUR, 16% | Low: under 1% of homes pay the market price |
| [Romania](plans/ro/AGENTS.md) | 806 RON, 14% | Low: no home had a dynamic contract in 2025 |
| [Denmark](plans/dk/AGENTS.md) | 391 DKK, 9% | High: most homes pay the market price |
| [Germany](plans/de/AGENTS.md) | 61 EUR, 8% | Low: dynamic contracts and Modul 3 need a smart meter, and 6% of meters are one |
| [Luxembourg](plans/lu/AGENTS.md) | 49 EUR, 8% | Low: few homes pay the market price |
| [Poland](plans/pl/AGENTS.md) | 241 PLN, 8% | Low: 0.03% of homes have a dynamic contract, and it bills another auction's price |
| [Belgium](plans/be/AGENTS.md) | 56 EUR, 7% | Low: 0.4% of homes pay the market price, and Flanders' fee per kW isn't counted |
| [Netherlands](plans/nl/AGENTS.md) | 60 EUR, 7% | High: 8% of homes pay the market price |
| [Norway](plans/no/AGENTS.md) | 276 NOK, 6% | Medium: most homes' power is at Norgespris, a fixed price, where the car's own schedule does as well |
| [Austria](plans/at/AGENTS.md) | 48 EUR, 5% | Low: about 1% of homes paid the market price in 2025 |
| [Italy](plans/it/AGENTS.md) | 45 EUR, 5% | Medium: about 3% of homes pay the hourly price |
| [Bulgaria](plans/bg/AGENTS.md) | 0 EUR, 0% | High: nearly every home pays a regulated price, cheaper at night |
| [Croatia](plans/hr/AGENTS.md) | 0 EUR, 0% | High: no home pays the market price, and the regulated one is cheaper at night |
| [Hungary](plans/hu/AGENTS.md) | 0 HUF, 0% | Medium: homes pay regulated prices, and the car's own meter (A2) pays less at night |
| [Ireland](plans/ie/AGENTS.md) | 0 EUR, 0% | High: electric car owners usually take a fixed plan with a cheap night, cheaper than dynamic contracts |
| [Slovakia](plans/sk/AGENTS.md) | 0 EUR, 0% | High: nearly every home pays a regulated price, cheaper at night |
| [Switzerland](plans/ch/AGENTS.md) | 0 CHF, 0% | High: homes pay their grid operator's price, which at EKZ, the largest, is the same at every hour |

Confidence says how many homes there pay the prices the example takes. High: many do. Medium: a few percent do, or most chose another price. Low: hardly any do, or the bill follows other prices.

AI agents researched each country in October 2026: its notes give every figure's source, what the example settled and what's still open, like [Lithuania's](plans/lt/AGENTS.md). Something wrong or out of date? [Open an issue](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/new/choose).

<details>
<summary>How it's calculated</summary>

- **The nights:** 365, from 1 October 2025 to 30 September 2026, in local time. 20,000 km a year at 15 kWh per 100 km is 8.2 kWh a night into the battery, and 9.1 kWh from the grid, as the board counts 90% of it reaching the battery.
- **The market prices:** each quarter-hour's, from the day-ahead auction, where a country has several areas, Stockholm's (SE3), Oslo's (NO1), Copenhagen's (DK2) and Italy's north. As Nord Pool keeps only its last two months free to download, they come from others that publish the same, to the cent where checked: Elering, Estonia's grid operator, for the Baltics and Finland, elprisetjustnu.se for Sweden, Energy-Charts for Norway and Romania and SMARD, Germany's Federal Network Agency, for the rest; for Spain and Portugal from OMIE, the Iberian market, as the board downloads them. Other currencies are at the ECB's daily rate, within half a percent of Nord Pool's own. Where bills count hours, as many still do, each hour costs the average of its four quarter-hours.
- **The grid fees:** the board's own plans, at their 2026 prices all year: each country's largest grid operator's, with a cheaper night where it has one, like [Vattenfall Tidstariff](plans/se/vattenfall-tidstariff.yaml), [ESO Standartinis with four zones](plans/lt/eso-standartinis-4-zones.yaml) and [2.0TD](plans/es/2-0td.yaml) in the chart. France's off-peak hours from 22:00 are a custom plan, as the board has none, and the Netherlands has no grid fee per kWh.
- **The rest of each kWh:** VAT, a typical dynamic contract's margin, and what's the same every hour: taxes, levies and, where suppliers bill them, grid losses, at their 2026 rates. Monthly fees are left out, as they don't depend on when the car charges, and so are fees per kW, which the board doesn't count yet ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)).
- **Regulated and fixed prices:** where homes can't have a dynamic contract, or it costs more than their usual price, the usual price, with its network fees and levies: the regulated prices in Bulgaria, Croatia, Slovakia and Switzerland, in Hungary A2, for an electric car's own meter, and in Ireland Night Boost, a plan for electric cars with a cheap night.
- **The car's own schedule:** from when the grid plan's night rate starts, like 22:00 in Lithuania, or from midnight where the rate is the same all night.
- **This board:** the board's own scheduler, the same code, picks each night's cheapest quarter-hours from the prices out at 18:00, for the same energy.
- **The research:** AI agents gathered each country's figures, one per country, from regulators', statistics offices' and suppliers' own pages, each with its source. Scripts then checked that no source or figure was lost when the notes were tidied, that each country's summary matches its row here, and that the market prices match Nord Pool's where both could be compared. Each country's notes list the sources to re-check every January.
- **The chart:** Sweden's costs are in kronor, as its board counts them, and its bars as long as their euros at the ECB's daily rate, so the three compare.
- **To check it**, run [test/savings_example.cpp](test/savings_example.cpp) in the project's folder, with a C++ compiler, curl and ArduinoJson, as [CONTRIBUTING.md](CONTRIBUTING.md#checks) shows. It downloads the prices and prints the costs and the table.

```bash
g++ -std=c++17 -I . -isystem ArduinoJson/src test/savings_example.cpp -o test/savings_example
test/savings_example
```

</details>

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
