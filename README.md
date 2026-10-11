<div align="center">

<img src="docs/images/logo.svg" width="88" height="88" alt="">

# ESPHome Tesla BLE Scheduler

**Charge your Tesla in the cheapest hours of the night, automatically.**<br>
A small ESP32 board by the car follows the electricity market and controls charging over Bluetooth.

[![Latest release](https://img.shields.io/github/v/release/zygimantas/esphome-tesla-ble-scheduler)](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest)
[![CI](https://github.com/zygimantas/esphome-tesla-ble-scheduler/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/zygimantas/esphome-tesla-ble-scheduler/actions/workflows/ci.yml)
[![License](https://img.shields.io/github/license/zygimantas/esphome-tesla-ble-scheduler)](LICENSE)

[Install](#install) · [How it works](#how-it-works) · [Countries](plans/README.md) · [User guide](docs/guide.md) · [Contributing](CONTRIBUTING.md)

<img src="docs/images/hero.png" width="760" alt="The board's page on a phone: tonight's schedule, one window from 00:45 to 05:30, and its settings">

</div>

## Why

Day-ahead electricity prices change every quarter-hour, and the cheapest hours of a night often cost a fraction of the evening peak. A Tesla's own schedule knows times, not prices, and the tools that follow prices usually need a cloud service, your Tesla account, Home Assistant or a new charger.

This board needs none of them. When you plug in, it picks the cheapest quarter-hours that still get the car to its charge limit by the time you leave, counting your grid fees and VAT.

- **Runs at home:** no cloud, no Tesla account, no subscription, no Home Assistant.
- **Can only charge:** its key can't unlock the doors or drive the car.
- **Works in 26 European countries,** most with their grid operators' plans built in.
- **Updates from its own page,** with releases signed by the project.

## Example of yearly savings

For one driver, the same in every country, from October 2025 to September 2026:

- **The car:** a 75 kWh Tesla driving 20,000 km a year, about 55 km a day.
- **Schedule:** the Tesla's own schedule starts charging at the same time every night, when cheap hours usually begin.
- **Full prices:** with grid fees, taxes and VAT.

**Sweden, Vattenfall Eldistribution Tidstariff (SE3)**

| Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| ---: | ---: | ---: | ---: |
| 22:00 | 6,197&nbsp;SEK | 4,974&nbsp;SEK | 1,223&nbsp;SEK,&nbsp;20% |

### Savings by country

Open your country to see what charging cost a year on each grid plan, with the car's own schedule and with this board.

<details>
<summary>How it's calculated</summary>
<p></p>

- **The nights:** 365, in local time, from 1 October 2025, when the market prices went to quarter-hours, plugged in from 18:00 to 07:00 and charging at 11 kW. 20,000 km a year at 15 kWh per 100 km is 8.2 kWh a night into the battery and 9.1 kWh from the grid, with 10% lost in charging.
- **The market prices:** on a dynamic (spot) contract, each quarter-hour's day-ahead price, northern Italy's for Italy, from Elering, elprisetjustnu.se, Energy-Charts, SMARD and OMIE, the same as Nord Pool's where checked. Other currencies are at the ECB's daily rate. Where bills count hours, each hour costs the average of its quarter-hours.
- **The grid fees:** every plan the board has where homes pay the market price, at 2026 prices, like [Vattenfall's Tidstariff](plans/se/vattenfall-tidstariff.yaml) in the example, each in the market area most of its homes are in, shown in brackets. France's off-peak hours from 22:00 are a custom plan, as the board has none, and the Netherlands has no grid fee per kWh.
- **The rest of each kWh:** VAT, a typical supplier's margin, taxes, levies and, where suppliers bill them, grid losses, at 2026 rates. Monthly fees are left out, and so are fees per kW ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)).
- **Regulated and fixed prices:** where homes usually pay those: the regulated prices in Bulgaria, Croatia, Slovakia and Switzerland, Hungary's A2 for an electric car's own meter, and Ireland's Night Boost, a plan for electric cars with a cheap night.
- **The car's own schedule:** starts when the plan's cheapest fee of the night does on a workday, in the month it starts latest, like 22:00 for Vattenfall's Tidstariff, or at midnight where the fee is the same all night.
- **This board:** its own code picks each night's cheapest quarter-hours from the prices out at 18:00.
- **To check it**, run [test/savings_example.cpp](test/savings_example.cpp) as [CONTRIBUTING.md](CONTRIBUTING.md#checks) shows: it downloads the prices and prints each plan's cost, then the list below.

```bash
g++ -std=c++17 -I . -isystem ArduinoJson/src test/savings_example.cpp -o test/savings_example
test/savings_example
```

</details>

<table>
<tr><td>🟢 <b>Worth it</b></td><td>A plan saves 15% or more, with high or medium confidence</td></tr>
<tr><td>🟡 <b>Maybe</b></td><td>Less than that, or low confidence</td></tr>
<tr><td>⚪ <b>Nothing</b></td><td>Homes pay regulated or fixed prices, where a night schedule does as well</td></tr>
</table>

<p><br></p>

<details>
<summary>🟡 <b>Austria</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Energie Klagenfurt | 00:00 | 886&nbsp;EUR | 841&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| Energienetze Steiermark | 00:00 | 953&nbsp;EUR | 908&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| Innsbrucker Kommunalbetriebe | 00:00 | 926&nbsp;EUR | 881&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| KNG-Kärnten Netz | 00:00 | 988&nbsp;EUR | 944&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| LINZ NETZ | 00:00 | 829&nbsp;EUR | 784&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| Netz Burgenland | 00:00 | 925&nbsp;EUR | 880&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| Netz Niederösterreich | 00:00 | 954&nbsp;EUR | 909&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| Netz Oberösterreich | 00:00 | 859&nbsp;EUR | 815&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| Salzburg Netz | 00:00 | 864&nbsp;EUR | 820&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| Stromnetz Graz | 00:00 | 820&nbsp;EUR | 775&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| TINETZ-Tiroler Netze | 00:00 | 871&nbsp;EUR | 826&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |
| Vorarlberger Energienetze | 00:00 | 801&nbsp;EUR | 756&nbsp;EUR | 45&nbsp;EUR,&nbsp;6% |
| Wiener Netze | 00:00 | 954&nbsp;EUR | 907&nbsp;EUR | 48&nbsp;EUR,&nbsp;5% |

Low confidence: about 1% of homes paid the market price in 2025. [The research](plans/at/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Belgium</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Fluvius Antwerpen, digital meter | 00:00 | 839&nbsp;EUR | 783&nbsp;EUR | 56&nbsp;EUR,&nbsp;7% |
| Fluvius Halle-Vilvoorde, digital meter | 00:00 | 851&nbsp;EUR | 796&nbsp;EUR | 56&nbsp;EUR,&nbsp;7% |
| Fluvius Imewo, digital meter | 00:00 | 847&nbsp;EUR | 792&nbsp;EUR | 56&nbsp;EUR,&nbsp;7% |
| Fluvius Kempen, digital meter | 00:00 | 849&nbsp;EUR | 793&nbsp;EUR | 56&nbsp;EUR,&nbsp;7% |
| Fluvius Limburg, digital meter | 00:00 | 838&nbsp;EUR | 782&nbsp;EUR | 56&nbsp;EUR,&nbsp;7% |
| Fluvius Midden-Vlaanderen, digital meter | 00:00 | 844&nbsp;EUR | 788&nbsp;EUR | 56&nbsp;EUR,&nbsp;7% |
| Fluvius West, digital meter | 00:00 | 855&nbsp;EUR | 799&nbsp;EUR | 56&nbsp;EUR,&nbsp;7% |
| Fluvius Zenne-Dijle, digital meter | 00:00 | 851&nbsp;EUR | 796&nbsp;EUR | 56&nbsp;EUR,&nbsp;7% |
| ORES Bihoraire, two rates | 22:00 | 1,000&nbsp;EUR | 878&nbsp;EUR | 122&nbsp;EUR,&nbsp;12% |
| ORES Impact, three rates | 01:00 | 831&nbsp;EUR | 799&nbsp;EUR | 31&nbsp;EUR,&nbsp;4% |
| ORES Monohoraire, one rate | 00:00 | 1,081&nbsp;EUR | 1,025&nbsp;EUR | 56&nbsp;EUR,&nbsp;5% |
| RESA Bihoraire, two rates | 22:00 | 979&nbsp;EUR | 858&nbsp;EUR | 122&nbsp;EUR,&nbsp;12% |
| RESA Impact, three rates | 01:00 | 819&nbsp;EUR | 788&nbsp;EUR | 31&nbsp;EUR,&nbsp;4% |
| RESA Monohoraire, one rate | 00:00 | 1,043&nbsp;EUR | 987&nbsp;EUR | 56&nbsp;EUR,&nbsp;5% |
| Sibelga Bihoraire, two rates | 22:00 | 949&nbsp;EUR | 823&nbsp;EUR | 126&nbsp;EUR,&nbsp;13% |
| Sibelga Monohoraire, one rate | 00:00 | 958&nbsp;EUR | 902&nbsp;EUR | 56&nbsp;EUR,&nbsp;6% |

Low confidence: 0.4% of homes pay the market price, and Flanders' fee per kW isn't counted. [The research](plans/be/AGENTS.md)

</details>

<details>
<summary>⚪ <b>Bulgaria</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Regulated price, in ERM Zapad's area | 23:00 | 306&nbsp;EUR | 306&nbsp;EUR | nothing |

High confidence: nearly every home pays a regulated price, cheaper at night. [The research](plans/bg/AGENTS.md)

</details>

<details>
<summary>⚪ <b>Croatia</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Regulated price, HEP ODS Bijeli | 22:00 | 338&nbsp;EUR | 338&nbsp;EUR | nothing |

High confidence: no home pays the market price, and the regulated one is cheaper at night. [The research](plans/hr/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Czechia</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| ČEZ Distribuce D01d | 00:00 | 24,263&nbsp;CZK | 23,141&nbsp;CZK | 1,123&nbsp;CZK,&nbsp;5% |
| ČEZ Distribuce D02d | 00:00 | 21,892&nbsp;CZK | 20,769&nbsp;CZK | 1,123&nbsp;CZK,&nbsp;5% |
| ČEZ Distribuce D27d, interval meter | 20:00 | 18,634&nbsp;CZK | 13,057&nbsp;CZK | 5,578&nbsp;CZK,&nbsp;30% |
| ČEZ Distribuce D61d | 00:00 | 23,179&nbsp;CZK | 20,523&nbsp;CZK | 2,655&nbsp;CZK,&nbsp;11% |
| EG.D D01d | 00:00 | 24,443&nbsp;CZK | 23,320&nbsp;CZK | 1,123&nbsp;CZK,&nbsp;5% |
| EG.D D02d | 00:00 | 22,766&nbsp;CZK | 21,643&nbsp;CZK | 1,123&nbsp;CZK,&nbsp;5% |
| EG.D D61d | 00:00 | 23,477&nbsp;CZK | 20,849&nbsp;CZK | 2,628&nbsp;CZK,&nbsp;11% |
| PREdistribuce D01d | 00:00 | 20,989&nbsp;CZK | 19,866&nbsp;CZK | 1,123&nbsp;CZK,&nbsp;5% |
| PREdistribuce D02d | 00:00 | 19,625&nbsp;CZK | 18,502&nbsp;CZK | 1,123&nbsp;CZK,&nbsp;6% |
| PREdistribuce D61d | 00:00 | 20,467&nbsp;CZK | 18,397&nbsp;CZK | 2,070&nbsp;CZK,&nbsp;10% |

Low confidence: few homes pay the market price. [The research](plans/cz/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Denmark</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Cerius Nettarif C (DK2) | 00:00 | 4,575&nbsp;DKK | 4,186&nbsp;DKK | 389&nbsp;DKK,&nbsp;9% |
| Dinel Nettarif C (DK1) | 00:00 | 4,394&nbsp;DKK | 3,996&nbsp;DKK | 398&nbsp;DKK,&nbsp;9% |
| Konstant Nettarif C (DK1) | 00:00 | 4,344&nbsp;DKK | 3,938&nbsp;DKK | 406&nbsp;DKK,&nbsp;9% |
| N1 Nettarif C (DK1) | 00:00 | 4,504&nbsp;DKK | 4,118&nbsp;DKK | 387&nbsp;DKK,&nbsp;9% |
| Nord Energi Net Nettarif C (DK1) | 00:00 | 4,443&nbsp;DKK | 4,050&nbsp;DKK | 393&nbsp;DKK,&nbsp;9% |
| Radius Elnet Nettarif C (DK2) | 00:00 | 4,537&nbsp;DKK | 4,146&nbsp;DKK | 391&nbsp;DKK,&nbsp;9% |
| TREFOR El-net Nettarif C (DK1) | 00:00 | 4,317&nbsp;DKK | 3,908&nbsp;DKK | 409&nbsp;DKK,&nbsp;9% |
| Vores Elnet Nettarif C (DK1) | 00:00 | 4,459&nbsp;DKK | 4,066&nbsp;DKK | 392&nbsp;DKK,&nbsp;9% |

High confidence: most homes pay the market price. [The research](plans/dk/AGENTS.md)

</details>

<details>
<summary>🟢 <b>Estonia</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Elektrilevi Võrk 1, up to 63 A | 00:00 | 682&nbsp;EUR | 604&nbsp;EUR | 79&nbsp;EUR,&nbsp;12% |
| Elektrilevi Võrk 2, up to 63 A | 22:00 | 645&nbsp;EUR | 433&nbsp;EUR | 212&nbsp;EUR,&nbsp;33% |
| Elektrilevi Võrk 4, up to 63 A | 22:00 | 586&nbsp;EUR | 374&nbsp;EUR | 213&nbsp;EUR,&nbsp;36% |
| Elektrilevi Võrk 5, up to 63 A | 22:00 | 625&nbsp;EUR | 413&nbsp;EUR | 212&nbsp;EUR,&nbsp;34% |

High confidence: exchange-price packages are common. [The research](plans/ee/AGENTS.md)

</details>

<details>
<summary>🟢 <b>Finland</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Caruna Espoo Kausisiirto | 22:00 | 400&nbsp;EUR | 300&nbsp;EUR | 100&nbsp;EUR,&nbsp;25% |
| Caruna Espoo Yleissiirto | 00:00 | 375&nbsp;EUR | 347&nbsp;EUR | 28&nbsp;EUR,&nbsp;7% |
| Caruna Espoo Yösiirto | 22:00 | 393&nbsp;EUR | 294&nbsp;EUR | 99&nbsp;EUR,&nbsp;25% |
| Caruna Kausisiirto | 22:00 | 462&nbsp;EUR | 362&nbsp;EUR | 100&nbsp;EUR,&nbsp;22% |
| Caruna Yleissiirto | 00:00 | 457&nbsp;EUR | 430&nbsp;EUR | 28&nbsp;EUR,&nbsp;6% |
| Caruna Yösiirto | 22:00 | 459&nbsp;EUR | 360&nbsp;EUR | 99&nbsp;EUR,&nbsp;22% |
| Elenia Vuodenaikasiirto | 22:00 | 447&nbsp;EUR | 347&nbsp;EUR | 100&nbsp;EUR,&nbsp;22% |
| Elenia Yleissiirto | 00:00 | 478&nbsp;EUR | 451&nbsp;EUR | 28&nbsp;EUR,&nbsp;6% |
| Elenia Yösiirto | 22:00 | 447&nbsp;EUR | 348&nbsp;EUR | 99&nbsp;EUR,&nbsp;22% |
| Helen Sähköverkko Aikasiirto | 22:00 | 407&nbsp;EUR | 308&nbsp;EUR | 99&nbsp;EUR,&nbsp;24% |
| Helen Sähköverkko Yleissiirto | 00:00 | 439&nbsp;EUR | 412&nbsp;EUR | 28&nbsp;EUR,&nbsp;6% |
| Oulun Energia Aikasähkö | 22:00 | 409&nbsp;EUR | 310&nbsp;EUR | 99&nbsp;EUR,&nbsp;24% |
| Oulun Energia Kausisähkö | 22:00 | 420&nbsp;EUR | 320&nbsp;EUR | 100&nbsp;EUR,&nbsp;24% |
| Oulun Energia Yleissähkö | 00:00 | 388&nbsp;EUR | 360&nbsp;EUR | 28&nbsp;EUR,&nbsp;7% |
| Savon Voima Aikasähkö | 22:00 | 458&nbsp;EUR | 358&nbsp;EUR | 100&nbsp;EUR,&nbsp;22% |
| Savon Voima Yleissähkö | 00:00 | 456&nbsp;EUR | 428&nbsp;EUR | 28&nbsp;EUR,&nbsp;6% |
| Tampereen Energia Aikasiirto | 22:00 | 402&nbsp;EUR | 303&nbsp;EUR | 99&nbsp;EUR,&nbsp;25% |
| Tampereen Energia Kausisiirto | 22:00 | 435&nbsp;EUR | 335&nbsp;EUR | 100&nbsp;EUR,&nbsp;23% |
| Tampereen Energia Yleissiirto | 00:00 | 391&nbsp;EUR | 363&nbsp;EUR | 28&nbsp;EUR,&nbsp;7% |
| Vantaan Energia Aikasiirto | 22:00 | 409&nbsp;EUR | 310&nbsp;EUR | 99&nbsp;EUR,&nbsp;24% |
| Vantaan Energia Kausisiirto | 22:00 | 445&nbsp;EUR | 345&nbsp;EUR | 100&nbsp;EUR,&nbsp;23% |
| Vantaan Energia Kerrostalosiirto | 00:00 | 386&nbsp;EUR | 358&nbsp;EUR | 28&nbsp;EUR,&nbsp;7% |
| Vantaan Energia Yleissiirto | 00:00 | 392&nbsp;EUR | 365&nbsp;EUR | 28&nbsp;EUR,&nbsp;7% |

High confidence: a third of homes pay the market price. [The research](plans/fi/AGENTS.md)

</details>

<details>
<summary>🟡 <b>France</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| TURPE CU4, off-peak 22:00 - 06:00 | 22:00 | 732&nbsp;EUR | 561&nbsp;EUR | 171&nbsp;EUR,&nbsp;23% |

Low confidence: two small suppliers bill the market price, and off-peak hours differ by address. [The research](plans/fr/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Germany</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Avacon Netz Modul 2, separate meter | 00:00 | 866&nbsp;EUR | 783&nbsp;EUR | 83&nbsp;EUR,&nbsp;10% |
| Avacon Netz Modul 3, three rates | 23:00 | 924&nbsp;EUR | 820&nbsp;EUR | 105&nbsp;EUR,&nbsp;11% |
| Avacon Netz, standard | 00:00 | 1,010&nbsp;EUR | 927&nbsp;EUR | 83&nbsp;EUR,&nbsp;8% |
| Bayernwerk Netz Modul 2, separate meter | 00:00 | 845&nbsp;EUR | 762&nbsp;EUR | 83&nbsp;EUR,&nbsp;10% |
| Bayernwerk Netz Modul 3, three rates | 22:00 | 1,037&nbsp;EUR | 889&nbsp;EUR | 148&nbsp;EUR,&nbsp;14% |
| Bayernwerk Netz, standard | 00:00 | 957&nbsp;EUR | 874&nbsp;EUR | 83&nbsp;EUR,&nbsp;9% |
| EAM Netz Modul 2, separate meter | 00:00 | 857&nbsp;EUR | 774&nbsp;EUR | 83&nbsp;EUR,&nbsp;10% |
| EAM Netz Modul 3, three rates | 23:00 | 857&nbsp;EUR | 773&nbsp;EUR | 84&nbsp;EUR,&nbsp;10% |
| EAM Netz, standard | 00:00 | 987&nbsp;EUR | 904&nbsp;EUR | 83&nbsp;EUR,&nbsp;8% |
| E.DIS Netz Modul 2, separate meter | 00:00 | 857&nbsp;EUR | 774&nbsp;EUR | 83&nbsp;EUR,&nbsp;10% |
| E.DIS Netz Modul 3, three rates | 23:30 | 893&nbsp;EUR | 808&nbsp;EUR | 85&nbsp;EUR,&nbsp;10% |
| E.DIS Netz, standard | 00:00 | 987&nbsp;EUR | 904&nbsp;EUR | 83&nbsp;EUR,&nbsp;8% |
| EWE NETZ Modul 2, separate meter | 00:00 | 821&nbsp;EUR | 738&nbsp;EUR | 83&nbsp;EUR,&nbsp;10% |
| EWE NETZ Modul 3, three rates | 23:00 | 805&nbsp;EUR | 719&nbsp;EUR | 86&nbsp;EUR,&nbsp;11% |
| EWE NETZ, standard | 00:00 | 897&nbsp;EUR | 814&nbsp;EUR | 83&nbsp;EUR,&nbsp;9% |
| Hamburger Energienetze Modul 2 | 00:00 | 924&nbsp;EUR | 841&nbsp;EUR | 83&nbsp;EUR,&nbsp;9% |
| Hamburger Energienetze Modul 3 | 00:30 | 903&nbsp;EUR | 863&nbsp;EUR | 41&nbsp;EUR,&nbsp;5% |
| Hamburger Energienetze, standard | 00:00 | 1,155&nbsp;EUR | 1,072&nbsp;EUR | 83&nbsp;EUR,&nbsp;7% |
| MITNETZ STROM Modul 2, separate meter | 00:00 | 870&nbsp;EUR | 787&nbsp;EUR | 83&nbsp;EUR,&nbsp;10% |
| MITNETZ STROM Modul 3, three rates | 19:00 | 1,078&nbsp;EUR | 831&nbsp;EUR | 247&nbsp;EUR,&nbsp;23% |
| MITNETZ STROM, standard | 00:00 | 1,020&nbsp;EUR | 937&nbsp;EUR | 83&nbsp;EUR,&nbsp;8% |
| N-ERGIE Netz Modul 2, separate meter | 00:00 | 864&nbsp;EUR | 781&nbsp;EUR | 83&nbsp;EUR,&nbsp;10% |
| N-ERGIE Netz Modul 3, three rates | 23:00 | 887&nbsp;EUR | 803&nbsp;EUR | 84&nbsp;EUR,&nbsp;9% |
| N-ERGIE Netz, standard | 00:00 | 1,006&nbsp;EUR | 923&nbsp;EUR | 83&nbsp;EUR,&nbsp;8% |
| Netze BW Modul 2, separate meter | 00:00 | 890&nbsp;EUR | 807&nbsp;EUR | 83&nbsp;EUR,&nbsp;9% |
| Netze BW Modul 3, three rates | 22:00 | 1,150&nbsp;EUR | 1,001&nbsp;EUR | 150&nbsp;EUR,&nbsp;13% |
| Netze BW, standard | 00:00 | 1,070&nbsp;EUR | 987&nbsp;EUR | 83&nbsp;EUR,&nbsp;8% |
| RheinNetz Modul 2, separate meter | 00:00 | 830&nbsp;EUR | 747&nbsp;EUR | 83&nbsp;EUR,&nbsp;10% |
| RheinNetz Modul 3, three rates | 23:45 | 815&nbsp;EUR | 751&nbsp;EUR | 64&nbsp;EUR,&nbsp;8% |
| RheinNetz, standard | 00:00 | 920&nbsp;EUR | 837&nbsp;EUR | 83&nbsp;EUR,&nbsp;9% |
| Stromnetz Berlin Modul 2, separate meter | 00:00 | 888&nbsp;EUR | 805&nbsp;EUR | 83&nbsp;EUR,&nbsp;9% |
| Stromnetz Berlin Modul 3, three rates | 22:15 | 928&nbsp;EUR | 812&nbsp;EUR | 117&nbsp;EUR,&nbsp;13% |
| Stromnetz Berlin, standard | 00:00 | 1,066&nbsp;EUR | 983&nbsp;EUR | 83&nbsp;EUR,&nbsp;8% |
| SWM Infrastruktur Modul 2, separate meter | 00:00 | 873&nbsp;EUR | 790&nbsp;EUR | 83&nbsp;EUR,&nbsp;10% |
| SWM Infrastruktur Modul 3, three rates | 00:45 | 926&nbsp;EUR | 869&nbsp;EUR | 57&nbsp;EUR,&nbsp;6% |
| SWM Infrastruktur, standard | 00:00 | 1,027&nbsp;EUR | 944&nbsp;EUR | 83&nbsp;EUR,&nbsp;8% |
| Westnetz Modul 2, separate meter | 00:00 | 921&nbsp;EUR | 838&nbsp;EUR | 83&nbsp;EUR,&nbsp;9% |
| Westnetz Modul 3, three rates | 00:00 | 808&nbsp;EUR | 747&nbsp;EUR | 61&nbsp;EUR,&nbsp;8% |
| Westnetz, standard | 00:00 | 1,148&nbsp;EUR | 1,065&nbsp;EUR | 83&nbsp;EUR,&nbsp;7% |

Low confidence: dynamic contracts and Modul 3 need a smart meter, and 6% of meters are one. [The research](plans/de/AGENTS.md)

</details>

<details>
<summary>⚪ <b>Hungary</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| A2 on the car's own meter, ELMŰ | 23:00 | 137,960&nbsp;HUF | 137,960&nbsp;HUF | nothing |

Medium confidence: homes pay regulated prices, and the car's own meter (A2) pays less at night. [The research](plans/hu/AGENTS.md)

</details>

<details>
<summary>⚪ <b>Ireland</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Electric Ireland Night Boost | 02:00 | 363&nbsp;EUR | 363&nbsp;EUR | nothing |

High confidence: electric car owners usually take a fixed plan with a cheap night, cheaper than dynamic contracts. [The research](plans/ie/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Italy</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| TD transport and meter charges | 00:00 | 971&nbsp;EUR | 926&nbsp;EUR | 45&nbsp;EUR,&nbsp;5% |

Medium confidence: about 3% of homes pay the hourly price. [The research](plans/it/AGENTS.md)

</details>

<details>
<summary>🟢 <b>Latvia</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Sadales tīkls Pamata | 00:00 | 561&nbsp;EUR | 418&nbsp;EUR | 142&nbsp;EUR,&nbsp;25% |
| Sadales tīkls Speciālais | 00:00 | 1,040&nbsp;EUR | 898&nbsp;EUR | 142&nbsp;EUR,&nbsp;14% |

High confidence: one home in six pays the market price. [The research](plans/lv/AGENTS.md)

</details>

<details>
<summary>🟢 <b>Lithuania</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| ESO Efektyvus, one zone | 00:00 | 734&nbsp;EUR | 593&nbsp;EUR | 141&nbsp;EUR,&nbsp;19% |
| ESO Efektyvus, two zones | 00:00 | 629&nbsp;EUR | 502&nbsp;EUR | 127&nbsp;EUR,&nbsp;20% |
| ESO Efektyvus, four zones | 22:00 | 745&nbsp;EUR | 492&nbsp;EUR | 254&nbsp;EUR,&nbsp;34% |
| ESO Namai, one zone | 00:00 | 758&nbsp;EUR | 617&nbsp;EUR | 141&nbsp;EUR,&nbsp;19% |
| ESO Namai, two zones | 00:00 | 646&nbsp;EUR | 519&nbsp;EUR | 127&nbsp;EUR,&nbsp;20% |
| ESO Standartinis, one zone | 00:00 | 811&nbsp;EUR | 670&nbsp;EUR | 141&nbsp;EUR,&nbsp;17% |
| ESO Standartinis, two zones | 00:00 | 678&nbsp;EUR | 552&nbsp;EUR | 126&nbsp;EUR,&nbsp;19% |
| ESO Standartinis, four zones | 22:00 | 786&nbsp;EUR | 536&nbsp;EUR | 250&nbsp;EUR,&nbsp;32% |

Medium confidence: a few percent of homes pay each quarter-hour's price, most a fixed one. [The research](plans/lt/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Luxembourg</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Network fee, standard client | 00:00 | 652&nbsp;EUR | 602&nbsp;EUR | 49&nbsp;EUR,&nbsp;8% |

Low confidence: few homes pay the market price. [The research](plans/lu/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Netherlands</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| No grid fee per kWh | 00:00 | 877&nbsp;EUR | 817&nbsp;EUR | 60&nbsp;EUR,&nbsp;7% |

High confidence: 8% of homes pay the market price. [The research](plans/nl/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Norway</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Arva, households (NO4) | 22:00 | 1,804&nbsp;NOK | 1,545&nbsp;NOK | 260&nbsp;NOK,&nbsp;14% |
| BKK, households (NO5) | 22:00 | 4,189&nbsp;NOK | 3,968&nbsp;NOK | 221&nbsp;NOK,&nbsp;5% |
| Elvia, households (NO1) | 22:00 | 4,520&nbsp;NOK | 4,243&nbsp;NOK | 276&nbsp;NOK,&nbsp;6% |
| Fagne, households (NO2) | 22:00 | 4,709&nbsp;NOK | 4,440&nbsp;NOK | 269&nbsp;NOK,&nbsp;6% |
| Glitre Nett, households (NO2) | 22:00 | 4,537&nbsp;NOK | 4,293&nbsp;NOK | 244&nbsp;NOK,&nbsp;5% |
| Lede, households (NO1) | 00:00 | 4,176&nbsp;NOK | 3,999&nbsp;NOK | 176&nbsp;NOK,&nbsp;4% |
| Lnett, households (NO2) | 22:00 | 4,443&nbsp;NOK | 4,172&nbsp;NOK | 271&nbsp;NOK,&nbsp;6% |
| Tensio TN, households (NO3) | 22:00 | 3,874&nbsp;NOK | 3,634&nbsp;NOK | 240&nbsp;NOK,&nbsp;6% |
| Tensio TS, households (NO3) | 22:00 | 3,757&nbsp;NOK | 3,513&nbsp;NOK | 244&nbsp;NOK,&nbsp;6% |

Medium confidence: most homes' power is at Norgespris, a fixed price, where the car's own schedule does as well. [The research](plans/no/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Poland</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Enea Operator G11 | 00:00 | 3,608&nbsp;PLN | 3,421&nbsp;PLN | 186&nbsp;PLN,&nbsp;5% |
| Enea Operator G12w | 22:00 | 3,217&nbsp;PLN | 2,737&nbsp;PLN | 480&nbsp;PLN,&nbsp;15% |
| Energa-Operator G11 | 00:00 | 4,030&nbsp;PLN | 3,843&nbsp;PLN | 186&nbsp;PLN,&nbsp;5% |
| Energa-Operator G12 | 23:00 | 2,986&nbsp;PLN | 2,746&nbsp;PLN | 240&nbsp;PLN,&nbsp;8% |
| Energa-Operator G12w | 23:00 | 2,996&nbsp;PLN | 2,748&nbsp;PLN | 248&nbsp;PLN,&nbsp;8% |
| PGE Dystrybucja G11 | 00:00 | 4,023&nbsp;PLN | 3,837&nbsp;PLN | 186&nbsp;PLN,&nbsp;5% |
| PGE Dystrybucja G12 | 23:00 | 2,961&nbsp;PLN | 2,721&nbsp;PLN | 240&nbsp;PLN,&nbsp;8% |
| PGE Dystrybucja G12w | 23:00 | 2,993&nbsp;PLN | 2,746&nbsp;PLN | 248&nbsp;PLN,&nbsp;8% |
| Stoen Operator G11 | 00:00 | 3,561&nbsp;PLN | 3,374&nbsp;PLN | 186&nbsp;PLN,&nbsp;5% |
| Stoen Operator G12 | 23:00 | 2,875&nbsp;PLN | 2,633&nbsp;PLN | 242&nbsp;PLN,&nbsp;8% |
| Stoen Operator G12w | 23:00 | 3,089&nbsp;PLN | 2,843&nbsp;PLN | 247&nbsp;PLN,&nbsp;8% |
| Tauron Dystrybucja G11 | 00:00 | 3,611&nbsp;PLN | 3,424&nbsp;PLN | 186&nbsp;PLN,&nbsp;5% |
| Tauron Dystrybucja G12 | 23:00 | 2,876&nbsp;PLN | 2,635&nbsp;PLN | 241&nbsp;PLN,&nbsp;8% |
| Tauron Dystrybucja G12w | 23:00 | 2,857&nbsp;PLN | 2,609&nbsp;PLN | 248&nbsp;PLN,&nbsp;9% |

Low confidence: 0.03% of homes have a dynamic contract, and it bills another auction's price. [The research](plans/pl/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Portugal</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Bi-horária, ciclo diário | 22:00 | 680&nbsp;EUR | 568&nbsp;EUR | 112&nbsp;EUR,&nbsp;16% |
| Bi-horária, ciclo semanal | 00:00 | 627&nbsp;EUR | 548&nbsp;EUR | 79&nbsp;EUR,&nbsp;13% |
| Simples | 00:00 | 812&nbsp;EUR | 694&nbsp;EUR | 118&nbsp;EUR,&nbsp;15% |
| Tri-horária, ciclo diário | 22:00 | 680&nbsp;EUR | 541&nbsp;EUR | 139&nbsp;EUR,&nbsp;20% |
| Tri-horária, ciclo semanal | 00:00 | 627&nbsp;EUR | 532&nbsp;EUR | 96&nbsp;EUR,&nbsp;15% |

Low confidence: under 1% of homes pay the market price. [The research](plans/pt/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Romania</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| DEER, low voltage | 00:00 | 5,704&nbsp;RON | 4,899&nbsp;RON | 806&nbsp;RON,&nbsp;14% |
| Delgaz Grid, low voltage | 00:00 | 5,836&nbsp;RON | 5,030&nbsp;RON | 806&nbsp;RON,&nbsp;14% |
| Distribuție Energie Oltenia, low voltage | 00:00 | 5,615&nbsp;RON | 4,810&nbsp;RON | 806&nbsp;RON,&nbsp;14% |
| Rețele Electrice România, low voltage | 00:00 | 5,551&nbsp;RON | 4,746&nbsp;RON | 806&nbsp;RON,&nbsp;15% |

Low confidence: no home had a dynamic contract in 2025. [The research](plans/ro/AGENTS.md)

</details>

<details>
<summary>⚪ <b>Slovakia</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Regulated price, DD3 in ZSD's area | 23:30 | 509&nbsp;EUR | 509&nbsp;EUR | nothing |

High confidence: nearly every home pays a regulated price, cheaper at night. [The research](plans/sk/AGENTS.md)

</details>

<details>
<summary>🟡 <b>Slovenia</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Network charge, user group 0 | 22:00 | 713&nbsp;EUR | 593&nbsp;EUR | 120&nbsp;EUR,&nbsp;17% |

Low confidence: 0.3% of homes pay the market price. [The research](plans/si/AGENTS.md)

</details>

<details>
<summary>🟢 <b>Spain</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| 2.0TD tolls and charges | 00:00 | 616&nbsp;EUR | 476&nbsp;EUR | 140&nbsp;EUR,&nbsp;23% |

High confidence: over a quarter of homes pay each hour's price, most on the regulated PVPC. [The research](plans/es/AGENTS.md)

</details>

<details>
<summary>🟢 <b>Sweden</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| Ellevio (SE3) | 00:00 | 5,384&nbsp;SEK | 4,824&nbsp;SEK | 560&nbsp;SEK,&nbsp;10% |
| Ellevio, single phase up to 20 A (SE3) | 00:00 | 6,184&nbsp;SEK | 5,624&nbsp;SEK | 560&nbsp;SEK,&nbsp;9% |
| E.ON Nord, 16-63 A (SE2) | 00:00 | 4,201&nbsp;SEK | 3,846&nbsp;SEK | 355&nbsp;SEK,&nbsp;8% |
| E.ON Nord, 16 A up to 8,000 kWh (SE2) | 00:00 | 6,558&nbsp;SEK | 6,203&nbsp;SEK | 355&nbsp;SEK,&nbsp;5% |
| E.ON Nord, apartment 16 A (SE2) | 00:00 | 6,964&nbsp;SEK | 6,610&nbsp;SEK | 355&nbsp;SEK,&nbsp;5% |
| E.ON Nord, apartment 20-25 A (SE2) | 00:00 | 6,383&nbsp;SEK | 6,028&nbsp;SEK | 355&nbsp;SEK,&nbsp;6% |
| E.ON Stockholm, 16-63 A (SE3) | 00:00 | 5,394&nbsp;SEK | 4,834&nbsp;SEK | 560&nbsp;SEK,&nbsp;10% |
| E.ON Stockholm, 16 A up to 8,000 kWh (SE3) | 00:00 | 6,757&nbsp;SEK | 6,197&nbsp;SEK | 560&nbsp;SEK,&nbsp;8% |
| E.ON Stockholm, apartment 16 A (SE3) | 00:00 | 7,212&nbsp;SEK | 6,652&nbsp;SEK | 560&nbsp;SEK,&nbsp;8% |
| E.ON Stockholm, apartment 20-25 A (SE3) | 00:00 | 6,757&nbsp;SEK | 6,197&nbsp;SEK | 560&nbsp;SEK,&nbsp;8% |
| E.ON Syd, 16-63 A (SE4) | 00:00 | 6,396&nbsp;SEK | 5,586&nbsp;SEK | 811&nbsp;SEK,&nbsp;13% |
| E.ON Syd, 16 A up to 8,000 kWh (SE4) | 00:00 | 8,378&nbsp;SEK | 7,567&nbsp;SEK | 811&nbsp;SEK,&nbsp;10% |
| E.ON Syd, apartment 16 A (SE4) | 00:00 | 8,960&nbsp;SEK | 8,149&nbsp;SEK | 811&nbsp;SEK,&nbsp;9% |
| E.ON Syd, apartment 20-25 A (SE4) | 00:00 | 8,378&nbsp;SEK | 7,567&nbsp;SEK | 811&nbsp;SEK,&nbsp;10% |
| Göteborg Energi, apartment (SE3) | 00:00 | 5,767&nbsp;SEK | 5,207&nbsp;SEK | 560&nbsp;SEK,&nbsp;10% |
| Göteborg Energi, house (SE3) | 00:00 | 5,284&nbsp;SEK | 4,724&nbsp;SEK | 560&nbsp;SEK,&nbsp;11% |
| Göteborg Energi Tidsindelad (SE3) | 00:00 | 4,734&nbsp;SEK | 4,174&nbsp;SEK | 560&nbsp;SEK,&nbsp;12% |
| Kraftringen (SE4) | 00:00 | 5,986&nbsp;SEK | 5,176&nbsp;SEK | 811&nbsp;SEK,&nbsp;14% |
| Mälarenergi (SE3) | 00:00 | 6,317&nbsp;SEK | 5,757&nbsp;SEK | 560&nbsp;SEK,&nbsp;9% |
| Öresundskraft Enkeltariff (SE4) | 00:00 | 5,886&nbsp;SEK | 5,076&nbsp;SEK | 811&nbsp;SEK,&nbsp;14% |
| Öresundskraft Tidstariff (SE4) | 22:00 | 6,509&nbsp;SEK | 4,793&nbsp;SEK | 1,716&nbsp;SEK,&nbsp;26% |
| Tekniska verken Linköping Alternativ (SE3) | 23:00 | 5,023&nbsp;SEK | 4,281&nbsp;SEK | 741&nbsp;SEK,&nbsp;15% |
| Tekniska verken Linköping, apartment (SE3) | 00:00 | 5,634&nbsp;SEK | 5,074&nbsp;SEK | 560&nbsp;SEK,&nbsp;10% |
| Tekniska verken Linköping Standard (SE3) | 00:00 | 5,014&nbsp;SEK | 4,454&nbsp;SEK | 560&nbsp;SEK,&nbsp;11% |
| Vattenfall Eldistribution Enkeltariff (SE3) | 00:00 | 6,000&nbsp;SEK | 5,441&nbsp;SEK | 560&nbsp;SEK,&nbsp;9% |
| Vattenfall Eldistribution Tidstariff (SE3) | 22:00 | 6,197&nbsp;SEK | 4,974&nbsp;SEK | 1,223&nbsp;SEK,&nbsp;20% |

High confidence: one contract in seven pays each quarter-hour's or hour's price, most the month's average. [The research](plans/se/AGENTS.md)

</details>

<details>
<summary>⚪ <b>Switzerland</b></summary>
<p></p>

| Grid plan | Car's&nbsp;schedule&nbsp;from | Car's&nbsp;schedule | This&nbsp;board | Saved&nbsp;a&nbsp;year |
| --- | ---: | ---: | ---: | ---: |
| EKZ's price, Netz 400ST | 00:00 | 803&nbsp;CHF | 803&nbsp;CHF | nothing |

High confidence: homes pay their grid operator's price, which at EKZ, the largest, is the same at every hour. [The research](plans/ch/AGENTS.md)

</details>

## How it works

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/images/how-it-works-dark.svg">
  <img src="docs/images/how-it-works-light.svg" width="760" alt="Day-ahead prices and your grid plan go into the board next to the car, which reads your Tesla and starts and stops its charging over Bluetooth, while your phone reaches the board's page over Wi-Fi">
</picture>
<p></p>

1. **Prices:** the board downloads tomorrow's prices once they're out, around 13:00 CET, and adds VAT and your grid plan's fees to each quarter-hour.
2. **Schedule:** when you plug in, it picks the cheapest quarter-hours that reach your charge limit by **Ready by**, with a spare one in case charging runs slow.
3. **Charging:** it starts and stops charging over Bluetooth, and sends the schedule to your phone through the ntfy app, if you like.

## What you need

- A Tesla Model 3, Model Y, Cybertruck, or Model S/X from 2021 on.
- An ESP32-S3-DevKitC-1 (N16R8) board, within Bluetooth range of the car and on your Wi-Fi.
- A USB-C cable for your computer, and a USB-C phone charger near the car to power the board.
- A home charger that charges whenever the car asks: no schedule, auto-lock or app approval (OCPP) of its own.
- An electricity price that follows the day-ahead market, called dynamic or spot. [The yearly savings](#example-of-yearly-savings) show where that pays off, and [Countries and plans](plans/README.md) has the details.
- Chrome or Edge on a computer, for the first install.

## Install

1. **At your computer:** download [the firmware](https://github.com/zygimantas/esphome-tesla-ble-scheduler/releases/latest/download/esphome-tesla-ble-scheduler.bin), plug the board's USB-C port labelled **COM** into the computer, and install the firmware with [ESPHome Web](https://web.esphome.io) in Chrome or Edge. Then give the board your Wi-Fi.
2. **On the board's page,** which ESPHome Web then opens: choose your country, grid plan and contract, and scan the QR code with your phone.
3. **At the car:** plug the board into a phone charger near the car, sit in the car with your key card, and finish the setup on your phone.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/images/setup-dark.gif">
  <img src="docs/images/setup-light.gif" width="800" alt="The whole setup: at a computer, ESPHome Web installs the firmware and sets up the Wi-Fi, then the board's page at tesla.local takes the prices and shows a QR code; the phone in the car scans it, opens the page and goes through the VIN, the charging key and the end of the setup, then shows the first schedule once the car is plugged in">
</picture>

From then on, just plug in: the car is charged by **Ready by**. The [user guide](docs/guide.md) explains the page, phone messages and what to do when something is off.

## Questions

<details>
<summary><b>Can I still charge right away?</b></summary>
<p></p>

Yes. **Start charging now** on the board's page charges to the limit at any price, and charging you start from the car or the Tesla app goes ahead: the board leaves it alone until you unplug.

</details>

<details>
<summary><b>What if the board or the prices are missing?</b></summary>
<p></p>

The car charges as usual. Without prices or a battery level, the board lets the car charge at any price, and tells your phone if messages are on. With the board unplugged, the car is on its own.

</details>

<details>
<summary><b>What can the board's key do?</b></summary>
<p></p>

Only charging. The board adds its own key, like a phone key, with Tesla's Charging Manager role: it can start and stop charging and set the charge limit, but it can't unlock the doors or drive. You can remove it in the car under **Controls** → **Locks**.

</details>

<details>
<summary><b>What leaves my home?</b></summary>
<p></p>

No account, and nothing goes through Tesla's servers. The board downloads market prices, grid plans and, when you open its page, news of a release. Only phone messages go out, if you turn them on: the schedule, through the ntfy app's server, under a long random topic. The page has no password: anyone on your Wi-Fi can open it.

</details>

## Credits and license

The Bluetooth link to the car is [esphome-tesla-ble](https://github.com/yoziru/esphome-tesla-ble), which implements Tesla's [vehicle-command](https://github.com/teslamotors/vehicle-command) protocol. Prices come from Nord Pool's data portal, SMARD (Bundesnetzagentur | SMARD.de, CC BY 4.0), OMIE (OMI-Polo Español, S.A.), OKTE (OKTE, a.s.) and SEMOpx (the market of Ireland and Northern Ireland). This project isn't affiliated with Tesla, Nord Pool, the Bundesnetzagentur, OMIE, OKTE, SEMOpx or any grid operator. Use it at your own risk.

Licensed under the [GNU Affero General Public License v3.0](LICENSE).
