# Statuses

What the **Status** row on the page can say, what it means, and what to do. A phone message says the same when there is no plan to report.

## Before it knows the car

- **Starting up**: the board has just started and has no time yet. Wait a few seconds.
- **Checking the car**: the charge port is open, but the car hasn't said whether it's plugged in. The board wakes it, every 10 minutes for half an hour. Wait.
- **Waiting for car**: the board hasn't heard from the car since it started. Pair the key if you haven't yet (step 7 of the manual setup), or wait for the car to wake up.
- **Reading battery**: the car is plugged in but hasn't reported its battery level. The board wakes it, every 10 minutes for half an hour. Wait.
- **Getting prices**: the board downloads the prices, for up to 10 minutes after a start. Wait.

## The plan

- **Charges at 01:30**, or **Charges at Mon 01:30** when it's more than a day away: the plan's next window. The plan's windows are listed below the status.
- **Starting**: the plan wants to charge and the board has told the car; it says Charging once the car reports that it does, usually within a minute.
- **Charging**: the car charges, as planned.
- **Can't start charging**: the car ignored three starts in a quarter-hour. The board tries again in the next one. If it keeps saying this, see the Troubleshooting section of the README.
- **Charger has no power**: the car says its charger gives no power. Turn off the charger's own schedule, auto-lock or OCPP approval. The board has asked the car to charge, so it starts as soon as power comes.
- **Charged**: the battery is at the charge limit.
- **Waiting for prices**: Ready by is later than the published prices, which come out around 13:00 CET for the next day. The plan is made as soon as they're out.
- **Waiting**: no time is left before Ready by, so there's nothing to plan. Pick a later Ready by.

## Charging without a plan

- **Charging now**: you pressed Start charging now, or charging was started from the car or the Tesla app. The car charges at any price until you unplug.
- **No plan**: you pressed Stop charging or Delete charging plan. Press Create charging plan or Start charging now, or plug in again.
- **Charging (no prices)**: the board couldn't get prices for 10 minutes, so the car charges as usual. Check that the board has internet, and `nordpool: area` in `config.yaml`.
- **Charging (battery unknown)**: the car didn't report its battery level in half an hour, so it charges as usual. Check the Bluetooth signal under Board.

## The board and the page

- **Unplugged**: the car isn't plugged in.
- **Tesla entities not found**: the Tesla part of the firmware is missing. Install again from a clean folder with the example `config.yaml`.
- **Connecting …**: the page has just opened and waits for the board.
- **No connection**: the page lost the board. Check that the phone is on the same Wi-Fi and the board has power; the page reconnects by itself.
