# plans/ro/

Romania's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a dynamic contract billing each quarter-hour's price, the board's charging cost 4,898.68 RON (949.16 EUR) over the year from 1 October 2025: 18% less than charging at once (5,954.76 RON, 1,155.45 EUR), 14% less than the car's own schedule from midnight (5,704.34 RON, 1,104.39 EUR) and 6% less than its best start, 04:30 (5,221.64 RON, 1,011.39 EUR). Confidence in the README is Low: no home had a dynamic contract in 2025.

## Decisions

- **Contract:** a dynamic contract billing OPCOM's day-ahead price by the quarter-hour, like ENGIE's Elec Flexibil, though no home had one in 2025.
- **Plan:** `ro/deer-jt`, the same fee at every hour, so the car's own schedule starts at midnight.
- **Billing:** each quarter-hour at its own price.
- **Margin:** 0.242 RON per kWh with VAT, ENGIE's supply component.
- **The same every hour:** 0.17847 RON per kWh with VAT: transmission, system services, green certificates, cogeneration, excise and the CfD contribution.
- **Left out:** some fixed offers' daily fees, as they don't depend on when the car charges.
- **Market prices:** Energy-Charts' RO prices in euros, converted at the ECB's daily rate. Its terms allow private use only, and it answers quick repeats with 429.

## Findings

- **Who pays the market price:** no household. The energy law (Legea 123/2012, art. 62 (1) i^1) gives anyone with a working smart meter the right to a dynamic contract from any supplier with over 200,000 customers: in 2025 Electrica Furnizare, E.ON Energie România, PPC Energie, Premier Energy and Hidroelectrica. [ANRE's report on 2025](https://anre.ro/wp-content/uploads/2026/07/Raport-2025-monitorizare-CPD.pdf), of July 2026, counted 288 customers on dynamic contracts, all businesses, and no household. Households almost all pay one price for every hour: in June 2026, 73% of the 8.82 million household customers had a competitive offer and 27% universal service ([ANRE's monthly report](https://anre.ro/wp-content/uploads/2026/09/Monit-iun26.pdf)). The price cap, 0.68 to 1.30 lei/kWh with VAT, ended on 30 June 2025 (OUG 6/2025, [PPC](https://www.ppcenergy.ro/energie-electrica/plafonare/)).
- **Meters and billing:** a dynamic contract needs the meter in the operator's smart metering system (SMI), which reached 34.5% of low-voltage users at the end of 2025 and about 14% in DEER's area ([ANRE's report on smart metering](https://anre.ro/wp-content/uploads/2026/04/RAPORT_implem_SMI_cf_CISMI_2019-2028_rezultate_la_31_12_2025.pdf)). The offers bill OPCOM's day-ahead price (PZU) in lei for each 15-minute interval the meter records, plus a supply component and the charges below. PPC MaxControl splits an hourly reading evenly over its quarter-hours.
- **VAT:** 21% since 1 August 2025, up from 19% ([Legea 141/2025](https://static.anaf.ro/static/10/Anaf/legislatie/L_141_2025.pdf), changing art. 291 of the Fiscal Code). It stayed 21% from October 2025 to September 2026 and still is in October 2026. Electricity has no reduced rate.
- **Per kWh, the same every hour:** 0.17847 lei with VAT from 1 July 2026 (0.14749 without), and 0.17738 with the cogeneration contribution of January to June 2026, checked against [Electrica's list of regulated fees](https://www.electricafurnizare.ro/asistenta/tarife-reglementate-transport-distributie-energie-electrica-contributie-cogenerare-energie-reactiva/), Transelectrica's report of 19 December 2025 and ENGIE's monthly offers. 2026's, in lei per kWh without and with VAT:
  - Transelectrica's fee for taking power from the grid (TL), 0.03645 and 0.04410 ([ANRE Order 74/2025](https://anre.ro/wp-content/uploads/2026/01/Ord-74-2025.pdf));
  - system services, 0.01470 and 0.01779 ([Order 73/2025](https://anre.ro/wp-content/uploads/2026/01/Ord-73-2025.pdf));
  - green certificates, 0.07402 and 0.08956: ANRE's quota of 0.499387 certificates per MWh ([Order 81/2025](https://anre.ro/wp-content/uploads/2026/01/Ord-81-2025.pdf)) times the previous month's certificate price, as ENGIE and Electrica billed them all year;
  - the high-efficiency cogeneration contribution, 0.01450 and 0.01755 from 1 July 2026 ([Order 49/2026](https://anre.ro/wp-content/uploads/2026/08/Ord-49-2026.pdf)), 0.01360 and 0.01646 from 1 November 2025 (Order 66/2025);
  - excise for non-business use, 0.00768 and 0.00929: the Fiscal Code's 4.74 lei/MWh (annex 1 of title VIII) raised with inflation every January (art. 342), as [Digi Energy](https://www.digi.ro/servicii/digi-energy/tarife) lists it;
  - the contribution for contracts for difference (CfD), 0.00014 and 0.00017 (0.000144 exactly, [Order 69/2025](https://anre.ro/wp-content/uploads/2025/12/Ord-69-2025.pdf));
  - not Transelectrica's grid injection fee (TG), which counts as part of the market price and isn't added.
- **Per kWh in October to December 2025:** less, without VAT: TL 0.03303, system services 0.01279, green certificates 0.07254, cogeneration 0.00840 in October and 0.01360 from November, excise 0.00724 and CfD 0.00021, in all 0.16239 with VAT in October and 0.16868 in November and December.
- **Margins:** the dynamic offers are ENGIE's Elec Flexibil, Electrica Dinamic, Hidroelectrica's Hidro Dinamic C, Premier Energy's PE Dinamic and, for prosumers only, PPC MaxControl. Some fixed offers add a daily fee, and the dynamic ones found don't. Per kWh:
  - ENGIE's supply component (CF), 0.20000 lei without VAT on top of the market price, 0.24200 with VAT, the same in its Elec Flexibil offers from January to September 2026, with no monthly fee ([the offer of 23 September 2026](https://www.engie.ro/wp-content/uploads/2026/09/Oferta-Elec-Flexibil-B2C.pdf));
  - Premier Energy's PE Dinamic of December 2025, 0.19000 plus an estimated 0.15000 for imbalances, without VAT, 0.41140 with VAT, no fee, and the customer forecasts each quarter-hour ([the offer](https://www.premierenergy.info/wp-content/uploads/2025/12/Premier-Oferta-Dinamic-_casnic-OEDC_12_2025.pdf));
  - PPC MaxControl of October 2026, 0.09500 without VAT, 0.11495 with VAT, with the market price held between 0 and 0.999 lei/kWh ([the offer](https://www.ppcenergy.ro/wp-content/uploads/oferta-ppc-maxcontrol-rezidential.pdf));
  - Hidroelectrica's Hidro Dinamic C at its launch in November 2023, 0.07300 plus 0.14000 for imbalances, as a news site reported ([e-nergia.ro](https://e-nergia.ro/hidroelectrica-a-lansat-oferta-de-energie-electrica-pentru-populatie-cu-pret-dinamic-care-se-poate-schimba-de-la-o-ora-la-alta)).
- **Cheap hours instead:** one supplier sells them: PPC's Ore Smart+ (smart meter, no prosumers) charges 0.36100 lei/kWh before network fees, taxes and VAT at weekends and on weekdays 00:00 - 06:00, also 11:00 - 15:00 from March to October, and 0.85700 at other times ([the offer](https://www.ppcenergy.ro/wp-content/uploads/oferta-ppc-ore-smart.pdf), October 2026).
- **The plan:** `ro/deer-jt`. Worked out from the smart metering report, DEER serves the most homes: about 4.0 of Romania's 10.4 million low-voltage users (39%), against 3.2 million for Rețele Electrice, 1.6 for Delgaz Grid and 1.5 for Oltenia. Its fee is the same in every hour, as Romania has no time-of-use network fees, so no cheap fee starts at night, and the grid charges homes no monthly or per-kW fee. Its 0.42996 lei/kWh with VAT, the sum of 31.96, 83.36 and 240.02 lei/MWh without VAT, one for each voltage level ([ANRE Order 77/2025](https://anre.ro/wp-content/uploads/2026/01/Ord-77-2025.pdf)), matches DEER's page and 2026's prices, unchanged through October 2026 (ENGIE's September and PPC's October offers still use 0.35534 without VAT). In 2025 it was 0.42463 with VAT (350.93 lei/MWh).
- **The car's own schedule:** it has no fee to follow. OPCOM's prices fall after 21:00, and, worked out from 73 sample days, every fifth day from October 2025 to September 2026, the cheapest night hour began around 04:15 on average.
- **Market prices:** OPCOM's day-ahead results, a CSV for each delivery day, with no account, at https://www.opcom.ro/rapoarte-pzu-raportPIP-export-csv/DD/MM/YYYY/ro?resolution=15, like [1 October 2025's](https://www.opcom.ro/rapoarte-pzu-raportPIP-export-csv/01/10/2025/ro?resolution=15). Rows `"Romania","n","price"` give lei per MWh for quarter-hour n, counted from 00:00 CET, which is 01:00 in Romania; clock-change days have 92 or 100. The answer's type is text/html, and later files add a Rezolutie column. It's the price the dynamic contracts bill: ENGIE's list of August 2026 prices repeats OPCOM's, and Nord Pool's TEL prices in RON, which the board downloads, matched OPCOM's within 0.03 lei/MWh on 15 September 2026. [Energy-Charts](https://api.energy-charts.info/price?bzn=RO&start=2025-10-01T00:00%2B02:00&end=2025-10-01T23:45%2B02:00) has the same prices in euros, which OPCOM converts at one rate a day (5.0783 for 1 October 2025, as the ECB's of 29 September).

## Changes ahead

- **1 October 2026:** system services drop to 0.01272 (Digi's page, PPC's October offers), after the year the example studies.
- **1 January each year:** usually ANRE's new prices for the operators and Transelectrica, and the excise, raised with inflation (Fiscal Code art. 342).
- **Each month:** the green certificate charge, at the previous month's certificate price; each year's is settled again the next September.
- **Any time:** the cogeneration contribution, by ANRE's order, as on 1 November 2025 and 1 July 2026.

## Open questions

- **Homes on dynamic contracts:** none in 2025; ANRE's next yearly report on them would say whether homes take them up.
- **Other margins:** E.ON's and Hidroelectrica's current margins are unknown, as their sites refused automated requests, and so is Electrica's, as it publishes its dynamic contract with the supply component left blank.
- **ENGIE's margin in July and August 2026:** those months' offers don't print the 0.20000.
- **Green certificates:** suppliers billed 0.07402 all through 2026, but the charge follows the certificate price month by month and is settled again the next September.
- **System services from 1 October 2026:** the new fee isn't on ANRE's list of orders yet.

## Notes

- **Rețele Electrice's prices:** ANRE's orders have a table for each operator, and Rețele Electrice's own page shows DEER's, the first in Order 78/2025: its own is on the last page.
- **Sites that refuse scripts:** E.ON's and Hidroelectrica's refused automated requests, and Energy-Charts answered repeated ones with HTTP 429.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| ANRE's orders for [DEER](https://anre.ro/wp-content/uploads/2026/01/Ord-77-2025.pdf), [Distribuție Energie Oltenia](https://anre.ro/wp-content/uploads/2026/01/Ord-75-2025.pdf), [Delgaz Grid](https://anre.ro/wp-content/uploads/2026/01/Ord-76-2025.pdf) and [Rețele Electrice](https://anre.ro/wp-content/uploads/2026/01/Ord-78-2025.pdf) | the plans' fees, a table for each operator | usually each 1 January |
| [DEER's prices](https://www.distributie-energie.ro/tarife/) | DEER's fee, as a check | usually each 1 January |
| ANRE's Orders [74/2025](https://anre.ro/wp-content/uploads/2026/01/Ord-74-2025.pdf), [73/2025](https://anre.ro/wp-content/uploads/2026/01/Ord-73-2025.pdf), [81/2025](https://anre.ro/wp-content/uploads/2026/01/Ord-81-2025.pdf), [49/2026](https://anre.ro/wp-content/uploads/2026/08/Ord-49-2026.pdf) and [69/2025](https://anre.ro/wp-content/uploads/2025/12/Ord-69-2025.pdf) | TL, system services, the green certificate quota, the cogeneration contribution and the CfD contribution | each year, and the cogeneration contribution any time |
| [Electrica's list of regulated fees](https://www.electricafurnizare.ro/asistenta/tarife-reglementate-transport-distributie-energie-electrica-contributie-cogenerare-energie-reactiva/) and [Digi Energy's prices](https://www.digi.ro/servicii/digi-energy/tarife) | the charges per kWh as suppliers bill them, the excise and the new system services fee included | any time |
| [ENGIE's Elec Flexibil offer](https://www.engie.ro/wp-content/uploads/2026/09/Oferta-Elec-Flexibil-B2C.pdf) | the margin | each month |
| [ANRE's report on dynamic contracts](https://anre.ro/wp-content/uploads/2026/07/Raport-2025-monitorizare-CPD.pdf) | customers on dynamic contracts | each year, around July |
| [ANRE's monthly report](https://anre.ro/wp-content/uploads/2026/09/Monit-iun26.pdf) | household customers by competitive offer and universal service | each month |
| [ANRE's report on smart metering](https://anre.ro/wp-content/uploads/2026/04/RAPORT_implem_SMI_cf_CISMI_2019-2028_rezultate_la_31_12_2025.pdf) | smart meters and low-voltage users by operator | each year |
| [Energy-Charts](https://api.energy-charts.info/price?bzn=RO&start=2025-10-01T00:00%2B02:00&end=2025-10-01T23:45%2B02:00) | the market prices in euros | daily |
| [OPCOM's day-ahead results](https://www.opcom.ro/rapoarte-pzu-raportPIP-export-csv/01/10/2025/ro?resolution=15) | the market prices in lei, a day a file | daily |
