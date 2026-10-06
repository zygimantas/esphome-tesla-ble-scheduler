# Countries and plans

The board follows market prices where Nord Pool, SMARD (Germany's Federal Network Agency) or OMIE (the market of Spain and Portugal) publishes the day-ahead prices of your area, and a fixed price anywhere. Your grid fees are a price per kWh that may change with the hour, or there are none. The board applies EU summer time, which every country here uses.

Your grid plan is what comes on top of the market price, usually your grid fees, so the board compares what you really pay. Your supplier's own price per kWh on top of the market price is the **Supplier's margin** instead. Most people only choose their grid operator's plan under **Grid plan** on the board's page.

The board starts with the plan from the release you installed, and downloads the current one every day, so new prices reach it without a reinstall. Each plan has a maintainer who keeps it up to date, and each country's folder has a page that says which plans there are and for whom.

## Countries

| Country | Area | Currency | Plans and notes |
| --- | --- | --- | --- |
| Austria | `AT` | `EUR` | [The network charges of each area](at). |
| Belgium | `BE` | `EUR` | [Fluvius, ORES, RESA and Sibelga's plans](be). Flanders' capacity tariff isn't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Bulgaria | `BG` | `EUR` | [The three big operators' plans](bg). |
| Croatia | `HR` | `EUR` | [HEP ODS's plans](hr). |
| Czechia | `CZ` | `CZK` | Prices from SMARD, in euros, converted into koruna at the ECB's daily rate. [The three operators' plans](cz). |
| Denmark | `DK1` (west), `DK2` (east) | `DKK` | [The eight big grid companies' plans](dk). |
| Estonia | `EE` | `EUR` | [Elektrilevi's plans](ee). |
| Finland | `FI` | `EUR` | [The eight biggest network companies' plans](fi), without Helen Sähköverkko Aikasiirto's charge per kW ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| France | `FR` | `EUR` | Off-peak hours are set for each address: [write a custom plan](fr) with them. |
| Germany | `DE` | `EUR` | [The big grid operators' plans](de), Modul 3 included. |
| Hungary | `HU` | `HUF` | Prices from SMARD, in euros, converted into forints at the ECB's daily rate. [The network fees](hu). |
| Italy, the north | `IT-NORTH` | `EUR` | Prices from SMARD, for the north's price area only. [The TD plan](it). |
| Latvia | `LV` | `EUR` | [Sadales tīkls' plans](lv). |
| Lithuania | `LT` | `EUR` | [ESO's plans](lt). |
| Luxembourg | `LU` | `EUR` | [Luxembourg's plan](lu), without the supplement above the reference power ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Netherlands | `NL` | `EUR` | No grid fee per kWh: [no plan needed](nl). |
| Norway | `NO1` to `NO5` | `NOK` | [The biggest network companies' plans](no), without the capacity step ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Poland | `PL` | `PLN` | [The five big operators' plans](pl). |
| Portugal | `PT` | `EUR` | Prices from OMIE. [The plans](pt), on the mainland. |
| Romania | `RO` | `RON` | [The four distribution operators' plans](ro). |
| Slovenia | `SI` | `EUR` | Prices from SMARD. [The network charge](si), without its charge per kW ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Spain | `ES` | `EUR` | Prices from OMIE. [The 2.0TD tolls and charges](es). |
| Sweden | `SE1` to `SE4` | `SEK` | [The big network companies' plans](se). Power fees at some aren't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Switzerland | `CH` | `CHF` | Prices from SMARD, in euros, converted into francs at the ECB's daily rate. [Nine operators' plans](ch). |

Your electricity contract or your supplier's price list names your area. Prices are in the currency the table gives, your grid fees too.

## Grid fees

The board adds VAT to the market price, and a grid fee that can change every quarter-hour: rates for each day of the week, other rates in some months, and their own on public holidays and other dates. A plan holds its grid operator's fee per kWh, with VAT. Taxes and charges that are the same in every hour don't change which hours are cheapest, so plans leave them out, and each country's page says which. Left out, they're missing from every price the board shows, the schedule's windows, the phone message and what Savings says you paid; added to every rate's price, those show your full price.

Some fees depend on your highest power instead, like Norway's capacity step. Until the board counts them ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)), set the car's charging current low enough for your step, in the car or the Tesla app, and **Charging power** in the board's setup to match.

## Custom plan

Without your grid operator's plan here, upload a custom plan, or [add the plan](../CONTRIBUTING.md#plans) for everyone on it. To change a plan's prices for yourself, download its `.yaml` file from its country's folder here, change it and upload it as a custom plan.

A plan is a text file, like [ESO's with four zones](lt/eso-standartinis-4-zones.yaml):

- `name`: what the page calls it, like `My grid plan`. It can be left out.
- `currency`: the currency of its prices, your country's in [Countries](#countries), like `EUR` or `NOK`. It can be left out.
- `calendar`: the months, like `jan-dec`, each with the days of the week, like `mon-fri` and `sat-sun`. Each month and each day of the week is named once. Ranges may run past the end of the year or the week, like `nov-mar` and `fri-mon`.
- A day's line: the rate from midnight, then each time it changes, on a quarter-hour, and the rate from then: `night 07:00 day 23:00 night`. One rate alone is the whole day.
- `exceptions`: public holidays and other dates that differ, like `12-25`, each with a line for the whole day. On those dates they replace the calendar.
- `rates`: each rate's price per kWh, with VAT. A rate's name is a word like `night` or `p1`.
- `clock: winter`: all times stay on winter time all year. `clock: local`, the default, follows the clock, summer time included.
- A line that belongs to another starts two spaces further in, like the months under `calendar` and the days under a month. A comment starts with `#`, on a line of its own.

A fee that's dearer from November to March, Monday to Saturday from 07:00 to 22:00, like Finland's seasonal grid fee:

```yaml
# Prices with VAT: https://example.com
name: My grid plan
currency: EUR
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

To upload it, open the board's page: where it lists plans for your country, tick **My plan isn't listed** under **Grid plan**. Press **Upload custom plan**, choose the file, a `.yaml`, `.yml` or `.txt` one, and press **Finish** in the setup or **Save** under **Prices**. **Grid plan** then shows it as **Custom:** and its `name`, and **Reset custom plan** drops it, to choose a plan from the list or upload another.

Your plan stays as you uploaded it, while the board downloads a plan from the list every day: change yours when your prices change, and holidays that move, like Easter Monday, every year.

The board turns away a plan without a calendar, a rate without a price, a rate no day uses, a rate, a date, a month, a day or the clock named twice, a month or a day not named at all, a time that isn't a later quarter-hour, a date that doesn't exist, more than 26 rates, or a `currency` other than your country's, and keeps the settings it has. The page says what's wrong, with the line's number in your file where there is one, like `your plan: line 3 isn't a key and a value`. Your plan and your other settings can have 4 kB together: if yours is longer, the page says so, and leaving out its comments helps.

## A fixed price

With a fixed price, or one that changes only with the hour of the day, choose **Fixed** under **Contract type** on the board's page: no market prices are downloaded. It works in every country under **Country / Area**, in its currency.

With your grid operator's plan, enter your supplier's own part per kWh, without the grid fees, as **Supplier's part**, which the board adds to the plan's fees in every hour. Where your supplier quotes one price with the grid fees in, it's the supplier's own line on the bill. Without a plan, all hours cost the same, so the board charges at once.

With day and night prices, upload a custom plan whose rates are your whole price per kWh, the supplier's price and the grid fee together, with VAT, and leave **Supplier's part** at 0: the board then charges in the cheap hours. A night rate that starts on the half-hour, from 00:30 to 05:30, with your rates:

```yaml
# Prices with VAT: https://example.com
name: My night rate
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
| The rest of Italy, Slovakia, Greece, Ireland and other countries outside Nord Pool, SMARD and OMIE | Their day-ahead prices are on other exchanges. ENTSO-E publishes them all. In Italy a fixed price works already, with [the TD plan](it); the others aren't under **Country / Area** yet. | [#80](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/80) |
| United Kingdom | Neither Nord Pool nor ENTSO-E has British prices. Suppliers like Octopus Agile publish their own. The page has no British pounds, so not even a fixed price works yet. | [#81](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/81) |

The page and these documents are in English.
