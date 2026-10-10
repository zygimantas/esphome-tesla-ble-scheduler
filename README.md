# ESPHome Tesla BLE Scheduler

[![Latest release](https://img.shields.io/github/v/release/zygimantas/esphome-tesla-ble-scheduler)](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest)
[![License](https://img.shields.io/github/license/zygimantas/esphome-tesla-ble-scheduler)](LICENSE)

**Charge your Tesla in the cheapest hours, automatically, with a small board next to the car.**

<p align="center"><img src="https://github.com/user-attachments/assets/fb4e0cc5-3873-4390-baa4-68e3c73b7a39" alt="The board's page on a phone: tonight's charging windows and their prices" width="240"></p>

Day-ahead electricity prices change every quarter-hour, and the cheapest hours of a night often cost a fraction of the evening peak. A Tesla's own schedule knows times, not prices.

When you plug in, the board picks the cheapest quarter-hours that still get the car to its charge limit by the time you leave, counting your grid fees and VAT. Then it starts and stops charging over Bluetooth.

- **No cloud:** no Tesla account, no subscription, no Home Assistant.
- **Charging only:** its key can't open the car or drive it.
- **26 European countries,** most with their grid operators' plans.

## What it saves

An ordinary day in Lithuania, 27 September 2026: a 75 kWh car plugs in at 18:00 with 20% and needs 80% by 07:00, on Nord Pool prices and a grid plan with four zones, VAT included.

| How it charges | Cost |
|---|---|
| At once, as a Tesla does when you plug in | 15.42 EUR |
| With the car's own schedule, from 23:00 | 6.27 EUR |
| **With this board, 00:45 to 05:30** | **4.05 EUR** |

The board finds the cheap hours wherever they fall that day, which a fixed schedule can't. The numbers are its scheduler's, run on the published prices.

## What you need

- A Tesla Model 3, Model Y, Cybertruck, or Model S/X from 2021 on.
- An ESP32-S3-DevKitC-1 (N16R8) board, within Bluetooth range of the car and on your Wi-Fi.
- A USB-C cable for your computer, and a USB-C phone charger near the car to power the board.
- A home charger that charges whenever the car asks: no schedule, auto-lock or app approval (OCPP) of its own.
- A price that follows the day-ahead market, often called spot or exchange price, in one of the [26 countries](plans/README.md), which also says how to use a fixed price instead.
- Chrome or Edge on a computer, for the first install.

## Install

1. Plug the board's USB-C port labelled **COM** into your computer.
2. Download [the firmware](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest/download/esphome-tesla-ble-scheduler.bin), open [ESPHome Web](https://web.esphome.io) in Chrome or Edge, and install it as the video shows:

https://github.com/user-attachments/assets/47da88c6-9992-4ed4-9c16-b7fc7fbd02fe

3. On the board's page, which ESPHome Web opens, choose your country, grid plan and contract, then scan the QR code with your phone.
4. Plug the board into the phone charger near the car, sit in the car with your key card, and finish the setup on your phone:

https://github.com/user-attachments/assets/41fc06e6-78c6-4639-b068-f61f561c2c50

From then on, just plug in: the car is charged by **Ready by**. New releases show up on the board's page.

## Learn more

- [User guide](docs/guide.md): the page, phone messages and troubleshooting.
- [Statuses](docs/status.md): what the page's status says, and what to do about it.
- [Countries and plans](plans/README.md): the markets, the grid plans, and how to add yours.
- [Contributing](CONTRIBUTING.md): the code, its checks and releases.

## Credits and license

The Bluetooth link to the car is [esphome-tesla-ble](https://github.com/yoziru/esphome-tesla-ble), which implements Tesla's [vehicle-command](https://github.com/teslamotors/vehicle-command) protocol. Prices come from Nord Pool's data portal, SMARD (Bundesnetzagentur | SMARD.de, CC BY 4.0), OMIE (OMI-Polo Español, S.A.), OKTE (OKTE, a.s.) and SEMOpx (the market of Ireland and Northern Ireland). This project isn't affiliated with Tesla, Nord Pool, the Bundesnetzagentur, OMIE, OKTE, SEMOpx or any grid operator. Use it at your own risk.

Licensed under the [GNU Affero General Public License v3.0](LICENSE).
