# ESPHome Tesla BLE Scheduler

## What problem it solves

Nord Pool electricity changes price every quarter-hour, and the cheapest hours of a night often cost a fraction of the evening peak. A Tesla can't follow that: its charging schedule works with times, not prices. Tools that can follow prices usually need a cloud service, your Tesla account, Home Assistant or a new charger.

ESPHome Tesla BLE Scheduler is a small ESP32 board that sits next to the car. When you plug in, it picks the cheapest quarter-hours that still reach your charge limit by the time you leave, counting grid fees and VAT. Then it starts and stops charging over Bluetooth. Everything runs at home: no cloud, no Tesla account, no subscription. Its key can only charge, so even a stolen board can't unlock or drive the car.

## Example savings

An ordinary day: Sunday 27 September 2026, when Nord Pool's Lithuanian prices averaged 0.104 EUR/kWh before VAT. A 75 kWh Tesla comes home at 18:00 with 20% and must have 80% by 07:00: 45 kWh into the battery, 50 kWh from the grid at 11 kW, on ESO's Standartinis plan with four zones, VAT included.

| How it charges | Cost |
|---|---|
| Plugged in and left to charge at once, as a Tesla does | 15.42 EUR |
| The car's own schedule, starting at 23:00 | 6.27 EUR |
| The board's schedule, 00:45 to 05:30 | 4.05 EUR |

The board picks the quarter-hours by price, grid fee included, so it finds the cheap hours wherever they fall that day, which a fixed schedule can't. The numbers come from the board's scheduler run on the published prices. On a weekday the evening costs more still, as the evening fee applies.

## Prerequisites

- A Tesla Model 3, Model Y, Cybertruck, or Model S/X from 2021 on.
- An ESP32-S3-DevKitC-1 (N16R8) board. It goes within Bluetooth range of the car and needs your Wi-Fi.
- A USB-C cable with the plug your computer takes, for the setup.
- A USB-C phone charger, with a socket near the car, to power the board.
- A home charger that charges whenever the car asks: no schedule, auto-lock or app approval (OCPP) on the charger itself.
- Electricity priced by the day-ahead market, which contracts often call the exchange or spot price, in Austria, Belgium, Bulgaria, Croatia, Czechia, Denmark, Estonia, Finland, France, Germany, Hungary, northern Italy, Latvia, Lithuania, Luxembourg, the Netherlands, Norway, Poland, Portugal, Romania, Slovenia, Spain, Sweden or Switzerland. [Countries](docs/countries.md) has the details, and how to use a fixed price instead.
- A computer to install and update the board.

## AI assisted setup

A coding agent on your computer, such as Claude Code or Codex, can do the manual setup below for you. Connect the board's USB-C port labelled **COM** (**UART** on some boards) to the computer, start the agent in a new, empty folder, and give it this prompt:

```text
Set up a Tesla charging board for me by following the manual setup in the README of
https://github.com/zygimantas/esphome-tesla-ble-scheduler. The board, an ESP32-S3-DevKitC-1, is
connected to this computer by USB, and this folder is for its files. Install what is missing first.
Download the two files the README names from the latest release, then ask me in one message for every
setting and secret they need, except the API key, which you generate yourself. Fill in the files
without ever showing my Wi-Fi password, the key, the VIN or the ntfy topic back to me. Build and
install the firmware with esphome run config.yaml --device <the board's USB port> --no-logs, since
the port prompt and the log stream of a plain esphome run never return, and wait until the board
answers at http://tesla.local. Then tell me exactly what to do in the car to pair it and what to turn
off in the Tesla app.
```

Answer its questions, then do what it tells you to do at the car.

## Manual setup

1. **Install ESPHome**: on a Mac with [Homebrew](https://brew.sh), `brew install esphome`; on Windows or Linux, install [Git](https://git-scm.com) and then ESPHome as its [install guide](https://esphome.io/guides/installing_esphome) says.
2. **Download the settings files**: download `config.example.yaml` and `secrets.example.yaml` from the [latest release](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest) into a new folder.
3. **Fill in `secrets.yaml`**: copy `secrets.example.yaml` to `secrets.yaml` and enter your Wi-Fi name and password, a password of at least 8 characters for the board's backup Wi-Fi, and a random key from [ESPHome's API page](https://esphome.io/components/api/#:~:text=randomly%20generated%20by%20your%20browser), shown next to **key** under **encryption** with a **Copy** button.
4. **Fill in `config.yaml`**: copy `config.example.yaml` to `config.yaml` and enter your [settings](#settings).
5. **Install it on the board**: connect the board's USB-C port labelled **COM** (**UART** on some boards) to the computer, open a terminal in that folder, run `esphome run config.yaml` and choose the board's USB port. The first time takes a while. If it can't connect, hold **BOOT**, press and release **RESET**, release **BOOT**, and try again.
6. **Put the board next to the car** on the USB charger, and give it a minute to join your Wi-Fi.
7. **Pair it with the car**: sit in the car, open http://tesla.local on your phone (type the `http://`: phones try https on their own, which the board doesn't speak), open **Board**, press **Pair BLE key**, tap your key card on the console and confirm on the car's screen.
8. **Turn off charging schedules for home** in the Tesla app or on the car's screen.

## Settings

Your settings in `config.yaml`:

| Setting | What it is |
|---|---|
| `currency` | The currency of all prices: your market area's own unless you set it, like `EUR` or `NOK`, otherwise euro. Without a market, any currency. |
| `market: area` | Where you buy electricity: your country's code, or your price area where the country has several, like `LT` or `SE3`: [Countries](docs/countries.md) lists them. Leave `market:` out with a fixed price. |
| `market: margin` | Your supplier's own price per kWh on top of the market price, with VAT, like `0.012`. Leave it out if there's none. |
| `market: vat` | The VAT added to the market prices: `0.21` is 21%. |
| `ntfy_server` | The [ntfy](https://ntfy.sh) server for phone messages. Keep `https://ntfy.sh` unless you run your own. |
| `ntfy_topic` | Your ntfy topic (see [Phone messages](#phone-messages)), or empty for no messages. |
| `tariff` | What comes on top of the market price: your grid operator's plan, or your own rates (see [Tariff](#tariff)). |
| `tesla_battery_kwh` | The car's usable battery in kWh: about `75` for a Long Range, `60` for a Standard Range. |
| `tesla_charging_kw` | The power the Tesla app shows while charging at home: `11` on three-phase 16 A, `7.4` on single-phase 32 A. |
| `tesla_vin` | Your car's VIN, 17 capital letters and digits, on the car's screen under **Controls** → **Software**. |
| `timezone` | The time zone the car lives in, like `Europe/Vilnius`, `Europe/Helsinki` or `Europe/Oslo`. |
| `version` | The release the board runs, like `v3.2.0`. |

Every price you write, here and in `tariff:`, is per kWh with VAT, as on your bill. The board adds `market: vat` only to the market prices it downloads.

After a change, run `esphome run config.yaml` again and choose the board's network address: it updates over Wi-Fi. If a setting is wrong, ESPHome stops and says what.

**To update**, set `version` to the latest release and do the same; the **Board** section on the page shows the version the board runs. The release notes say if anything else needs changing.

## Using it

Open http://tesla.local on your phone. On an iPhone, **Share** → **Add to Home Screen** turns it into an app. There's no password: anyone on your Wi-Fi can use it.

- **When you plug in**, the board makes a schedule by itself: the cheapest quarter-hours to reach the car's charge limit by **Ready by**. The schedule lists each window with its price, like `02:00 - 02:45 +1` at `0.196 EUR/kWh`, where `+1` means tomorrow. A faded window is a spare, used only if charging runs slow. If Ready by is later than the published prices, the board waits for them, and charges now only what the hours after them can't fit.
- **To change the schedule**, press **Delete schedule**, pick **Charge limit** and **Ready by**, then **Create schedule**. Ready by offers only times with published prices: tomorrow's come out around 13:00 CET. The time you pick becomes your daily Ready by.
- **Start charging now** charges to the limit at any price, until you unplug. **Stop charging** waits until you create a schedule, start charging or plug in again.
- **Charging started from the car or the Tesla app** goes ahead: the board leaves it alone until you unplug.
- **Stopping from the car or the Tesla app** lasts only until the board charges again: use **Stop charging** here instead.
- **To let the car charge on its own**, unplug the board. Without prices or a battery level, the car also charges as usual.
- **Savings** shows what charging saved against the day's average price in the last 30 days and the last 12 months, and underneath, against plugging in and charging at once. Prices count VAT, grid fees and the supplier's price per kWh in `market: margin`, not monthly fees. **Reset savings** starts again from zero.

### Phone messages

Install the ntfy app, subscribe to a topic with a long random name, and put that name in `ntfy_topic`: anyone who knows it can read the messages. About two minutes after you plug in, or once tomorrow's prices are out if Ready by is later than the published ones, your phone gets the schedule, like `56 to 80% by Thu 06:30; avg 0.096 EUR/kWh over 3 window(s)`. Tap the message to open the page. You also get a message when the board has to let the car charge at any price, for lack of prices or of a battery level or because charging was started from the car or the Tesla app, and when Ready by passes with the car short of its limit.

## Tariff

The `tariff:` part of `config.yaml` is what comes on top of the market price, usually your grid fees, so the board compares what you really pay. It names your grid operator's plan: in the example, `lt/eso-standartinis-4-zones`, ESO's Standartinis plan with four zones (Lithuania). Your supplier, like Ignitis or Enefit, usually bills the grid operator's fee unchanged, and its bill names your plan. The board downloads the current plan every day, so new prices reach it without a reinstall. [ESO plans](docs/eso.md) has ESO's other plans, and [Tariff](docs/tariff.md) explains how to change a plan's prices for yourself, or to write your own for another grid operator.

With a fixed price, leave `market:` out: no market prices are downloaded, and each rate's price in `tariff:` becomes your whole price per kWh with VAT, the supplier's price included. With one rate for every hour, all hours cost the same, so the board charges at once.

## Troubleshooting

[Statuses](docs/status.md) explains everything the Status row can say and what to do about it.

- **The Tesla app says "Charging equipment not ready"**, or the page says **Charger has no power**: the charger isn't supplying power. Turn off its own schedule, auto-lock or OCPP approval.
- **The page doesn't open**: your phone must be on the same Wi-Fi, and the address must start with `http://`, not `https://`. If the board can't join your Wi-Fi, it opens its own network called **tesla**: join it with your backup Wi-Fi password and enter the new Wi-Fi details.
- **The board can't reach the car**: the **Bluetooth** signal under **Board** is empty or very weak. Move the board closer to the car.
- **The car doesn't charge at night**: check that the board can wake it. Let the car fall asleep, open http://tesla.local/?full and press **Wake up**.
- **The schedule's times are an hour or two off**: check `timezone` in `config.yaml`.
- **ESPHome warns about the web_server OTA platform**: that's expected. The page accepts updates only on the board's backup Wi-Fi.

## Development

See [CONTRIBUTING.md](CONTRIBUTING.md).

## Credits and license

The Bluetooth link to the car is [esphome-tesla-ble](https://github.com/yoziru/esphome-tesla-ble), which implements Tesla's [vehicle-command](https://github.com/teslamotors/vehicle-command) protocol. Prices come from Nord Pool's data portal, from SMARD (Bundesnetzagentur | SMARD.de, CC BY 4.0) and from OMIE (OMI-Polo Español, S.A.). This project isn't affiliated with Tesla, Nord Pool, the Bundesnetzagentur, OMIE or any grid operator. Use it at your own risk.

Licensed under the GNU Affero General Public License v3.0; see [LICENSE](LICENSE).
