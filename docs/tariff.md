# Tariff

The `tariff:` part of `config.yaml` is what comes on top of the market price, usually your grid fees, so the board compares what you really pay. Most people only name their grid operator's plan:

```yaml
scheduler:
  market:
    area: LT
    vat: 0.21
  tariff:
    plan: lt/eso-standartinis-4-zones
```

The board starts with the plan from the release you installed, and downloads the current one every day, so new prices reach it without a reinstall. Each plan has a maintainer from its country who keeps it up to date. [Plans](#plans) says which there are.

## Your own changes

Next to `plan:`, these change the plan for you alone:

- `rates`: your own price for a rate, or a rate the plan doesn't have.
- `exceptions`: a date of your own, or another line for one of the plan's.
- `calendar` and `clock`: your own calendar or clock, instead of the plan's whole one.

```yaml
scheduler:
  tariff:
    plan: lt/eso-standartinis-4-zones
    exceptions:
      12-31: night 07:00 day 22:00 night
    rates:
      night: 0.05
```

A rate you set has to be one your days use. If a plan later drops or renames it, the board keeps the plan it has and says why in its log.

## Your own tariff

Without your grid operator's plan here, write the tariff yourself, or [add the plan](../CONTRIBUTING.md#plans) for everyone on it:

- `calendar`: the months, like `jan-dec`, each with the days of the week, like `mon-fri` and `sat-sun`. Each month and each day of the week is named once. Ranges may run past the end of the year or the week, like `nov-mar` and `fri-mon`.
- A day's line: the rate from midnight, then each time it changes, on a quarter-hour, and the rate from then: `night 07:00 day 23:00 night`. One rate alone is the whole day.
- `exceptions`: public holidays and other dates that differ, like `12-25`, each with a line for the whole day. On those dates they replace the calendar.
- `rates`: each rate's price per kWh, with VAT, in your `currency`. A rate's name is a word like `night` or `p1`. YAML reads `on`, `off`, `yes`, `no`, `true`, `false` and `null` as true, false or nothing, so those can't be names.
- `clock: winter`: all times stay on winter time all year. `clock: local`, the default, follows the clock, summer time included; it undoes a plan's `clock: winter`.

The plans are examples too, like [ESO's four zones](../plans/lt/eso-standartinis-4-zones.yaml).

A fee that's dearer from November to March, Monday to Saturday from 07:00 to 22:00, like Finland's seasonal grid fee:

```yaml
scheduler:
  tariff:
    calendar:
      apr-oct:
        mon-sun: low
      nov-mar:
        mon-sat: low 07:00 high 22:00 low
        sun: low
    rates:
      low: 0.03
      high: 0.08
```

Holidays that move, like Easter Monday, are this year's dates. Change them every year, or use a plan, whose maintainer does.

If something is wrong, the install stops and says what: a rate without a price, a rate of yours no day uses, a month or a day named twice or not at all, a time that isn't a later quarter-hour, a date that doesn't exist, more than 26 rates, or a plan in another currency than yours.

## A fixed price

Leave `market:` out: no market prices are downloaded, and each rate's price is your whole price per kWh, the supplier's price and the grid fee together, with VAT. One price for every hour:

```yaml
  tariff:
    calendar:
      jan-dec:
        mon-sun: flat
    rates:
      flat: 0.24
```

All hours then cost the same, so the board charges at once. With day and night prices, give each its rate, and the board charges in the cheap one. A night rate that starts on the half-hour, like Octopus Go's from 00:30 to 05:30 in the UK, with your rates:

```yaml
scheduler:
  currency: GBP
  tariff:
    calendar:
      jan-dec:
        mon-sun: standard 00:30 cheap 05:30 standard
    rates:
      standard: 0.245
      cheap: 0.085
```

## Plans

The plans are in [plans](../plans), a folder for each country:

- Lithuania: [ESO plans](eso.md).
