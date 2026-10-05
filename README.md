# ESPHome Tesla BLE Scheduler

## What problem it solves

Nord Pool electricity changes price every quarter-hour, and the cheapest hours of a night often cost a fraction of the evening peak. A Tesla can't follow that: its charging schedule works with times, not prices. Tools that can follow prices usually need a cloud service, your Tesla account, Home Assistant or a new charger.

ESPHome Tesla BLE Scheduler is a small ESP32 board that sits next to the car. When you plug in, it picks the cheapest quarter-hours that still reach your charge limit by the time you leave, counting grid fees and VAT. Then it starts and stops charging over Bluetooth. Everything runs at home: no cloud, no Tesla account, no subscription. Its key can only charge, so even a stolen board can't unlock or drive the car.

## Example savings

An ordinary day: Sunday 27 September 2026, when Nord Pool's Lithuanian prices averaged 0.104 EUR/kWh before VAT. A 75 kWh Tesla comes home at 18:00 with 20% and must have 80% by 07:00: 45 kWh into the battery, 50 kWh from the grid at 11 kW, on a grid plan with four zones, VAT included.

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
- Electricity priced by the day-ahead market, which contracts often call the exchange or spot price, in Austria, Belgium, Bulgaria, Croatia, Czechia, Denmark, Estonia, Finland, France, Germany, Hungary, northern Italy, Latvia, Lithuania, Luxembourg, the Netherlands, Norway, Poland, Portugal, Romania, Slovenia, Spain, Sweden or Switzerland. [Countries and plans](plans/README.md) has the details, and how to use a fixed price instead.
- A computer with Chrome or Edge, for the first install. The board updates itself after that.

## Setup

1. **Download the firmware**: [esphome-tesla-ble-scheduler.bin](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest/download/esphome-tesla-ble-scheduler.bin), from the latest release.
2. **Install it on the board**: connect the board's USB-C port labelled **COM** (**UART** on some boards) to the computer, open [ESPHome Web](https://web.esphome.io) in Chrome or Edge, press **Connect**, choose the port with **USB** in its name, like **USB Single Serial** on a Mac, and press **Connect** again. If you're not sure which it is, it's the one that goes away when you unplug the board. Then press **Install** and choose the file. If it can't connect, hold **BOOT**, press and release **RESET**, release **BOOT**, and try again.
3. **Connect it to your Wi-Fi**: once it's installed, press **Configure Wi-Fi**, choose your network and enter its password.
4. **Open the setup on your phone**: press **Visit Device**. The board's page opens with a QR code: scan it with your phone's camera to open the page there. It also lists what to take to the car.
5. **Put the board next to the car**: unplug it from the computer, plug it into the USB charger near the car, and give it a minute to join your Wi-Fi.
6. **Finish the setup in the car**: sit in the car with your key card and the page open on your phone. If it isn't, open http://tesla.local (type the `http://`: browsers try https on their own, which the board doesn't speak).
   - At **Car**, enter your car's VIN, which the board needs to find the car over Bluetooth and talk to it: it's on the car's screen under **Controls** → **Software** and at the bottom of the Tesla app's home screen. Check the battery's size, guessed from your car's model, and the charging power the Tesla app shows while the car charges at home. Press **Continue**: the page checks the VIN, and says what's wrong if anything is.
   - At **Key**, press **Continue**, tap your key card on the console and confirm on the car's screen: once the car answers, the page moves on by itself.
   - At **Prices**, check **Country / Area**, pick your **Grid plan** where the page lists them, or tick **My plan isn't listed**, choose your **Contract type**, dynamic or fixed, as your contract says, with its margin or fixed price per kWh if you like, and press **Finish**. The board restarts with the settings, or says what's wrong.
7. **Turn off charging schedules for home** in the Tesla app or on the car's screen.

A new board starts with your country's VAT on electricity and your phone's time zone. To change them, or to get phone messages, open **Board** on the page and press **Change settings**: [Settings](#settings) says what each one is.

The board installs each new release by itself within a day, while the car isn't charging. **Board** on the page shows the version it runs.

### From 3.x

Boards on 3.x don't update themselves. Install the latest release once with steps 1 to 3, and don't erase the board when ESPHome Web asks, so it keeps the car's key and its savings. Then go through the setup as in steps 4 to 6 and, under **Change settings**, enter the rest of the ones in your `config.yaml`. With rates of your own, tick **My plan isn't listed** in the setup, then upload a settings file with them under **Change settings**, as [Settings](#settings) says. If Home Assistant had the board, delete it there and add it again within 15 minutes of plugging the board in: Home Assistant then gives it a new key, as the old one stayed with the old firmware.

## Settings

The setup asks only for what the board can't guess. To change any of the settings most people need later, open **Board** and press **Change settings**. They're a file on the board: **Download settings** and **Upload settings**, under the form, give it to you and take it back, for what the form doesn't show: rates of your own (see [Grid plan](#grid-plan)), a currency of your choice or your own ntfy server. The file is YAML, as in [settings.example.yaml](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest/download/settings.example.yaml): two spaces before the settings under `market:` and `tariff:`, and `#` before a comment.

| Setting | What it is |
|---|---|
| `currency` | The currency of all prices: your market area's own unless you set it, like `EUR` or `NOK`, otherwise euro. Without a market, any currency. |
| `fixed_price` | Your supplier's own part of a fixed price per kWh, with VAT and without the grid fees, like `0.12`, without `market:`. The board adds it to your grid plan's fees in every hour, so the costs it shows are complete. Where your supplier quotes one price with the grid fees in, take the supplier's own line on the bill. Leave it out with a monthly average, or with rates of your own that are your whole price. |
| `market: area` | Where you buy electricity: your country's code, or your price area where the country has several, like `LT` or `SE3`: [Countries and plans](plans/README.md#countries) lists them. Leave `market:` out with a fixed price. |
| `market: margin` | Your supplier's own price per kWh on top of the market price, with VAT, like `0.012`. Leave it out if there's none. |
| `market: vat` | The VAT added to the market prices: `0.21` is 21%. |
| `ntfy_server` | The [ntfy](https://ntfy.sh) server for phone messages. Keep `https://ntfy.sh` unless you run your own. |
| `ntfy_topic` | Your ntfy topic (see [Phone messages](#phone-messages)), or empty for no messages. |
| `tariff` | What comes on top of the market price: your grid operator's plan, or your own rates (see [Grid plan](#grid-plan)). Leave it out if your grid fees don't change with the hour. |
| `tesla_battery_kwh` | The car's usable battery in kWh: about `75` for a Long Range, `60` for a Standard Range. |
| `tesla_charging_kw` | The power the Tesla app shows while charging at home: `11` on three-phase 16 A, `7.4` on single-phase 32 A. |
| `tesla_vin` | Your car's VIN, 17 capital letters and digits, on the car's screen under **Controls** → **Software** and at the bottom of the Tesla app's home screen. |
| `timezone` | The time zone the car lives in, like `Europe/Vilnius`, `Europe/Helsinki` or `Europe/Oslo`. |

Every price you write, here and in `tariff:`, is per kWh with VAT, as on your bill. The board adds `market: vat` only to the market prices it downloads.

If a setting is wrong, the board keeps the ones it has and says what.

## Using it

Open http://tesla.local on your phone. On an iPhone, **Share** → **Add to Home Screen** turns it into an app. There's no password: anyone on your Wi-Fi can use it and see your settings.

- **When you plug in**, the board makes a schedule by itself: the cheapest quarter-hours to reach the car's charge limit by **Ready by**. The schedule lists each window with its price, like `02:00 - 02:45 +1` at `0.196 EUR/kWh`, where `+1` means tomorrow. A faded window is a spare, used only if charging runs slow. If Ready by is later than the published prices, the board waits for them, and charges now only what the hours after them can't fit.
- **To change the schedule**, press **Delete schedule**, pick **Charge limit** and **Ready by**, then **Create schedule**. Ready by offers only times with published prices: tomorrow's come out around 13:00 CET. The time you pick becomes your daily Ready by.
- **Start charging now** charges to the limit at any price, until you unplug. **Stop charging** waits until you create a schedule, start charging or plug in again.
- **Charging started from the car or the Tesla app** goes ahead: the board leaves it alone until you unplug.
- **Stopping from the car or the Tesla app** lasts only until the board charges again: use **Stop charging** here instead.
- **To let the car charge on its own**, unplug the board. Without prices or a battery level, the car also charges as usual.
- **Savings** shows what charging saved against the day's average price in the last 30 days and the last 12 months, and underneath, against plugging in and charging at once. Prices count VAT, grid fees and the supplier's price per kWh in `market: margin`, not monthly fees. **Reset savings** starts again from zero.
- **To start over**, install the firmware again with steps 1 to 3 of [Setup](#setup), and let ESPHome Web erase the board when it asks: it forgets its settings, Wi-Fi, the car's key and savings. Then set it up again, and remove its old key in the car under **Controls** → **Locks**.

### Phone messages

Phone messages are optional, through the ntfy app: install it, subscribe to a topic with a long random name, and enter that name as **ntfy topic** under **Change settings**. Anyone who knows it can read the messages. About two minutes after you plug in, or once tomorrow's prices are out if Ready by is later than the published ones, your phone gets the schedule, like `56 to 80% by Thu 06:30; avg 0.096 EUR/kWh over 3 window(s)`. Tap the message to open the page. You also get a message when the board has to let the car charge at any price, for lack of prices or of a battery level or because charging was started from the car or the Tesla app, and when Ready by passes with the car short of its limit.

## Grid plan

The `tariff:` part of your settings is what comes on top of the market price, usually your grid fees, so the board compares what you really pay. It names your grid operator's plan, like `lt/eso-standartinis-4-zones` in the example. Your supplier usually bills the grid operator's fee unchanged, and its bill names your plan. Without one, the board picks the hours by the market price alone, and the page says so. The board downloads the current plan every day, so new prices reach it without a reinstall. [Countries and plans](plans/README.md) lists the plans for each country, and explains how to change a plan's prices for yourself, or to write your own for another grid operator.

With a fixed price, or one that follows the monthly average, choose **Fixed** under **Contract type**: the board leaves `market:` out, downloads no market prices and charges by your grid plan's zones. Enter your supplier's own part, without the grid fees, under **Supplier's part**, so what the savings card says charging cost is complete. Where your supplier quotes one price with them in, it's the supplier's own line on the bill: the whole price would count the grid fees twice. With a monthly average, leave it at 0.00: the cost is then only the grid's part, while what it saved is right. Without a plan, every hour costs the same, so the board charges at once, and the page says so. With day and night prices, from your grid operator or your supplier, write your own rates in your settings file and upload it under **Change settings**, as [A fixed price](plans/README.md#a-fixed-price) shows. With rates of your own instead of a plan, each rate's price in `tariff:` becomes your whole price per kWh with VAT, the supplier's price included. With one rate for every hour, all hours cost the same, so the board charges at once.

## Troubleshooting

[Statuses](docs/status.md) explains everything the Status row can say and what to do about it.

- **The Tesla app says "Charging equipment not ready"**, or the page says **Charger has no power**: the charger isn't supplying power. Turn off its own schedule, auto-lock or OCPP approval.
- **The page doesn't open**: your phone must be on the same Wi-Fi, and the address must start with `http://`, not `https://`. If the board can't join your Wi-Fi, it opens its own network called **tesla** for 15 minutes after you plug it in: join it from your phone and choose your Wi-Fi. Unplug the board and plug it in again for another 15 minutes.
- **The board can't reach the car**: the **Bluetooth** signal under **Board** is empty or very weak. Move the board closer to the car.
- **The car doesn't charge at night**: check that the board can wake it. Let the car fall asleep, open http://tesla.local/?full and press **Wake up**.
- **The schedule's times are an hour or two off**: check `timezone` in your settings.

## Development

See [CONTRIBUTING.md](CONTRIBUTING.md).

## Credits and license

The Bluetooth link to the car is [esphome-tesla-ble](https://github.com/yoziru/esphome-tesla-ble), which implements Tesla's [vehicle-command](https://github.com/teslamotors/vehicle-command) protocol. Prices come from Nord Pool's data portal, from SMARD (Bundesnetzagentur | SMARD.de, CC BY 4.0) and from OMIE (OMI-Polo Español, S.A.). This project isn't affiliated with Tesla, Nord Pool, the Bundesnetzagentur, OMIE or any grid operator. Use it at your own risk.

Licensed under the GNU Affero General Public License v3.0; see [LICENSE](LICENSE).
