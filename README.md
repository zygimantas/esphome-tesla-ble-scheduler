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
- A computer with Chrome or Edge, for the first install. After that, the board's page offers each new release.

## Setup

1. **Connect the board**: connect the board's USB-C port labelled **COM** to the computer.
2. **Download the firmware**: [esphome-tesla-ble-scheduler.bin](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest/download/esphome-tesla-ble-scheduler.bin), from the latest release.
3. **Install the firmware**: Open [ESPHome Web](https://web.esphome.io) in Chrome or Edge and follow steps from this video:

https://github.com/user-attachments/assets/47da88c6-9992-4ed4-9c16-b7fc7fbd02fe

4. **Scan QR code with phone**: scan the QR code the page shows with your phone's camera, which opens the final setup steps.
6. **Put the board next to the car**: unplug it from the computer, plug it into the USB charger near the car, and give it a minute to join your Wi-Fi.
7. **Finish the setup in the car**: sit in the car with your key card and the page open on your phone. If you closed it, open it again from the browser's history, or at http://tesla.local, which some Android phones don't find (type the `http://`: browsers try https on their own, which the board doesn't speak).
   - Enter your car's VIN, which the board needs to find the car over Bluetooth and talk to it: it's on the car's screen under **Controls** → **Software** and at the bottom of the Tesla app's home screen. Check the battery's size, guessed from your car's model, and the charging power the Tesla app shows while the car charges at home. Press **Continue**: the page checks the VIN, and says what's wrong if anything is.
   - Then tick **I am in the car with my Tesla key card** and press **Create key**, which says Looking for the car … until the board finds the car over Bluetooth. Tap your key card on the console and confirm on the car's screen while the button says Waiting for the car …; if the car missed it, the button turns to **Try again**. Once the car answers, the page says the setup is done, and how to add it to your home screen.
8. **Turn off charging schedules for home** in the Tesla app or on the car's screen.

To change the country, the grid plan, the contract or the car's battery and charging power, open **Settings**, below **Savings** on the page: the country sets the VAT and the time zone too. Saving them deletes the schedule, as it was made with the old ones: press **Create schedule** for a new one. Saving only a new **Ntfy topic** doesn't. The car's VIN stays as you set it up, until you start over.

When you open the page after a new release, it says so at the top, with the release the board runs and a link to the release notes, and **Update**: the board installs it only when you press it, and restarts with it in about a minute. **Later** hides it until you open the page again. **Settings** on the page shows the version it runs.

## Using it

Open the page on your phone, from the home screen once you've added it there, as the end of the setup says. It's at the board's address on your Wi-Fi, like http://192.168.1.23, which the QR code opened. If the router gives the board a new one, open http://tesla.local, which some Android phones don't find, or tap a phone message, which has the new one. There's no password: anyone on your Wi-Fi can use it and see your settings.

- **When you plug in**, the board makes a schedule by itself: the cheapest quarter-hours to reach the car's charge limit by **Ready by**. The schedule lists each window with its price, like `Thu 02:00 - 02:45` at `0.196 EUR/kWh`, or `Thu 23:15 - Fri 01:00` past midnight. A faded window is a spare, used only if charging runs slow. If Ready by is later than the published prices, the board waits for them, and charges now only what the hours after them can't fit.
- **To change the schedule**, press **Delete schedule**, pick **Charge limit** and **Ready by**, then **Create schedule**. Ready by offers only times with published prices: tomorrow's come out around 13:00 CET. The time you pick becomes your daily Ready by.
- **Start charging now** charges to the limit at any price, until you unplug. **Stop charging** waits until you create a schedule, start charging or plug in again.
- **Charging started from the car or the Tesla app** goes ahead: the board leaves it alone until you unplug.
- **Stopping from the car or the Tesla app** lasts only until the board charges again: use **Stop charging** here instead.
- **To let the car charge on its own**, unplug the board. Without prices or a battery level, the car also charges as usual.
- **Savings** shows what charging saved against the day's average price in the last 30 days and the last 12 months. The **?** after each shows the energy and what it cost, and the saving against plugging in and charging at once. Prices count VAT, grid fees and the supplier's margin or part per kWh, not monthly fees. **Reset savings** starts again from zero.
- **To start over**, as for another car, press **Restart setup** under **Settings**: the board forgets its settings, its key and Ready by, keeps its Wi-Fi and the savings, and starts the setup again. Remove its old key in the car under **Controls** → **Locks**.

### Phone messages

Phone messages are optional, through the ntfy app: install it, open **Settings** on the board's page and press the button at the end of the **Ntfy topic** field, which makes a long random topic and copies it, then subscribe to it in the app by pasting it. **Send test message** checks that the app gets one; then press **Save**. Anyone who knows the topic can read the messages. About two minutes after you plug in, or once tomorrow's prices are out if Ready by is later than the published ones, your phone gets the schedule, like `56 to 80% by Thu 06:30; avg 0.096 EUR/kWh over 3 window(s)`. Tap the message to open the page. You also get a message when the board has to let the car charge at any price, for lack of prices or of a battery level or because charging was started from the car or the Tesla app, and when Ready by passes with the car short of its limit.

## Troubleshooting

[Statuses](docs/status.md) explains everything the Status row can say and what to do about it.

- **The Tesla app says "Charging equipment not ready"**, or the page says **Charger has no power**: the charger isn't supplying power. Turn off its own schedule, auto-lock or OCPP approval.
- **The page doesn't open**: your phone must be on the same Wi-Fi, and the address must start with `http://`, not `https://`. If the board can't join your Wi-Fi, it opens its own network called **tesla** for 15 minutes after you plug it in: join it from your phone and choose your Wi-Fi. Unplug the board and plug it in again for another 15 minutes.
- **The board can't reach the car**: the Bluetooth signal at the top of the page is empty or very weak. Move the board closer to the car.
- **The car doesn't charge at night**: check that the board can wake it. Let the car fall asleep, open http://tesla.local/?full and press **Wake up**.
- **The schedule's times are an hour or two off**: check **Country / Area** under **Settings**, which sets the board's time zone.

## Development

See [CONTRIBUTING.md](CONTRIBUTING.md).

## Credits and license

The Bluetooth link to the car is [esphome-tesla-ble](https://github.com/yoziru/esphome-tesla-ble), which implements Tesla's [vehicle-command](https://github.com/teslamotors/vehicle-command) protocol. Prices come from Nord Pool's data portal, from SMARD (Bundesnetzagentur | SMARD.de, CC BY 4.0) from OMIE (OMI-Polo Español, S.A.), from OKTE (OKTE, a.s.) and from SEMOpx (the market of Ireland and Northern Ireland). This project isn't affiliated with Tesla, Nord Pool, the Bundesnetzagentur, OMIE, OKTE, SEMOpx or any grid operator. Use it at your own risk.

Licensed under the GNU Affero General Public License v3.0; see [LICENSE](LICENSE).
