# User guide

Using the board once it's set up, as the [README](../README.md#install) says.

## Using it

Open the page on your phone, from the home screen once you've added it there, as the end of the setup says. It's at the board's address on your Wi-Fi, like http://192.168.1.23, which the QR code opened. If the router gives the board a new one, open http://tesla.local, which some Android phones don't find, or tap a phone message, which has the new one. There's no password: anyone on your Wi-Fi can use it and see your settings.

- **When you plug in**, the board makes a schedule by itself: the cheapest quarter-hours to reach the car's charge limit by **Ready by**. The schedule lists each window with its price, like `Thu 02:00 - 02:45` at `0.196 EUR/kWh`, or `Thu 23:15 - Fri 01:00` past midnight. A faded window is a spare, used only if charging runs slow. If Ready by is later than the published prices, the board waits for them, and charges now only what the hours after them can't fit.
- **To change the schedule**, press **Delete schedule**, pick **Charge limit** and **Ready by**, then **Create schedule**. Ready by offers only times with published prices: tomorrow's come out around 13:00 CET. The time you pick becomes your daily Ready by.
- **Start charging now** charges to the limit at any price, until you unplug. **Stop charging** waits until you create a schedule, start charging or plug in again.
- **Charging started from the car or the Tesla app** goes ahead: the board leaves it alone until you unplug.
- **Stopping from the car or the Tesla app** lasts only until the board charges again: use **Stop charging** here instead.
- **To let the car charge on its own**, unplug the board. Without prices or a battery level, the car also charges as usual.
- **To unlock the cable when charging finishes**, tick **Unlock the charge port when charged** under **Settings**: once the car reaches its charge limit, the board unlocks the charge port, as Unlock in the Tesla app does, so the cable comes out without the car's key.
- **Savings** shows what charging saved against the day's average price in the last 30 days and the last 12 months. The **?** after each shows the energy and what it cost, and the saving against plugging in and charging at once. Prices count VAT, grid fees and the supplier's margin or part per kWh, not monthly fees. **Reset savings** starts again from zero.
- **To start over**, as for another car, press **Restart setup** under **Settings**: the board forgets its settings, its key and Ready by, keeps its Wi-Fi and the savings, and starts the setup again. Remove its old key in the car under **Controls** → **Locks**.

## Phone messages

Phone messages are optional, through the ntfy app: install it, open **Settings** on the board's page and press the button at the end of the **Ntfy topic** field, which makes a long random topic and copies it, then subscribe to it in the app by pasting it. **Send test message** checks that the app gets one; then press **Save**. Anyone who knows the topic can read the messages. About two minutes after you plug in, or once tomorrow's prices are out if Ready by is later than the published ones, your phone gets the schedule, like `56 to 80% by Thu 06:30; avg 0.096 EUR/kWh over 3 window(s)`. Tap the message to open the page. You also get a message when the board has to let the car charge at any price, for lack of prices or of a battery level or because charging was started from the car or the Tesla app, and when Ready by passes with the car short of its limit.

## Troubleshooting

[Statuses](status.md) explains everything the Status row can say and what to do about it.

- **The Tesla app says "Charging equipment not ready"**, or the page says **Charger has no power**: the charger isn't supplying power. Turn off its own schedule, auto-lock or OCPP approval.
- **The page doesn't open**: your phone must be on the same Wi-Fi, and the address must start with `http://`, not `https://`. If the board can't join your Wi-Fi, it opens its own network called **tesla** for 15 minutes after you plug it in: join it from your phone and choose your Wi-Fi. Unplug the board and plug it in again for another 15 minutes.
- **The board can't reach the car**: the Bluetooth signal at the top of the page is empty or very weak. Move the board closer to the car.
- **The car doesn't charge at night**: check that the board can wake it. Let the car fall asleep, open http://tesla.local/?full and press **Wake up**.
- **The schedule's times are an hour or two off**: check **Country / Area** under **Settings**, which sets the board's time zone.
