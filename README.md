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

Worked out on the market prices of every night from October 2025, when they went to quarter-hours, to September 2026, for one driver, the same in every country:

- **The car:** a 75 kWh Tesla that drives 20,000 km a year, about 55 km a day, at 15 kWh per 100 km.
- **Every night:** plugged in from 18:00 to 07:00, charging at 11 kW.
- **The driver already saves:** the Tesla's own scheduler starts charging at the same time every night, when cheap hours usually begin.
- **The contract:** a price that changes every hour with the market, called dynamic or spot, or the usual price where homes can't have one.
- **Full prices:** each kWh costs what the bill would say, with grid fees, taxes and VAT.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/images/savings-dark.svg">
  <img src="docs/images/savings-light.svg" width="720" alt="October 2025 to September 2026: 20,000 km a year, about 55 km a day. Sweden, Vattenfall Tidstariff (SE3), in kronor: 7,543 SEK charging at once, 6,197 SEK with the car's own schedule from 22:00 and 4,974 SEK with this board. Lithuania, ESO Standartinis with four zones: 1,001 EUR at once, 786 EUR from 22:00 and 536 EUR with this board. Spain, 2.0TD: 744 EUR at once, 616 EUR from 00:00 and 476 EUR with this board.">
</picture>

That's 20% less than the car's own schedule in Sweden, 32% in Lithuania and 23% in Spain, as the board finds the cheap quarter-hours wherever they fall each night. No fixed start does as well: the board still costs 5%, 8% and 12% less than the best one, which shows only afterwards, from 03:30 in Sweden, 03:45 in Lithuania and 04:00 in Spain. A driver whose car charges as soon as it's plugged in would save more: 34%, 46% and 36%.

### Every country

Where homes pay the market price, charging with the board cost 4% to 36% less than charging scheduled for the night, by the country and the grid plan. Much of that comes from charging later in the night, which a schedule can do too: set to the start time that turned out cheapest over the year, it came within 2% of the board on nearly half the plans, though no one knows that time in advance. Where homes pay regulated or fixed prices, a night schedule does as well as the board. In many countries few homes pay the market price yet, as each country's confidence says. These are shares of what charging costs, not of the whole bill.

Open your country for what charging cost a year on each grid plan, on the car's own schedule and with this board. Confidence says how many homes there pay the prices the example takes: high where many do, medium where a few percent do or most chose another price, low where hardly any do or the bill follows other prices.

<details>
<summary><b>Austria</b>: 5% to 6% less, low confidence</summary>

- **Energie Klagenfurt:** 886 EUR on the car's schedule from 00:00, 841 EUR with this board, 5% less
- **Energienetze Steiermark:** 953 EUR on the car's schedule from 00:00, 908 EUR with this board, 5% less
- **Innsbrucker Kommunalbetriebe:** 926 EUR on the car's schedule from 00:00, 881 EUR with this board, 5% less
- **KNG-Kärnten Netz:** 988 EUR on the car's schedule from 00:00, 944 EUR with this board, 5% less
- **LINZ NETZ:** 829 EUR on the car's schedule from 00:00, 784 EUR with this board, 5% less
- **Netz Burgenland:** 925 EUR on the car's schedule from 00:00, 880 EUR with this board, 5% less
- **Netz Niederösterreich:** 954 EUR on the car's schedule from 00:00, 909 EUR with this board, 5% less
- **Netz Oberösterreich:** 859 EUR on the car's schedule from 00:00, 815 EUR with this board, 5% less
- **Salzburg Netz:** 864 EUR on the car's schedule from 00:00, 820 EUR with this board, 5% less
- **Stromnetz Graz:** 820 EUR on the car's schedule from 00:00, 775 EUR with this board, 5% less
- **TINETZ-Tiroler Netze:** 871 EUR on the car's schedule from 00:00, 826 EUR with this board, 5% less
- **Vorarlberger Energienetze:** 801 EUR on the car's schedule from 00:00, 756 EUR with this board, 6% less
- **Wiener Netze:** 954 EUR on the car's schedule from 00:00, 907 EUR with this board, 5% less

Low: about 1% of homes paid the market price in 2025. [The research](plans/at/AGENTS.md)

</details>

<details>
<summary><b>Belgium</b>: 4% to 13% less, low confidence</summary>

- **Fluvius Antwerpen, digital meter:** 839 EUR on the car's schedule from 00:00, 783 EUR with this board, 7% less
- **Fluvius Halle-Vilvoorde, digital meter:** 851 EUR on the car's schedule from 00:00, 796 EUR with this board, 7% less
- **Fluvius Imewo, digital meter:** 847 EUR on the car's schedule from 00:00, 792 EUR with this board, 7% less
- **Fluvius Kempen, digital meter:** 849 EUR on the car's schedule from 00:00, 793 EUR with this board, 7% less
- **Fluvius Limburg, digital meter:** 838 EUR on the car's schedule from 00:00, 782 EUR with this board, 7% less
- **Fluvius Midden-Vlaanderen, digital meter:** 844 EUR on the car's schedule from 00:00, 788 EUR with this board, 7% less
- **Fluvius West, digital meter:** 855 EUR on the car's schedule from 00:00, 799 EUR with this board, 7% less
- **Fluvius Zenne-Dijle, digital meter:** 851 EUR on the car's schedule from 00:00, 796 EUR with this board, 7% less
- **ORES Bihoraire, two rates:** 1,000 EUR on the car's schedule from 22:00, 878 EUR with this board, 12% less
- **ORES Impact, three rates:** 831 EUR on the car's schedule from 01:00, 799 EUR with this board, 4% less
- **ORES Monohoraire, one rate:** 1,081 EUR on the car's schedule from 00:00, 1,025 EUR with this board, 5% less
- **RESA Bihoraire, two rates:** 979 EUR on the car's schedule from 22:00, 858 EUR with this board, 12% less
- **RESA Impact, three rates:** 819 EUR on the car's schedule from 01:00, 788 EUR with this board, 4% less
- **RESA Monohoraire, one rate:** 1,043 EUR on the car's schedule from 00:00, 987 EUR with this board, 5% less
- **Sibelga Bihoraire, two rates:** 949 EUR on the car's schedule from 22:00, 823 EUR with this board, 13% less
- **Sibelga Monohoraire, one rate:** 958 EUR on the car's schedule from 00:00, 902 EUR with this board, 6% less

Low: 0.4% of homes pay the market price, and Flanders' fee per kW isn't counted. [The research](plans/be/AGENTS.md)

</details>

<details>
<summary><b>Bulgaria</b>: nothing saved, high confidence</summary>

- **Regulated price, in ERM Zapad's area:** 306 EUR on the car's schedule from 23:00, 306 EUR with this board, the same

High: nearly every home pays a regulated price, cheaper at night. [The research](plans/bg/AGENTS.md)

</details>

<details>
<summary><b>Croatia</b>: nothing saved, high confidence</summary>

- **Regulated price, HEP ODS Bijeli:** 338 EUR on the car's schedule from 22:00, 338 EUR with this board, the same

High: no home pays the market price, and the regulated one is cheaper at night. [The research](plans/hr/AGENTS.md)

</details>

<details>
<summary><b>Czechia</b>: 5% to 30% less, low confidence</summary>

- **EG.D D01d:** 24,443 CZK on the car's schedule from 00:00, 23,320 CZK with this board, 5% less
- **EG.D D02d:** 22,766 CZK on the car's schedule from 00:00, 21,643 CZK with this board, 5% less
- **EG.D D61d:** 23,477 CZK on the car's schedule from 00:00, 20,849 CZK with this board, 11% less
- **PREdistribuce D01d:** 20,989 CZK on the car's schedule from 00:00, 19,866 CZK with this board, 5% less
- **PREdistribuce D02d:** 19,625 CZK on the car's schedule from 00:00, 18,502 CZK with this board, 6% less
- **PREdistribuce D61d:** 20,467 CZK on the car's schedule from 00:00, 18,397 CZK with this board, 10% less
- **ČEZ Distribuce D01d:** 24,263 CZK on the car's schedule from 00:00, 23,141 CZK with this board, 5% less
- **ČEZ Distribuce D02d:** 21,892 CZK on the car's schedule from 00:00, 20,769 CZK with this board, 5% less
- **ČEZ Distribuce D27d, interval meter:** 18,634 CZK on the car's schedule from 20:00, 13,057 CZK with this board, 30% less
- **ČEZ Distribuce D61d:** 23,179 CZK on the car's schedule from 00:00, 20,523 CZK with this board, 11% less

Low: few homes pay the market price. [The research](plans/cz/AGENTS.md)

</details>

<details>
<summary><b>Denmark</b>: 9% less, high confidence</summary>

- **Cerius Nettarif C (DK2):** 4,575 DKK on the car's schedule from 00:00, 4,186 DKK with this board, 9% less
- **Dinel Nettarif C (DK1):** 4,394 DKK on the car's schedule from 00:00, 3,996 DKK with this board, 9% less
- **Konstant Nettarif C (DK1):** 4,344 DKK on the car's schedule from 00:00, 3,938 DKK with this board, 9% less
- **N1 Nettarif C (DK1):** 4,504 DKK on the car's schedule from 00:00, 4,118 DKK with this board, 9% less
- **Nord Energi Net Nettarif C (DK1):** 4,443 DKK on the car's schedule from 00:00, 4,050 DKK with this board, 9% less
- **Radius Elnet Nettarif C (DK2):** 4,537 DKK on the car's schedule from 00:00, 4,146 DKK with this board, 9% less
- **TREFOR El-net Nettarif C (DK1):** 4,317 DKK on the car's schedule from 00:00, 3,908 DKK with this board, 9% less
- **Vores Elnet Nettarif C (DK1):** 4,459 DKK on the car's schedule from 00:00, 4,066 DKK with this board, 9% less

High: most homes pay the market price. [The research](plans/dk/AGENTS.md)

</details>

<details>
<summary><b>Estonia</b>: 12% to 36% less, high confidence</summary>

- **Elektrilevi Võrk 1, up to 63 A:** 682 EUR on the car's schedule from 00:00, 604 EUR with this board, 12% less
- **Elektrilevi Võrk 2, up to 63 A:** 645 EUR on the car's schedule from 22:00, 433 EUR with this board, 33% less
- **Elektrilevi Võrk 4, up to 63 A:** 586 EUR on the car's schedule from 22:00, 374 EUR with this board, 36% less
- **Elektrilevi Võrk 5, up to 63 A:** 625 EUR on the car's schedule from 22:00, 413 EUR with this board, 34% less

High: exchange-price packages are common. [The research](plans/ee/AGENTS.md)

</details>

<details>
<summary><b>Finland</b>: 6% to 25% less, high confidence</summary>

- **Caruna Espoo Kausisiirto:** 400 EUR on the car's schedule from 22:00, 300 EUR with this board, 25% less
- **Caruna Espoo Yleissiirto:** 375 EUR on the car's schedule from 00:00, 347 EUR with this board, 7% less
- **Caruna Espoo Yösiirto:** 393 EUR on the car's schedule from 22:00, 294 EUR with this board, 25% less
- **Caruna Kausisiirto:** 462 EUR on the car's schedule from 22:00, 362 EUR with this board, 22% less
- **Caruna Yleissiirto:** 457 EUR on the car's schedule from 00:00, 430 EUR with this board, 6% less
- **Caruna Yösiirto:** 459 EUR on the car's schedule from 22:00, 360 EUR with this board, 22% less
- **Elenia Vuodenaikasiirto:** 447 EUR on the car's schedule from 22:00, 347 EUR with this board, 22% less
- **Elenia Yleissiirto:** 478 EUR on the car's schedule from 00:00, 451 EUR with this board, 6% less
- **Elenia Yösiirto:** 447 EUR on the car's schedule from 22:00, 348 EUR with this board, 22% less
- **Helen Sähköverkko Aikasiirto:** 407 EUR on the car's schedule from 22:00, 308 EUR with this board, 24% less
- **Helen Sähköverkko Yleissiirto:** 439 EUR on the car's schedule from 00:00, 412 EUR with this board, 6% less
- **Oulun Energia Aikasähkö:** 409 EUR on the car's schedule from 22:00, 310 EUR with this board, 24% less
- **Oulun Energia Kausisähkö:** 420 EUR on the car's schedule from 22:00, 320 EUR with this board, 24% less
- **Oulun Energia Yleissähkö:** 388 EUR on the car's schedule from 00:00, 360 EUR with this board, 7% less
- **Savon Voima Aikasähkö:** 458 EUR on the car's schedule from 22:00, 358 EUR with this board, 22% less
- **Savon Voima Yleissähkö:** 456 EUR on the car's schedule from 00:00, 428 EUR with this board, 6% less
- **Tampereen Energia Aikasiirto:** 402 EUR on the car's schedule from 22:00, 303 EUR with this board, 25% less
- **Tampereen Energia Kausisiirto:** 435 EUR on the car's schedule from 22:00, 335 EUR with this board, 23% less
- **Tampereen Energia Yleissiirto:** 391 EUR on the car's schedule from 00:00, 363 EUR with this board, 7% less
- **Vantaan Energia Aikasiirto:** 409 EUR on the car's schedule from 22:00, 310 EUR with this board, 24% less
- **Vantaan Energia Kausisiirto:** 445 EUR on the car's schedule from 22:00, 345 EUR with this board, 23% less
- **Vantaan Energia Kerrostalosiirto:** 386 EUR on the car's schedule from 00:00, 358 EUR with this board, 7% less
- **Vantaan Energia Yleissiirto:** 392 EUR on the car's schedule from 00:00, 365 EUR with this board, 7% less

High: a third of homes pay the market price. [The research](plans/fi/AGENTS.md)

</details>

<details>
<summary><b>France</b>: 23% less, low confidence</summary>

- **TURPE CU4, off-peak 22:00 - 06:00:** 732 EUR on the car's schedule from 22:00, 561 EUR with this board, 23% less

Low: two small suppliers bill the market price, and off-peak hours differ by address. [The research](plans/fr/AGENTS.md)

</details>

<details>
<summary><b>Germany</b>: 5% to 23% less, low confidence</summary>

- **Avacon Netz Modul 2, separate meter:** 866 EUR on the car's schedule from 00:00, 783 EUR with this board, 10% less
- **Avacon Netz Modul 3, three rates:** 924 EUR on the car's schedule from 23:00, 820 EUR with this board, 11% less
- **Avacon Netz, standard:** 1,010 EUR on the car's schedule from 00:00, 927 EUR with this board, 8% less
- **Bayernwerk Netz Modul 2, separate meter:** 845 EUR on the car's schedule from 00:00, 762 EUR with this board, 10% less
- **Bayernwerk Netz Modul 3, three rates:** 1,037 EUR on the car's schedule from 22:00, 889 EUR with this board, 14% less
- **Bayernwerk Netz, standard:** 957 EUR on the car's schedule from 00:00, 874 EUR with this board, 9% less
- **E.DIS Netz Modul 2, separate meter:** 857 EUR on the car's schedule from 00:00, 774 EUR with this board, 10% less
- **E.DIS Netz Modul 3, three rates:** 893 EUR on the car's schedule from 23:30, 808 EUR with this board, 10% less
- **E.DIS Netz, standard:** 987 EUR on the car's schedule from 00:00, 904 EUR with this board, 8% less
- **EAM Netz Modul 2, separate meter:** 857 EUR on the car's schedule from 00:00, 774 EUR with this board, 10% less
- **EAM Netz Modul 3, three rates:** 857 EUR on the car's schedule from 23:00, 773 EUR with this board, 10% less
- **EAM Netz, standard:** 987 EUR on the car's schedule from 00:00, 904 EUR with this board, 8% less
- **EWE NETZ Modul 2, separate meter:** 821 EUR on the car's schedule from 00:00, 738 EUR with this board, 10% less
- **EWE NETZ Modul 3, three rates:** 805 EUR on the car's schedule from 23:00, 719 EUR with this board, 11% less
- **EWE NETZ, standard:** 897 EUR on the car's schedule from 00:00, 814 EUR with this board, 9% less
- **Hamburger Energienetze Modul 2:** 924 EUR on the car's schedule from 00:00, 841 EUR with this board, 9% less
- **Hamburger Energienetze Modul 3:** 903 EUR on the car's schedule from 00:30, 863 EUR with this board, 5% less
- **Hamburger Energienetze, standard:** 1,155 EUR on the car's schedule from 00:00, 1,072 EUR with this board, 7% less
- **MITNETZ STROM Modul 2, separate meter:** 870 EUR on the car's schedule from 00:00, 787 EUR with this board, 10% less
- **MITNETZ STROM Modul 3, three rates:** 1,078 EUR on the car's schedule from 19:00, 831 EUR with this board, 23% less
- **MITNETZ STROM, standard:** 1,020 EUR on the car's schedule from 00:00, 937 EUR with this board, 8% less
- **N-ERGIE Netz Modul 2, separate meter:** 864 EUR on the car's schedule from 00:00, 781 EUR with this board, 10% less
- **N-ERGIE Netz Modul 3, three rates:** 887 EUR on the car's schedule from 23:00, 803 EUR with this board, 9% less
- **N-ERGIE Netz, standard:** 1,006 EUR on the car's schedule from 00:00, 923 EUR with this board, 8% less
- **Netze BW Modul 2, separate meter:** 890 EUR on the car's schedule from 00:00, 807 EUR with this board, 9% less
- **Netze BW Modul 3, three rates:** 1,150 EUR on the car's schedule from 22:00, 1,001 EUR with this board, 13% less
- **Netze BW, standard:** 1,070 EUR on the car's schedule from 00:00, 987 EUR with this board, 8% less
- **RheinNetz Modul 2, separate meter:** 830 EUR on the car's schedule from 00:00, 747 EUR with this board, 10% less
- **RheinNetz Modul 3, three rates:** 815 EUR on the car's schedule from 23:45, 751 EUR with this board, 8% less
- **RheinNetz, standard:** 920 EUR on the car's schedule from 00:00, 837 EUR with this board, 9% less
- **SWM Infrastruktur Modul 2, separate meter:** 873 EUR on the car's schedule from 00:00, 790 EUR with this board, 10% less
- **SWM Infrastruktur Modul 3, three rates:** 926 EUR on the car's schedule from 00:45, 869 EUR with this board, 6% less
- **SWM Infrastruktur, standard:** 1,027 EUR on the car's schedule from 00:00, 944 EUR with this board, 8% less
- **Stromnetz Berlin Modul 2, separate meter:** 888 EUR on the car's schedule from 00:00, 805 EUR with this board, 9% less
- **Stromnetz Berlin Modul 3, three rates:** 928 EUR on the car's schedule from 22:15, 812 EUR with this board, 13% less
- **Stromnetz Berlin, standard:** 1,066 EUR on the car's schedule from 00:00, 983 EUR with this board, 8% less
- **Westnetz Modul 2, separate meter:** 921 EUR on the car's schedule from 00:00, 838 EUR with this board, 9% less
- **Westnetz Modul 3, three rates:** 808 EUR on the car's schedule from 00:00, 747 EUR with this board, 8% less
- **Westnetz, standard:** 1,148 EUR on the car's schedule from 00:00, 1,065 EUR with this board, 7% less

Low: dynamic contracts and Modul 3 need a smart meter, and 6% of meters are one. [The research](plans/de/AGENTS.md)

</details>

<details>
<summary><b>Hungary</b>: nothing saved, medium confidence</summary>

- **A2 on the car's own meter, ELMŰ:** 137,960 HUF on the car's schedule from 23:00, 137,960 HUF with this board, the same

Medium: homes pay regulated prices, and the car's own meter (A2) pays less at night. [The research](plans/hu/AGENTS.md)

</details>

<details>
<summary><b>Ireland</b>: nothing saved, high confidence</summary>

- **Electric Ireland Night Boost:** 363 EUR on the car's schedule from 02:00, 363 EUR with this board, the same

High: electric car owners usually take a fixed plan with a cheap night, cheaper than dynamic contracts. [The research](plans/ie/AGENTS.md)

</details>

<details>
<summary><b>Italy</b>: 5% less, medium confidence</summary>

- **TD transport and meter charges:** 971 EUR on the car's schedule from 00:00, 926 EUR with this board, 5% less

Medium: about 3% of homes pay the hourly price. [The research](plans/it/AGENTS.md)

</details>

<details>
<summary><b>Latvia</b>: 14% to 25% less, high confidence</summary>

- **Sadales tīkls Pamata:** 561 EUR on the car's schedule from 00:00, 418 EUR with this board, 25% less
- **Sadales tīkls Speciālais:** 1,040 EUR on the car's schedule from 00:00, 898 EUR with this board, 14% less

High: one home in six pays the market price. [The research](plans/lv/AGENTS.md)

</details>

<details>
<summary><b>Lithuania</b>: 17% to 34% less, medium confidence</summary>

- **ESO Efektyvus, four zones:** 745 EUR on the car's schedule from 22:00, 492 EUR with this board, 34% less
- **ESO Efektyvus, one zone:** 734 EUR on the car's schedule from 00:00, 593 EUR with this board, 19% less
- **ESO Efektyvus, two zones:** 629 EUR on the car's schedule from 00:00, 502 EUR with this board, 20% less
- **ESO Namai, one zone:** 758 EUR on the car's schedule from 00:00, 617 EUR with this board, 19% less
- **ESO Namai, two zones:** 646 EUR on the car's schedule from 00:00, 519 EUR with this board, 20% less
- **ESO Standartinis, four zones:** 786 EUR on the car's schedule from 22:00, 536 EUR with this board, 32% less
- **ESO Standartinis, one zone:** 811 EUR on the car's schedule from 00:00, 670 EUR with this board, 17% less
- **ESO Standartinis, two zones:** 678 EUR on the car's schedule from 00:00, 552 EUR with this board, 19% less

Medium: a few percent of homes pay each quarter-hour's price, most a fixed one. [The research](plans/lt/AGENTS.md)

</details>

<details>
<summary><b>Luxembourg</b>: 8% less, low confidence</summary>

- **Network fee, standard client:** 652 EUR on the car's schedule from 00:00, 602 EUR with this board, 8% less

Low: few homes pay the market price. [The research](plans/lu/AGENTS.md)

</details>

<details>
<summary><b>Netherlands</b>: 7% less, high confidence</summary>

- **No grid fee per kWh:** 877 EUR on the car's schedule from 00:00, 817 EUR with this board, 7% less

High: 8% of homes pay the market price. [The research](plans/nl/AGENTS.md)

</details>

<details>
<summary><b>Norway</b>: 4% to 14% less, medium confidence</summary>

- **Arva, households (NO4):** 1,804 NOK on the car's schedule from 22:00, 1,545 NOK with this board, 14% less
- **BKK, households (NO5):** 4,189 NOK on the car's schedule from 22:00, 3,968 NOK with this board, 5% less
- **Elvia, households (NO1):** 4,520 NOK on the car's schedule from 22:00, 4,243 NOK with this board, 6% less
- **Fagne, households (NO2):** 4,709 NOK on the car's schedule from 22:00, 4,440 NOK with this board, 6% less
- **Glitre Nett, households (NO2):** 4,537 NOK on the car's schedule from 22:00, 4,293 NOK with this board, 5% less
- **Lede, households (NO1):** 4,176 NOK on the car's schedule from 00:00, 3,999 NOK with this board, 4% less
- **Lnett, households (NO2):** 4,443 NOK on the car's schedule from 22:00, 4,172 NOK with this board, 6% less
- **Tensio TN, households (NO3):** 3,874 NOK on the car's schedule from 22:00, 3,634 NOK with this board, 6% less
- **Tensio TS, households (NO3):** 3,757 NOK on the car's schedule from 22:00, 3,513 NOK with this board, 6% less

Medium: most homes' power is at Norgespris, a fixed price, where the car's own schedule does as well. [The research](plans/no/AGENTS.md)

</details>

<details>
<summary><b>Poland</b>: 5% to 15% less, low confidence</summary>

- **Enea Operator G11:** 3,608 PLN on the car's schedule from 00:00, 3,421 PLN with this board, 5% less
- **Enea Operator G12w:** 3,217 PLN on the car's schedule from 22:00, 2,737 PLN with this board, 15% less
- **Energa-Operator G11:** 4,030 PLN on the car's schedule from 00:00, 3,843 PLN with this board, 5% less
- **Energa-Operator G12:** 2,986 PLN on the car's schedule from 23:00, 2,746 PLN with this board, 8% less
- **Energa-Operator G12w:** 2,996 PLN on the car's schedule from 23:00, 2,748 PLN with this board, 8% less
- **PGE Dystrybucja G11:** 4,023 PLN on the car's schedule from 00:00, 3,837 PLN with this board, 5% less
- **PGE Dystrybucja G12:** 2,961 PLN on the car's schedule from 23:00, 2,721 PLN with this board, 8% less
- **PGE Dystrybucja G12w:** 2,993 PLN on the car's schedule from 23:00, 2,746 PLN with this board, 8% less
- **Stoen Operator G11:** 3,561 PLN on the car's schedule from 00:00, 3,374 PLN with this board, 5% less
- **Stoen Operator G12:** 2,875 PLN on the car's schedule from 23:00, 2,633 PLN with this board, 8% less
- **Stoen Operator G12w:** 3,089 PLN on the car's schedule from 23:00, 2,843 PLN with this board, 8% less
- **Tauron Dystrybucja G11:** 3,611 PLN on the car's schedule from 00:00, 3,424 PLN with this board, 5% less
- **Tauron Dystrybucja G12:** 2,876 PLN on the car's schedule from 23:00, 2,635 PLN with this board, 8% less
- **Tauron Dystrybucja G12w:** 2,857 PLN on the car's schedule from 23:00, 2,609 PLN with this board, 9% less

Low: 0.03% of homes have a dynamic contract, and it bills another auction's price. [The research](plans/pl/AGENTS.md)

</details>

<details>
<summary><b>Portugal</b>: 13% to 20% less, low confidence</summary>

- **Bi-horária, ciclo diário:** 680 EUR on the car's schedule from 22:00, 568 EUR with this board, 16% less
- **Bi-horária, ciclo semanal:** 627 EUR on the car's schedule from 00:00, 548 EUR with this board, 13% less
- **Simples:** 812 EUR on the car's schedule from 00:00, 694 EUR with this board, 15% less
- **Tri-horária, ciclo diário:** 680 EUR on the car's schedule from 22:00, 541 EUR with this board, 20% less
- **Tri-horária, ciclo semanal:** 627 EUR on the car's schedule from 00:00, 532 EUR with this board, 15% less

Low: under 1% of homes pay the market price. [The research](plans/pt/AGENTS.md)

</details>

<details>
<summary><b>Romania</b>: 14% to 15% less, low confidence</summary>

- **DEER, low voltage:** 5,704 RON on the car's schedule from 00:00, 4,899 RON with this board, 14% less
- **Delgaz Grid, low voltage:** 5,836 RON on the car's schedule from 00:00, 5,030 RON with this board, 14% less
- **Distribuție Energie Oltenia, low voltage:** 5,615 RON on the car's schedule from 00:00, 4,810 RON with this board, 14% less
- **Rețele Electrice România, low voltage:** 5,551 RON on the car's schedule from 00:00, 4,746 RON with this board, 15% less

Low: no home had a dynamic contract in 2025. [The research](plans/ro/AGENTS.md)

</details>

<details>
<summary><b>Slovakia</b>: nothing saved, high confidence</summary>

- **Regulated price, DD3 in ZSD's area:** 509 EUR on the car's schedule from 23:30, 509 EUR with this board, the same

High: nearly every home pays a regulated price, cheaper at night. [The research](plans/sk/AGENTS.md)

</details>

<details>
<summary><b>Slovenia</b>: 17% less, low confidence</summary>

- **Network charge, user group 0:** 713 EUR on the car's schedule from 22:00, 593 EUR with this board, 17% less

Low: 0.3% of homes pay the market price. [The research](plans/si/AGENTS.md)

</details>

<details>
<summary><b>Spain</b>: 23% less, high confidence</summary>

- **2.0TD tolls and charges:** 616 EUR on the car's schedule from 00:00, 476 EUR with this board, 23% less

High: over a quarter of homes pay each hour's price, most on the regulated PVPC. [The research](plans/es/AGENTS.md)

</details>

<details>
<summary><b>Sweden</b>: 5% to 26% less, high confidence</summary>

- **E.ON Nord, 16 A up to 8,000 kWh (SE2):** 6,558 SEK on the car's schedule from 00:00, 6,203 SEK with this board, 5% less
- **E.ON Nord, 16-63 A (SE2):** 4,201 SEK on the car's schedule from 00:00, 3,846 SEK with this board, 8% less
- **E.ON Nord, apartment 16 A (SE2):** 6,964 SEK on the car's schedule from 00:00, 6,610 SEK with this board, 5% less
- **E.ON Nord, apartment 20-25 A (SE2):** 6,383 SEK on the car's schedule from 00:00, 6,028 SEK with this board, 6% less
- **E.ON Stockholm, 16 A up to 8,000 kWh (SE3):** 6,757 SEK on the car's schedule from 00:00, 6,197 SEK with this board, 8% less
- **E.ON Stockholm, 16-63 A (SE3):** 5,394 SEK on the car's schedule from 00:00, 4,834 SEK with this board, 10% less
- **E.ON Stockholm, apartment 16 A (SE3):** 7,212 SEK on the car's schedule from 00:00, 6,652 SEK with this board, 8% less
- **E.ON Stockholm, apartment 20-25 A (SE3):** 6,757 SEK on the car's schedule from 00:00, 6,197 SEK with this board, 8% less
- **E.ON Syd, 16 A up to 8,000 kWh (SE4):** 8,378 SEK on the car's schedule from 00:00, 7,567 SEK with this board, 10% less
- **E.ON Syd, 16-63 A (SE4):** 6,396 SEK on the car's schedule from 00:00, 5,586 SEK with this board, 13% less
- **E.ON Syd, apartment 16 A (SE4):** 8,960 SEK on the car's schedule from 00:00, 8,149 SEK with this board, 9% less
- **E.ON Syd, apartment 20-25 A (SE4):** 8,378 SEK on the car's schedule from 00:00, 7,567 SEK with this board, 10% less
- **Ellevio (SE3):** 5,384 SEK on the car's schedule from 00:00, 4,824 SEK with this board, 10% less
- **Ellevio, single phase up to 20 A (SE3):** 6,184 SEK on the car's schedule from 00:00, 5,624 SEK with this board, 9% less
- **Göteborg Energi Tidsindelad (SE3):** 4,734 SEK on the car's schedule from 00:00, 4,174 SEK with this board, 12% less
- **Göteborg Energi, apartment (SE3):** 5,767 SEK on the car's schedule from 00:00, 5,207 SEK with this board, 10% less
- **Göteborg Energi, house (SE3):** 5,284 SEK on the car's schedule from 00:00, 4,724 SEK with this board, 11% less
- **Kraftringen (SE4):** 5,986 SEK on the car's schedule from 00:00, 5,176 SEK with this board, 14% less
- **Mälarenergi (SE3):** 6,317 SEK on the car's schedule from 00:00, 5,757 SEK with this board, 9% less
- **Tekniska verken Linköping Alternativ (SE3):** 5,023 SEK on the car's schedule from 23:00, 4,281 SEK with this board, 15% less
- **Tekniska verken Linköping Standard (SE3):** 5,014 SEK on the car's schedule from 00:00, 4,454 SEK with this board, 11% less
- **Tekniska verken Linköping, apartment (SE3):** 5,634 SEK on the car's schedule from 00:00, 5,074 SEK with this board, 10% less
- **Vattenfall Eldistribution Enkeltariff (SE3):** 6,000 SEK on the car's schedule from 00:00, 5,441 SEK with this board, 9% less
- **Vattenfall Eldistribution Tidstariff (SE3):** 6,197 SEK on the car's schedule from 22:00, 4,974 SEK with this board, 20% less
- **Öresundskraft Enkeltariff (SE4):** 5,886 SEK on the car's schedule from 00:00, 5,076 SEK with this board, 14% less
- **Öresundskraft Tidstariff (SE4):** 6,509 SEK on the car's schedule from 22:00, 4,793 SEK with this board, 26% less

High: one contract in seven pays each quarter-hour's or hour's price, most the month's average. [The research](plans/se/AGENTS.md)

</details>

<details>
<summary><b>Switzerland</b>: nothing saved, high confidence</summary>

- **EKZ's price, Netz 400ST:** 803 CHF on the car's schedule from 00:00, 803 CHF with this board, the same

High: homes pay their grid operator's price, which at EKZ, the largest, is the same at every hour. [The research](plans/ch/AGENTS.md)

</details>

AI agents researched each country in October 2026: its notes give every figure's source, what the example settled and what's still open, like [Lithuania's](plans/lt/AGENTS.md). Something wrong or out of date? [Open an issue](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/new/choose).

<details>
<summary>How it's calculated</summary>

- **The nights:** 365, from 1 October 2025 to 30 September 2026, in local time. 20,000 km a year at 15 kWh per 100 km is 8.2 kWh a night into the battery, and 9.1 kWh from the grid, as the board counts 90% of it reaching the battery.
- **The market prices:** each quarter-hour's, from the day-ahead auction, where a country has several areas, Stockholm's (SE3), Oslo's (NO1), Copenhagen's (DK2) and Italy's north. As Nord Pool keeps only its last two months free to download, they come from others that publish the same, to the cent where checked: Elering, Estonia's grid operator, for the Baltics and Finland, elprisetjustnu.se for Sweden, Energy-Charts for Norway and Romania and SMARD, Germany's Federal Network Agency, for the rest; for Spain and Portugal from OMIE, the Iberian market, as the board downloads them. Other currencies are at the ECB's daily rate, within half a percent of Nord Pool's own. Where bills count hours, as many still do, each hour costs the average of its four quarter-hours.
- **The grid fees:** every plan the board has, at their 2026 prices all year, like [Vattenfall Tidstariff](plans/se/vattenfall-tidstariff.yaml), [ESO Standartinis with four zones](plans/lt/eso-standartinis-4-zones.yaml) and [2.0TD](plans/es/2-0td.yaml) in the chart. Where a country has several market areas, a plan takes the one most of its company's homes are in, named after it. France's off-peak hours from 22:00 are a custom plan, as the board has none, and the Netherlands has no grid fee per kWh.
- **The rest of each kWh:** VAT, a typical dynamic contract's margin, and what's the same every hour: taxes, levies and, where suppliers bill them, grid losses, at their 2026 rates. Monthly fees are left out, as they don't depend on when the car charges, and so are fees per kW, which the board doesn't count yet ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)).
- **Regulated and fixed prices:** where homes can't have a dynamic contract, or it costs more than their usual price, the usual price, with its network fees and levies: the regulated prices in Bulgaria, Croatia, Slovakia and Switzerland, in Hungary A2, for an electric car's own meter, and in Ireland Night Boost, a plan for electric cars with a cheap night.
- **The car's own schedule:** from when the plan's cheapest fee of the night starts on a workday, in the month it starts latest, like 22:00 for ESO's four zones, or from midnight where the fee is the same all night.
- **This board:** the board's own scheduler, the same code, picks each night's cheapest quarter-hours from the prices out at 18:00, for the same energy.
- **The research:** AI agents gathered each country's figures, one per country, from regulators', statistics offices' and suppliers' own pages, each with its source. Scripts then checked that no source or figure was lost when the notes were tidied, that each country's summary matches its row here, and that the market prices match Nord Pool's where both could be compared. Each country's notes list the sources to re-check every January.
- **The chart:** Sweden's costs are in kronor, as its board counts them, and its bars as long as their euros at the ECB's daily rate, so the three compare.
- **To check it**, run [test/savings_example.cpp](test/savings_example.cpp) in the project's folder, with a C++ compiler, curl and ArduinoJson, as [CONTRIBUTING.md](CONTRIBUTING.md#checks) shows. It downloads the prices, prints what each plan cost, and last the list of countries above.

```bash
g++ -std=c++17 -I . -isystem ArduinoJson/src test/savings_example.cpp -o test/savings_example
test/savings_example
```

</details>

## How it works

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/images/how-it-works-dark.svg">
  <img src="docs/images/how-it-works-light.svg" width="760" alt="Day-ahead prices and your grid plan go into the board next to the car, which starts and stops your Tesla over Bluetooth, and your phone reaches the board's page over Wi-Fi">
</picture>

1. **Prices:** the board downloads tomorrow's prices once they're out, around 13:00 CET, and adds VAT and your grid plan's fees to each quarter-hour.
2. **Schedule:** when you plug in, it picks the cheapest quarter-hours that reach your charge limit by **Ready by**, with a spare one in case charging runs slow.
3. **Charging:** it starts and stops charging over Bluetooth, and sends the schedule to your phone through the ntfy app, if you like.

## What you need

- A Tesla Model 3, Model Y, Cybertruck, or Model S/X from 2021 on.
- An ESP32-S3-DevKitC-1 (N16R8) board, within Bluetooth range of the car and on your Wi-Fi.
- A USB-C cable for your computer, and a USB-C phone charger near the car to power the board.
- A home charger that charges whenever the car asks: no schedule, auto-lock or app approval (OCPP) of its own.
- A price that follows the day-ahead market, often called spot or exchange price, in Austria, Belgium, Bulgaria, Croatia, Czechia, Denmark, Estonia, Finland, France, Germany, Hungary, Ireland, Italy (the north), Latvia, Lithuania, Luxembourg, the Netherlands, Norway, Poland, Portugal, Romania, Slovakia, Slovenia, Spain, Sweden or Switzerland. [Countries and plans](plans/README.md) has the details, and how to use a fixed price instead.
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

Yes. **Start charging now** on the board's page charges to the limit at any price, and charging you start from the car or the Tesla app goes ahead: the board leaves it alone until you unplug.

</details>

<details>
<summary><b>What if the board or the prices are missing?</b></summary>

The car charges as usual. Without prices or a battery level, the board lets the car charge at any price, and tells your phone if messages are on. With the board unplugged, the car is on its own.

</details>

<details>
<summary><b>What can the board's key do?</b></summary>

Only charging. The board adds its own key, like a phone key, with Tesla's Charging Manager role: it can start and stop charging and set the charge limit, but it can't unlock the doors or drive. You can remove it in the car under **Controls** → **Locks**.

</details>

<details>
<summary><b>What leaves my home?</b></summary>

No account, and nothing goes through Tesla's servers. The board downloads market prices, grid plans and, when you open its page, news of a release. Only phone messages go out, if you turn them on: the schedule, through the ntfy app's server, under a long random topic. The page has no password: anyone on your Wi-Fi can open it.

</details>

## For developers

The scheduler is plain C++17 that runs on a computer as well as on the board.

- **Tested:** 85 unit tests cover every line and branch of the scheduler, mutation testing kills 92% of its mutants, and a simulation runs the board's logic with a simulated Tesla.
- **Built on ESPHome:** an external component in `scheduler/`, with [esphome-tesla-ble](https://github.com/yoziru/esphome-tesla-ble) for the car, and a page of plain JavaScript and CSS that the board serves.
- **Checked in CI:** clang-tidy, CodeQL, SonarCloud and the page's tests in a headless browser, and every release signed for the boards' updates.
- **Open to your country:** a grid operator's plan is a short YAML file in `plans/`, which every board downloads within a day.

[CONTRIBUTING.md](CONTRIBUTING.md) has the checks to run and how releases are made.

## Support

If it saves you money, star the repository so others find it, or [buy the developer a coffee](https://buymeacoffee.com/zygimantas_berziunas). Ideas and problems are welcome as [issues](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues).

## Credits and license

The Bluetooth link to the car is [esphome-tesla-ble](https://github.com/yoziru/esphome-tesla-ble), which implements Tesla's [vehicle-command](https://github.com/teslamotors/vehicle-command) protocol. Prices come from Nord Pool's data portal, SMARD (Bundesnetzagentur | SMARD.de, CC BY 4.0), OMIE (OMI-Polo Español, S.A.), OKTE (OKTE, a.s.) and SEMOpx (the market of Ireland and Northern Ireland). This project isn't affiliated with Tesla, Nord Pool, the Bundesnetzagentur, OMIE, OKTE, SEMOpx or any grid operator. Use it at your own risk.

Licensed under the [GNU Affero General Public License v3.0](LICENSE).
