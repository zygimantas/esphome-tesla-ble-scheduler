# Countries

The board follows market prices where Nord Pool, SMARD (Germany's Federal Network Agency) or OMIE (the market of Spain and Portugal) publishes the day-ahead prices of your area, and a fixed price anywhere. Your grid fees are a price per kWh that may change with the hour, or there are none. The board applies EU summer time, which every country here uses.

## Supported

| Country | `market: area` | `currency` | Good to know |
| --- | --- | --- | --- |
| Austria | `AT` | `EUR` | [The network charges of each area](austria.md). |
| Belgium | `BE` | `EUR` | [Fluvius, ORES, RESA and Sibelga's plans](belgium.md). Flanders' capacity tariff isn't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Bulgaria | `BG` | `EUR` | [The three big operators' plans](bulgaria.md). |
| Croatia | `HR` | `EUR` | [HEP ODS's plans](croatia.md). |
| Czechia | `CZ` | `CZK` or `EUR` | Prices from SMARD, in euros, converted into koruna at the ECB's daily rate with `currency: CZK`. [The three operators' plans](czechia.md). |
| Denmark | `DK1` (west), `DK2` (east) | `DKK` or `EUR` | [The eight big grid companies' plans](denmark.md). |
| Estonia | `EE` | `EUR` | [Elektrilevi's plans](elektrilevi.md). |
| Finland | `FI` | `EUR` | [The eight biggest network companies' plans](finland.md), without Helen Sähköverkko Aikasiirto's charge per kW ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| France | `FR` | `EUR` | Off-peak hours are set for each address: [write your own tariff](france.md) with them. |
| Germany | `DE` | `EUR` | [The big grid operators' plans](germany.md), Modul 3 included. |
| Hungary | `HU` | `HUF` or `EUR` | Prices from SMARD, in euros, converted into forints at the ECB's daily rate with `currency: HUF`. [The network fees](hungary.md). |
| Italy, the north | `IT-NORTH` | `EUR` | Prices from SMARD, for the north's price area only. [The TD tariff](italy.md). |
| Latvia | `LV` | `EUR` | [Sadales tīkls' plans](latvia.md). |
| Lithuania | `LT` | `EUR` | [ESO's plans](eso.md). |
| Luxembourg | `LU` | `EUR` | [Luxembourg's network tariff](luxembourg.md), without the supplement above the reference power ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Netherlands | `NL` | `EUR` | No grid fee per kWh: [no plan needed](netherlands.md). |
| Norway | `NO1` to `NO5` | `NOK` or `EUR` | [The biggest network companies' plans](norway.md), without the capacity step ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Poland | `PL` | `PLN` or `EUR` | [The five big operators' plans](poland.md). |
| Portugal | `PT` | `EUR` | Prices from OMIE. [The access tariffs](portugal.md), on the mainland. |
| Romania | `RO` | `RON` or `EUR` | [The four distribution operators' plans](romania.md). |
| Slovenia | `SI` | `EUR` | Prices from SMARD. [The network charge](omreznina.md), without its charge per kW ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Spain | `ES` | `EUR` | Prices from OMIE. [The 2.0TD tolls and charges](spain.md). |
| Sweden | `SE1` to `SE4` | `SEK` or `EUR` | [The big network companies' plans](sweden.md). Power fees at some aren't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). |
| Switzerland | `CH` | `CHF` or `EUR` | Prices from SMARD, in euros, converted into francs at the ECB's daily rate with `currency: CHF`. [Nine operators' plans](switzerland.md). |

Your electricity contract or your supplier's price list names your area. Prices are in your area's own currency unless you set `currency`, or in euros where they come from SMARD or OMIE, and your grid fees are in the same. In Czechia, Hungary and Switzerland, `currency` set to the country's own converts SMARD's prices at the European Central Bank's daily rate, as the page does.

## Grid fees

The board adds VAT to the price, and a grid fee that can change every quarter-hour: rates for each day of the week, other rates in some months, and their own on public holidays and other dates. [Tariff](tariff.md) has the plans and the format. Taxes and charges that are the same in every hour don't change which hours are cheapest. Left out, they're missing from every price the board shows, the schedule's windows, the phone message and what Savings says you paid; added to every rate's price, those show your full price.

Some fees depend on your highest power instead, like Norway's capacity step. Until the board counts them ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)), set the car's charging current low enough for your step, in the car or the Tesla app, and `tesla_charging_kw` to match.

## A fixed price

With a fixed price, or one that changes only with the hour of the day, leave `market:` out: no market prices are downloaded, and each rate's price in `tariff:` is your whole price per kWh with VAT. It works in any country whose clocks change on the EU's dates, in any currency, like `currency: GBP`. [Tariff](tariff.md#a-fixed-price) has an example.

## Not supported yet

| Where | Why | Issue |
| --- | --- | --- |
| The rest of Italy, Slovakia, Greece, Ireland and other countries outside Nord Pool, SMARD and OMIE | Their day-ahead prices are on other exchanges. ENTSO-E publishes them all. A fixed price works already. | [#80](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/80) |
| United Kingdom | Neither Nord Pool nor ENTSO-E has British prices. Suppliers like Octopus Agile publish their own. A fixed price works already, also with a night rate on the half-hour like Octopus Go's ([Tariff](tariff.md#a-fixed-price)). | [#81](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/81) |

The page and these documents are in English.
