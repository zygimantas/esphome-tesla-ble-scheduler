# Statuses

What the **Status** row on the page can say, what it means, and what to do. A phone message says the same when there is no schedule to report.

## Before it knows the car

- **Starting up**: the board has just started and has no time yet. Wait a few seconds. If it stays, the board can't reach the internet, which it needs for the time and the prices: check the Wi-Fi.
- **Checking the car**: the charge port is open, but the car hasn't said whether it's plugged in. The board wakes it, every 10 minutes for half an hour. Wait.
- **Waiting for car**: the board hasn't heard from the car since it started. Pair the key if you haven't yet, as in step 6 of the README's [Setup](../README.md#setup), or wait for the car to wake up.
- **Reading battery**: the car is plugged in but hasn't reported its battery level. The board wakes it, every 10 minutes for half an hour. Wait.
- **Getting prices**: the board downloads the prices, for up to 10 minutes after a start. Wait.

## The schedule

- **Charges at 01:30**, or **Charges at Mon 01:30** when it's more than a day away: the schedule's next window. The schedule's windows are listed below the status.
- **Starting**: the schedule wants to charge, or you pressed Start charging now, and the board has told the car; it says Charging, or Charging now, once the car reports that it does, usually within a minute.
- **Charging**: the car charges, as scheduled.
- **Can't start charging**: the car ignored three starts in a quarter-hour. The board tries again in the next one. If it keeps saying this, see the Troubleshooting section of the README.
- **Charger has no power**: the car says its charger gives no power. Turn off the charger's own schedule, auto-lock or OCPP approval. The board has asked the car to charge, so it starts as soon as power comes.
- **Charged**: the battery is at the charge limit.
- **Waiting for prices**: Ready by is later than the published prices, which come out around 13:00 CET for the next day. The schedule is made as soon as they're out.
- **Waiting**: no time is left before Ready by, so there's nothing to schedule. Pick a later Ready by.

## Charging without a schedule

- **Charging now**: you pressed Start charging now, or charging was started from the car or the Tesla app. The car charges at any price until you unplug.
- **No schedule**: you pressed Stop charging or Delete schedule. Press Create schedule or Start charging now, or plug in again.
- **Charging (no prices)**: the board has no price for this quarter-hour, because the download failed for the 10 minutes after a start or the prices ran out later, so the car charges as usual. Check that the board has internet, and `market: area` in your settings.
- **Charging (battery unknown)**: the car didn't report its battery level in half an hour, so it charges as usual. Check the Bluetooth signal under Board.

## The board and the page

- **Unplugged**: the car isn't plugged in.
- **No settings yet**: the board has no settings yet. Fill in Setup on the page and press Save, as the README's [Setup](../README.md#setup) says.
- **Settings: …**: the settings the board had no longer pass its checks, after an update made one stricter. The page shows them in its form: fix what it says and press Save.
- **Tesla entities not found**: the Tesla part of the firmware is missing. Install it again with ESPHome Web, as the README's [Setup](../README.md#setup) says.
- **Connecting …**: the page has just opened and waits for the board.
- **No connection**: the page lost the board. Check that the phone is on the same Wi-Fi and the board has power; the page reconnects by itself.
