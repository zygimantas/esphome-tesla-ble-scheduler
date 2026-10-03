# Countries

The board follows market prices where Nord Pool publishes the day-ahead prices of your area, and a fixed price anywhere. Your grid fees are a price per kWh that may change with the hour, or there are none. The board applies EU summer time, which every country here uses.

## Supported

| Country | `market: area` | `currency` | Good to know |
| --- | --- | --- | --- |
| Austria | `AT` | `EUR` | |
| Belgium | `BE` | `EUR` | Flanders' capacity tariff isn't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Bulgaria | `BG` | `EUR` | |
| Croatia | `HR` | `EUR` | |
| Denmark | `DK1` (west), `DK2` (east) | `DKK` or `EUR` | |
| Estonia | `EE` | `EUR` | |
| Finland | `FI` | `EUR` | |
| France | `FR` | `EUR` | |
| Germany and Luxembourg | `DE` or `LU` | `EUR` | |
| Latvia | `LV` | `EUR` | |
| Lithuania | `LT` | `EUR` | [ESO's plans](eso.md). |
| Netherlands | `NL` | `EUR` | |
| Norway | `NO1` to `NO5` | `NOK` or `EUR` | The capacity step isn't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Poland | `PL` | `PLN` or `EUR` | |
| Romania | `RO` | `RON` or `EUR` | |
| Sweden | `SE1` to `SE4` | `SEK` or `EUR` | Power fees at some network companies aren't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |

Your electricity contract or your supplier's price list names your area. Prices are in your area's own currency unless you set `currency`, and your grid fees are in the same.

## Grid fees

The board adds VAT to the price, and a grid fee that can change every quarter-hour: rates for each day of the week, other rates in some months, and their own on public holidays and other dates. [Tariff](tariff.md) has the plans and the format. Taxes and charges that are the same in every hour don't change which hours are cheapest. Left out, they're missing from every price the board shows, the schedule's windows, the phone message and what Savings says you paid; added to every rate's price, those show your full price.

Some fees depend on your highest power instead, like Norway's capacity step. Until the board counts them ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)), set the car's charging current low enough for your step, in the car or the Tesla app, and `tesla_charging_kw` to match.

## A fixed price

With a fixed price, or one that changes only with the hour of the day, leave `market:` out: no market prices are downloaded, and each rate's price in `tariff:` is your whole price per kWh with VAT. It works in any country whose clocks change on the EU's dates, in any currency, like `currency: GBP` under `scheduler:`. [Grid fees](grid-fees.md#a-fixed-price) has an example.

## Not supported yet

| Where | Why | Issue |
| --- | --- | --- |
| Spain, Portugal, Italy, Czechia, Slovakia, Hungary, Slovenia, Greece, Switzerland, Ireland and other countries outside Nord Pool | Their day-ahead prices are on other exchanges. ENTSO-E publishes them all. A fixed price works already. | [#80](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/80) |
| United Kingdom | Neither Nord Pool nor ENTSO-E has British prices. Suppliers like Octopus Agile publish their own. A fixed price works already, also with a night rate on the half-hour like Octopus Go's ([Tariff](tariff.md#a-fixed-price)). | [#81](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/81) |

The page and these documents are in English.
