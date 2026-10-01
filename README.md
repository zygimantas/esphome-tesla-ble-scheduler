# ESPHome Tesla BLE Scheduler

## What problem it solves

Nord Pool electricity changes price every quarter-hour, and the cheapest hours of a night often cost a fraction of the evening peak. A Tesla can't follow that: its charging schedule works with times, not prices. Tools that can follow prices usually need a cloud service, your Tesla account, Home Assistant or a new charger.

ESPHome Tesla BLE Scheduler is a small ESP32 board that sits next to the car. When you plug in, it picks the cheapest quarter-hours that still reach your charge limit by the time you leave, counting grid fees and VAT. Then it starts and stops charging over Bluetooth. Everything runs at home: no cloud, no Tesla account, no subscription. Its key can only charge, so even a stolen board can't unlock or drive the car.

## What you need

- A Tesla Model 3, Model Y, Cybertruck, or Model S/X from 2021 on.
- An ESP32-S3-DevKitC-1 (N16R8) board, a USB-C cable and a USB phone charger. The board goes within Bluetooth range of the car and needs your Wi-Fi.
- A home charger that charges whenever the car asks: no schedule, auto-lock or app approval (OCPP) on the charger itself.
- Electricity priced by the Nord Pool day-ahead market, such as in the Baltics or the Nordics.
- A computer to install and update the board.

## Setup

1. **Install ESPHome**: on a Mac with [Homebrew](https://brew.sh), `brew install esphome`; on Windows or Linux, install [Git](https://git-scm.com) and then ESPHome as its [install guide](https://esphome.io/guides/installing_esphome) says.
2. **Download the settings files**: download `config.example.yaml` and `secrets.example.yaml` from the [latest release](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest) into a new folder.
3. **Fill in `secrets.yaml`**: copy `secrets.example.yaml` to `secrets.yaml` and enter your Wi-Fi name and password, any password for the board's backup Wi-Fi, and a random key from [ESPHome's API page](https://esphome.io/components/api/).
4. **Fill in `config.yaml`**: copy `config.example.yaml` to `config.yaml` and enter your [settings](#settings).
5. **Install it on the board**: connect the board's USB-C port labelled **COM** (**UART** on some boards) to the computer, open a terminal in that folder, run `esphome run config.yaml` and choose the board's USB port. The first time takes a while. If it can't connect, hold **BOOT**, press and release **RESET**, release **BOOT**, and try again.
6. **Put the board next to the car** on the USB charger, and give it a minute to join your Wi-Fi.
7. **Pair it with the car**: sit in the car, open http://tesla.local on your phone (type the `http://`: phones try https on their own, which the board doesn't speak), open **Board**, press **Pair BLE key**, tap your key card on the console and confirm on the car's screen.
8. **Turn off charging schedules for home** in the Tesla app or on the car's screen.

## Settings

Your settings in `config.yaml`:

| Setting | What it is |
|---|---|
| `grid` | Your grid fees (see [Grid fees](#grid-fees)). |
| `nordpool: area` | Your Nord Pool price area, such as `LT`, `LV`, `EE`, `FI`, `SE3`, `NO1` or `DK1`. |
| `nordpool: vat` | The VAT added to Nord Pool prices: `0.21` is 21%. |
| `ntfy_server` | The [ntfy](https://ntfy.sh) server for phone messages. Keep `https://ntfy.sh` unless you run your own. |
| `ntfy_topic` | Your ntfy topic (see [Phone messages](#phone-messages)), or empty for no messages. |
| `tesla_battery_kwh` | The car's usable battery in kWh: about `75` for a Long Range, `60` for a Standard Range. |
| `tesla_charging_kw` | The power the Tesla app shows while charging at home: `11` on three-phase 16 A, `7.4` on single-phase 32 A. |
| `tesla_vin` | Your car's VIN, on the car's screen under **Controls** → **Software**. |
| `version` | The release the board runs, like `v0.1.0`. |

After a change, run `esphome run config.yaml` again and choose the board's network address: it updates over Wi-Fi. If a setting is wrong, ESPHome stops and says what.

**To update**, set `version` to the latest release and do the same. The release notes say if anything else needs changing.

## Using it

Open http://tesla.local on your phone. On an iPhone, **Share** → **Add to Home Screen** turns it into an app. There's no password: anyone on your Wi-Fi can use it.

- **When you plug in**, the board plans by itself: the cheapest quarter-hours to reach the car's charge limit by **Ready by**. The plan lists each window with its price, like `02:00 - 02:45 +1` at `19.6 ct/kWh`, where `+1` means tomorrow. A faded window is a spare, used only if charging runs slow. If Ready by is later than the published prices, the status says **Waiting for prices** until they're out.
- **To change the plan**, press **Delete charging plan**, pick **Charge limit** and **Ready by**, then **Create charging plan**. Ready by offers only times with published prices: tomorrow's come out around 13:00 CET. The time you pick becomes your daily Ready by.
- **Start charging now** charges to the limit at any price, until you unplug. **Stop charging** waits until you create a plan, start charging or plug in again.
- **Charging started from the car or the Tesla app** goes ahead: the board leaves it alone until you unplug.
- **Stopping from the car or the Tesla app** lasts only until the board charges again: use **Stop charging** here instead.
- **To let the car charge on its own**, unplug the board. Without prices or a battery level, the car also charges as usual.

### Phone messages

Install the ntfy app, subscribe to a topic with a long random name, and put that name in `ntfy_topic`: anyone who knows it can read the messages. About two minutes after you plug in, your phone gets the plan, like `56 to 80% by Thu 06:30; avg 9.6 ct/kWh over 3 window(s)`.

## Grid fees

The `grid:` part of `config.yaml` adds your grid fees to the Nord Pool prices, so the board compares what you really pay. It comes with ESO's 2026 Standartinis plan with four zones (Lithuania):

- `fees`: the fee per kWh, with VAT, for each zone letter.
- `hours`: the zone of each hour from 00:00 to 23:00, one letter per hour, on a `workday`, a `weekend` day and a public `holiday`.
- `holidays`: public holidays as `MM-DD`, or days around Easter as `easter` and `easter+1`.
- `clock: winter` (optional): the zone hours stay on winter time all year.

ESO's 2026 household plans, in EUR/kWh with VAT. One zone has the same letter every hour. Two zones have `clock: winter`, `workday: nnnnnnnddddddddddddddddn`, `weekend: nnnnnnnnnnnnnnnnnnnnnnnn` and no holidays.

| Plan | One zone | Two zones (`n`, `d`) | Four zones (`n`, `m`, `d`, `e`) |
|---|---|---|---|
| Standartinis | 0.11132 | 0.07139, 0.12947 | 0.06292, 0.08349, 0.10406, 0.14641 |
| Efektyvus | 0.08833 | 0.05687, 0.10164 | 0.05082, 0.06534, 0.08228, 0.11374 |
| Namai | 0.09559 | 0.06171, 0.11011 | - |

## Troubleshooting

- **The Tesla app says "Charging equipment not ready"**: the charger isn't supplying power. Turn off its own schedule, auto-lock or OCPP approval.
- **The page doesn't open**: your phone must be on the same Wi-Fi, and the address must start with `http://`, not `https://`. If the board can't join your Wi-Fi, it opens its own network called **tesla**: join it with your backup Wi-Fi password and enter the new Wi-Fi details.
- **The board can't reach the car**: the **Bluetooth** signal under **Board** is empty or very weak. Move the board closer to the car.
- **The car doesn't charge at night**: check that the board can wake it. Let the car fall asleep, open http://tesla.local/?full and press **Wake up**.
- **The plan's times are an hour or two off**: the board takes its time zone from the computer that installed it. Install from a computer set to your time zone.
- **ESPHome warns about the web_server OTA platform**: that's expected. The page accepts updates only on the board's backup Wi-Fi.

## Development

See [CONTRIBUTING.md](CONTRIBUTING.md).

## Credits and license

The Bluetooth link to the car is [esphome-tesla-ble](https://github.com/yoziru/esphome-tesla-ble), which implements Tesla's [vehicle-command](https://github.com/teslamotors/vehicle-command) protocol. Prices come from Nord Pool's data portal. This project isn't affiliated with Tesla, Nord Pool or any grid operator. Use it at your own risk.

Licensed under the GNU Affero General Public License v3.0; see [LICENSE](LICENSE).
