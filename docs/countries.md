# Countries

The board works where Nord Pool publishes the day-ahead prices of your area, and your grid fees are a price per kWh that may change with the hour, or there are none. It applies EU summer time, which every country here uses.

## Supported

| Country | `nordpool: area` | `nordpool: currency` | Good to know |
| --- | --- | --- | --- |
| Austria | `AT` | `EUR` | |
| Belgium | `BE` | `EUR` | Flanders' capacity tariff isn't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Bulgaria | `BG` | `EUR` | |
| Croatia | `HR` | `EUR` | |
| Denmark | `DK1` (west), `DK2` (east) | `DKK` or `EUR` | |
| Estonia | `EE` | `EUR` | |
| Finland | `FI` | `EUR` | |
| France | `FR` | `EUR` | |
| Germany and Luxembourg | `GER` | `EUR` | |
| Latvia | `LV` | `EUR` | |
| Lithuania | `LT` | `EUR` | ESO's plans are ready to paste in [ESO plans](eso.md). |
| Netherlands | `NL` | `EUR` | |
| Norway | `NO1` to `NO5` | `NOK` or `EUR` | The capacity step isn't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Poland | `PL` | `EUR` | Prices and fees in euros only, not złoty ([#79](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/79)). |
| Sweden | `SE1` to `SE4` | `SEK` or `EUR` | Power fees at some network companies aren't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |

Your electricity contract or your supplier's price list names your area. Write your grid fees in the same currency as the prices.

## Grid fees

The board adds VAT to the price, and a grid fee that can change by the hour: zones by hour for workdays, weekends and public holidays, other hours for part of the year, and holidays on fixed dates or around Easter. Taxes and charges that are the same in every hour don't change which hours are cheapest. Left out, they're missing from every price the board shows, the plan's windows, the phone message and what Savings says you paid; added to every zone's fee, those show your full price. [Grid fees](grid-fees.md) explains the format.

Some fees depend on your highest power instead, like Norway's capacity step. Until the board counts them ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)), set the car's charging current low enough for your step, in the car or the Tesla app, and `tesla_charging_kw` to match.

## Not supported yet

| Where | Why | Issue |
| --- | --- | --- |
| Romania | Nord Pool has its prices, as area `TEL` in lei, but the board doesn't offer them yet. | [#78](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/78) |
| Spain, Portugal, Italy, Czechia, Slovakia, Hungary, Slovenia, Greece, Switzerland, Ireland and other countries outside Nord Pool | Their day-ahead prices are on other exchanges. ENTSO-E publishes them all. | [#80](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/80) |
| United Kingdom | Neither Nord Pool nor ENTSO-E has British prices. Suppliers like Octopus Agile publish their own. | [#81](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/81) |
| Holidays on a moving weekday, like Sweden's Midsummer Eve | Holidays can only be fixed dates or days around Easter. | [#83](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/83) |

The page and these documents are in English.
