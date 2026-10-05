# Countries and plans

The board follows market prices where Nord Pool, SMARD (Germany's Federal Network Agency) or OMIE (the market of Spain and Portugal) publishes the day-ahead prices of your area, and a fixed price anywhere. Your grid fees are a price per kWh that may change with the hour, or there are none. The board applies EU summer time, which every country here uses.

The `tariff:` part of your settings file is what comes on top of the market price, usually your grid fees, so the board compares what you really pay. Your supplier's own price per kWh on top of the market price goes in `market: margin` instead. Most people only name their grid operator's plan, which their country's page in the table below lists:

```yaml
market:
  area: LT
  vat: 0.21
tariff:
  plan: lt/eso-standartinis-4-zones
```

The board starts with the plan from the release you installed, and downloads the current one every day, so new prices reach it without a reinstall. Each plan has a maintainer who keeps it up to date, and each country's folder has a page that says which plans there are and for whom.

## Countries

| Country | `market: area` | `currency` | Plans and notes |
| --- | --- | --- | --- |
| Austria | `AT` | `EUR` | [The network charges of each area](at). |
| Belgium | `BE` | `EUR` | [Fluvius, ORES, RESA and Sibelga's plans](be). Flanders' capacity tariff isn't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Bulgaria | `BG` | `EUR` | [The three big operators' plans](bg). |
| Croatia | `HR` | `EUR` | [HEP ODS's plans](hr). |
| Czechia | `CZ` | `EUR` | Prices from SMARD, in euros only: give your grid fees in euros too. |
| Denmark | `DK1` (west), `DK2` (east) | `DKK` or `EUR` | [The eight big grid companies' plans](dk). |
| Estonia | `EE` | `EUR` | [Elektrilevi's plans](ee). |
| Finland | `FI` | `EUR` | [The eight biggest network companies' plans](fi), without Helen Sähköverkko Aikasiirto's charge per kW ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| France | `FR` | `EUR` | Off-peak hours are set for each address: [write your own plan](fr) with them. |
| Germany | `DE` | `EUR` | [The big grid operators' plans](de), Modul 3 included. |
| Hungary | `HU` | `EUR` | Prices from SMARD, in euros only: give your grid fees in euros too. |
| Italy, the north | `IT-NORTH` | `EUR` | Prices from SMARD, for the north's price area only. [The TD tariff](it). |
| Latvia | `LV` | `EUR` | [Sadales tīkls' plans](lv). |
| Lithuania | `LT` | `EUR` | [ESO's plans](lt). |
| Luxembourg | `LU` | `EUR` | [Luxembourg's network tariff](lu), without the supplement above the reference power ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Netherlands | `NL` | `EUR` | No grid fee per kWh: [no plan needed](nl). |
| Norway | `NO1` to `NO5` | `NOK` or `EUR` | [The biggest network companies' plans](no), without the capacity step ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Poland | `PL` | `PLN` or `EUR` | [The five big operators' plans](pl). |
| Portugal | `PT` | `EUR` | Prices from OMIE. [The access tariffs](pt), on the mainland. |
| Romania | `RO` | `RON` or `EUR` | [The four distribution operators' plans](ro). |
| Slovenia | `SI` | `EUR` | Prices from SMARD. [The network charge](si), without its charge per kW ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Spain | `ES` | `EUR` | Prices from OMIE. [The 2.0TD tolls and charges](es). |
| Sweden | `SE1` to `SE4` | `SEK` or `EUR` | [The big network companies' plans](se). Power fees at some aren't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Switzerland | `CH` | `EUR` | Prices from SMARD, in euros only: give your grid fees in euros too. |

Your electricity contract or your supplier's price list names your area. Prices are in your area's own currency unless you set `currency`, or in euros where they come from SMARD or OMIE, and your grid fees are in the same.

## Grid fees

The board adds VAT to the market price, and a grid fee that can change every quarter-hour: rates for each day of the week, other rates in some months, and their own on public holidays and other dates. A plan holds its grid operator's fee per kWh, with VAT. Taxes and charges that are the same in every hour don't change which hours are cheapest, so plans leave them out, and each country's page says which. Left out, they're missing from every price the board shows, the schedule's windows, the phone message and what Savings says you paid; added to every rate's price, those show your full price.

Some fees depend on your highest power instead, like Norway's capacity step. Until the board counts them ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)), set the car's charging current low enough for your step, in the car or the Tesla app, and `tesla_charging_kw` to match.

## Your own changes

Next to `plan:`, these change the plan for you alone:

- `rates`: your own price for a rate, or a rate the plan doesn't have.
- `exceptions`: a date of your own, or another line for one of the plan's.
- `calendar` and `clock`: your own calendar or clock, instead of the plan's whole one.

```yaml
tariff:
  plan: lt/eso-standartinis-4-zones
  exceptions:
    12-31: night 07:00 day 22:00 night
  rates:
    night: 0.05
```

A rate you set has to be one your days use. If a plan later drops or renames it, the board keeps the plan it has and says why in its log.

## Your own plan

Without your grid operator's plan here, write the plan yourself, or [add the plan](../CONTRIBUTING.md#plans) for everyone on it:

- `calendar`: the months, like `jan-dec`, each with the days of the week, like `mon-fri` and `sat-sun`. Each month and each day of the week is named once. Ranges may run past the end of the year or the week, like `nov-mar` and `fri-mon`.
- A day's line: the rate from midnight, then each time it changes, on a quarter-hour, and the rate from then: `night 07:00 day 23:00 night`. One rate alone is the whole day.
- `exceptions`: public holidays and other dates that differ, like `12-25`, each with a line for the whole day. On those dates they replace the calendar.
- `rates`: each rate's price per kWh, with VAT, in your `currency`. A rate's name is a word like `night` or `p1`. YAML reads `on`, `off`, `yes`, `no`, `true`, `false` and `null` as true, false or nothing, so those can't be names.
- `clock: winter`: all times stay on winter time all year. `clock: local`, the default, follows the clock, summer time included; it undoes a plan's `clock: winter`.

The plans are examples too, like [one with four zones](lt/eso-standartinis-4-zones.yaml).

A fee that's dearer from November to March, Monday to Saturday from 07:00 to 22:00, like Finland's seasonal grid fee:

```yaml
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

If something is wrong, the board keeps the settings it has and says what when you upload them: a rate without a price, a rate of yours no day uses, a month or a day named twice or not at all, a time that isn't a later quarter-hour, a date that doesn't exist, more than 26 rates, or a plan in another currency than yours.

## A fixed price

With a fixed price, or one that changes only with the hour of the day, leave `market:` out: no market prices are downloaded. It works in any country whose clocks change on the EU's dates, in any currency, like `currency: GBP`.

With your grid operator's plan, add your supplier's own part, without the grid fees, as `fixed_price`, which the board adds to the plan's fees in every hour. Where your supplier quotes one price with the grid fees in, it's the supplier's own line on the bill:

```yaml
fixed_price: 0.15
tariff:
  plan: lt/eso-standartinis-2-zones
```

With rates of your own, each rate's price is your whole price per kWh, the supplier's price and the grid fee together, with VAT. One price for every hour:

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
currency: GBP
tariff:
  calendar:
    jan-dec:
      mon-sun: standard 00:30 cheap 05:30 standard
  rates:
    standard: 0.245
    cheap: 0.085
```

## Not supported yet

| Where | Why | Issue |
| --- | --- | --- |
| The rest of Italy, Slovakia, Greece, Ireland and other countries outside Nord Pool, SMARD and OMIE | Their day-ahead prices are on other exchanges. ENTSO-E publishes them all. A fixed price works already, in Italy with [the TD tariff](it). | [#80](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/80) |
| United Kingdom | Neither Nord Pool nor ENTSO-E has British prices. Suppliers like Octopus Agile publish their own. A fixed price works already, also with a night rate on the half-hour like Octopus Go's ([A fixed price](#a-fixed-price)). | [#81](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/81) |

The page and these documents are in English.
