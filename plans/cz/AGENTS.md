# plans/cz/

Czechia's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a spot contract billing OTE's day-ahead price, the board's charging cost 13,056.74 CZK (537.78 EUR) over the year from 1 October 2025: 50% less than charging at once (25,961.35 CZK, 1,069.13 EUR), 30% less than the car's own schedule from 20:00 (18,634.40 CZK, 767.52 EUR) and 0% less than its best start, 03:00 (13,085.74 CZK, 538.97 EUR). Confidence in the README is Low: few homes pay the market price.

## Decisions

- **Contract:** a spot contract billing OTE's day-ahead price.
- **Plan:** ČEZ Distribuce's D27d for electric cars, in its variant for interval meters with fixed hours (EMOV1), which plans/cz hasn't got, as a custom plan: `mon-sun: vt 02:00 nt 06:00 vt 20:00 nt`, nt 0.14097 and vt 2.72546 CZK with VAT. The car's own schedule starts at 20:00, when its low fee does.
- **Billing:** each hour at the average of its four quarter-hours, as Delta Green's and Yello's price lists say.
- **Margin:** 0.4477 CZK per kWh with VAT, the middle of four fixed adders.
- **The same every hour:** 0.23297 CZK per kWh with VAT, the electricity tax and system services, POZE being 0 in 2026.
- **Left out:** the breaker fee, the suppliers' monthly fees and the non-network infrastructure fee, as they don't depend on when the car charges.
- **Market prices:** SMARD's series 261, in euros, at the ECB's daily rate.
- **Currency:** the plans are in koruna, and the board converts SMARD's euros into it at the ECB's daily rate, as the owner settled.

## Findings

- **Who pays the market price:** few homes. Dynamic contracts are available, but most households pay a fixed price or an open-ended price list. No count of households on spot contracts was found.
- **Meters and billing:** a household's contract with a dynamic price takes effect only once its supply point has interval metering (Energy Act 458/2000 Sb., § 11ca(3)), and suppliers with 200,000 supply points or more must offer one to such customers (§ 30(3)(a)). Interval metering means types A and B or the smart meters C1 to C3 (decree 359/2020 Sb., § 5). Most homes still have a C4 meter, read once a year. The decree requires a smart meter above 6 MWh a year, with solar or storage, or in energy sharing, and a home with an EV often passes 6 MWh. ČEZ's and bezDodavatele's price lists bill each quarter-hour; Delta Green's and Yello's still say each hour, and OTE also publishes each hour's average of its four quarter-hours. Hourly offers for homes without interval metering, like bezDodavatele's 24/7, spread the month's use by a standard profile (TDD), so when the car charges doesn't change that bill.
- **VAT:** 21%, the basic rate, on the electricity and on every charge below (VAT Act 235/2004 Sb., § 47(1)(a)), the same from October to December 2025.
- **Per kWh, the same every hour:** 0.23297 with VAT in 2026, and nothing is a share of the bill:
  - the electricity tax (daň z elektřiny), 28.30 Kč/MWh: 0.02830 without VAT, 0.03424 with it (Act 261/2007 Sb., part 47, § 6(2));
  - system services (systémové služby), 164.24 Kč/MWh: 0.16424 without VAT, 0.19873 with it ([ERÚ's cenový výměr 13/2025](https://eru.gov.cz/sites/default/files/obsah/prilohy/erv172025.pdf), point 3.1.1);
  - POZE, the fee for renewables: 0 all of 2026, as the state pays it ([cenový výměr 15/2025](https://eru.gov.cz/sites/default/files/obsah/prilohy/erv192025.pdf), of 29 December 2025). ERÚ's later change to the same price measure, in April 2026, concerns only local distribution systems.

  The suppliers' 2026 price lists show the same tax and system services, and Electree's and Yello's show POZE at 0. Monthly, so not in it: the breaker fee, the supplier's fee and the non-network infrastructure fee (12.87 Kč, for OTE, the data centre and ERÚ). OTE's 2.59 Kč per MWh traded (point 7.4) is charged to suppliers, so it's inside their margin.
- **October to December 2025:** the same charges cost more. System services were 170.92 Kč/MWh, and POZE was 495 Kč/MWh, the cap in Act 165/2012 Sb., § 28a(2), which every home paid, as the price per breaker, 84.70 Kč/A a month for each phase, came out far higher ([ERÚ](https://eru.gov.cz/sites/default/files/obsah/prilohy/erv132024.pdf)). Their sum with VAT was 0.84001, 0.60703 more than in 2026.
- **Margins:** what the suppliers add to the market price per kWh; the fixed adders are the same in the high and the low tariff:
  - ČEZ Prodej, Elektřina SPOT, from 1 January 2026 and the same since 1 October 2025: 0.39900 without VAT, 0.48279 with it, and 128 Kč a month ([price list](https://www.cez.cz/webpublic/file/edee/ceniky/moo_ele_cez_spot_01012026_12026.pdf));
  - bezDodavatele, SPOT, from 1 January 2026: 0.39000 without VAT, 0.47190 with it (0.35000 with a control system it supports), and 4.18 Kč a day with a two-rate tariff ([price list](https://www.bezdodavatele.cz/wp-content/uploads/2026/01/26_01_EL_Cenik_DOM_Spot_info.pdf));
  - Electree, HOME SPOT-A, from 1 September 2025: 0.34900 without VAT, 0.42229 with it, and 99 Kč a month ([price list](https://electree.cz/wp-content/uploads/2026/06/Elektrina-HOME-SPOT-A.pdf)); its HOME ELECTREE DRIVE for EV owners, from 16 April 2026: 0.31900 without VAT, 0.38599 with it, and 130 Kč a month ([price list](https://electree.cz/wp-content/uploads/2026/06/Elektrina-HOME-ELECTREE-DRIVE.pdf));
  - Delta Green, Delta SPOT, from 1 June 2026: 0.30000 without VAT, 0.36300 with it, and 159 Kč a month; its SPOT from 1 January 2026: 0.35000 without VAT, 0.42350 with it, and 129 Kč a month ([price list](https://cdn.prod.website-files.com/63072b68a235768212bfaa95/6a16ab2e98aeb497cc7b26b9_Delta%20Green%20-%20Cen%C3%ADk%20Delta%20SPOT%20-%20%C4%8CEZ%20-%20Dom%C3%A1cnosti.pdf));
  - Yello, PRE's brand, Yello Spot, from 1 January 2026, adds 15% to the market price instead (takes 85% of a negative one), and 199 Kč a month ([price list](https://www.yello.cz/files/ceniky/yello-spot-cenik/)).

  Worked out from these, the middle of four fixed adders is about 370 Kč/MWh, 0.44770 per kWh with VAT.
- **The plan:** D27d, the rate for EV owners, from ČEZ Distribuce, the largest operator. ERÚ gives D27d to anyone who shows they own or lease an EV, with a low tariff of at least 8 hours a day between 18:00 and 08:00, set by the operator ([cenový výměr 14/2025](https://eru.gov.cz/sites/default/files/obsah/prilohy/erv182025.pdf), D 27d). For supply points with interval metering, which a dynamic contract needs, ČEZ Distribuce fixes D27d's low tariff at 02:00 - 06:00 and 20:00 - 24:00 every day ([variant EMOV1](https://www.cezdistribuce.cz/file/edee/distribuce/cezdistribuce_pasmaplatnostintavt_prubehove_mereni.pdf)). With ripple control (HDO), the hours differ by place. Its fees in 2026 are nt 116.50 and vt 2,252.45 Kč/MWh without VAT, 0.14097 and 2.72546 per kWh with it, and from October to December 2025 they were nt 0.24926 and vt 2.74320 with VAT ([ERÚ](https://eru.gov.cz/sites/default/files/obsah/prilohy/erv142024.pdf)). Its breaker fee is D02d's (255 against 256 Kč a month at 3x25 A). Worked out from these and cz/cez-d02d, D27d pays with an EV: 3,300 kWh charged in the low tariff cost about 7,800 Kč a year less than on D02d, while the rest of the home pays 0.21038 more per kWh. On one rate, D02d, the plan is cz/cez-d02d.
- **The plans' fees:** all nine plans' prices match cenový výměr 14/2025 times 1.21, to 5 decimals.
- **Market prices:** spot contracts bill OTE's day-ahead price for the Czech area, set per quarter-hour in euros since 1 October 2025, converted at the ČNB's middle rate of the day; the board converts at the ECB's. SMARD's series 261 has Czechia's quarter-hour day-ahead prices in euros per MWh, free and without an account, a week a file, named by its Monday's 00:00 Prague time in ms, like [the week from Monday 29 September 2025](https://www.smard.de/app/chart_data/261/DE/261_DE_quarterhour_1759096800000.json), and listed in https://www.smard.de/app/chart_data/261/DE/index_quarterhour.json. They are OTE's prices: both have 97.21 EUR/MWh for 1 October 2025, 00:00 - 00:15 ([OTE's day](https://www.ote-cr.cz/pubweb/attachments/01/2025/month10/day01/DT_15MIN_01_10_2025_CZ.xlsx)). The files have every quarter-hour from 1 October 2025 to the morning of 1 October 2026.

## Changes ahead

- **1 January each year:** the operators' new prices, from ERÚ's price decision at the end of November. System services and POZE come from ERÚ's price decisions too, and both changed on 1 January 2026.
- **Smart meters, by 2030:** ČEZ Distribuce's SmartME project installs about 1.03 million smart meters ([ČEZ Distribuce](https://www.cezdistribuce.cz/cs/dotacni-projekty/cez-distribuce-smartme-inteligentni-mereni)), and the national plan aims at 2 million by 2030; the research didn't record where that's from.

## Open questions

- **How many homes pay the market price:** no figures on households with a spot contract, nor on EV owners with D27d rather than D02d or a heating rate with its own low tariff (D25d, D26d, D56d, D57d). ERÚ's reports page refused access.
- **D27d on smart meters:** whether smart meters (C1 to C3) on D27d get EMOV1's hours or the place's HDO command, as ČEZ's sheet names types A and B, and whether EMOV1 applied from October to December 2025 too.
- **E.ON's and innogy's offers:** their dynamic offers for households weren't found, though suppliers that large must have one.
- **Delta Green's and Yello's billing:** whether they bill quarter-hours or hours since October 2025: their price lists say hours.
- **The margin:** it differs by supplier: four offers from 0.36300 to 0.48279 with VAT, with monthly fees of 99 to 199 Kč left out.

## Notes

- **Not done yet:** D27d and the other rates whose low-tariff hours ripple control sets per place, left out of plans/cz in October 2026. D27d's EMOV1 variant for interval meters has fixed hours (The plan, above), which the example holds as a custom plan.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [ERÚ's cenový výměr 14/2025](https://eru.gov.cz/sites/default/files/obsah/prilohy/erv182025.pdf) | the operators' distribution fees, D27d's and the nine plans' | each 1 January, by ERÚ's decision at the end of November |
| [ČEZ Distribuce's hours for interval meters](https://www.cezdistribuce.cz/file/edee/distribuce/cezdistribuce_pasmaplatnostintavt_prubehove_mereni.pdf) | D27d's fixed low-tariff hours (EMOV1) | not known |
| [ERÚ's cenový výměr 13/2025](https://eru.gov.cz/sites/default/files/obsah/prilohy/erv172025.pdf) | system services | each 1 January |
| [ERÚ's cenový výměr 15/2025](https://eru.gov.cz/sites/default/files/obsah/prilohy/erv192025.pdf) | POZE | each 1 January |
| Act 261/2007 Sb., part 47 | the electricity tax | when the Act changes |
| The suppliers' price lists: [ČEZ Prodej](https://www.cez.cz/webpublic/file/edee/ceniky/moo_ele_cez_spot_01012026_12026.pdf), [bezDodavatele](https://www.bezdodavatele.cz/wp-content/uploads/2026/01/26_01_EL_Cenik_DOM_Spot_info.pdf), [Electree](https://electree.cz/wp-content/uploads/2026/06/Elektrina-HOME-SPOT-A.pdf), [Delta Green](https://cdn.prod.website-files.com/63072b68a235768212bfaa95/6a16ab2e98aeb497cc7b26b9_Delta%20Green%20-%20Cen%C3%ADk%20Delta%20SPOT%20-%20%C4%8CEZ%20-%20Dom%C3%A1cnosti.pdf), [Yello](https://www.yello.cz/files/ceniky/yello-spot-cenik/) | the margins and monthly fees | any time |
| [SMARD's series 261](https://www.smard.de/app/chart_data/261/DE/index_quarterhour.json) | the market prices, a week a file | daily |
| [OTE's day-ahead results](https://www.ote-cr.cz/pubweb/attachments/01/2025/month10/day01/DT_15MIN_01_10_2025_CZ.xlsx) | the prices spot contracts bill, a day a file, to check SMARD's against | daily |
