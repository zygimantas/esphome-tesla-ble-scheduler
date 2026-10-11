# plans/hr/

Croatia's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On HEP Elektra's regulated price, with HEP ODS's Bijeli fees and the renewables fee, the board's charging cost 337.57 EUR over the year from 1 October 2025: 49% less than charging at once (663.43 EUR), and the same as the car's own schedule from 22:00 (337.57 EUR), which is also its best start, as every night quarter-hour costs the same. Confidence in the README is High: no home pays the market price, and the regulated one is cheaper at night.

## Decisions

- **Contract:** the regulated price, as no home pays the market price: HEP Elektra's universal service price, with HEP ODS's Bijeli network fees and the renewables fee.
- **Plan:** a custom plan with all three: 0.10127 EUR per kWh with VAT at night and 0.19903 by day, the night from 21:00 winter time (22:00 on the clock in summer). The car's own schedule starts at 22:00.
- **Margin:** none, as no home pays the market price.
- **The same every hour:** the renewables fee, 0.01496 EUR per kWh with VAT, in the custom plan's rates.
- **Left out:** the monthly fees, which don't depend on when the car charges; the 35% more on a half-year's kWh above 3,000; the lower prices of October to December 2025, as 2026's are used all year.
- **Market prices:** none, as none reach a home's bill.

## Findings

- **Who pays the market price:** no home. [HERA's annual report for 2025](https://www.hera.hr/hr/docs/HERA_izvjesce_2025.pdf) (table 3.1.5) counts 0 homes with a contract that follows the market in every year from 2021 to 2025. Five suppliers offered such contracts in 2025, to 21,091 metering points, none of them a home's, and the biggest market supplier, HEP-Opskrba, offered none and had no requests.
- **The universal service:** in December 2025, 90% of the electricity homes bought came from HEP Elektra's universal service (univerzalna usluga), at prices the government sets by decree for six months at a time. The rest came from market suppliers, none of them by the market (HEP-Opskrba's Hepi sells two-rate and one-rate prices), whose prices the decree caps at the same level (art. 3 of NN 101/2026, as art. 4 of the decree before it did). So CROPEX's price for the HR area reaches no home's bill, and the regulated energy price takes its place.
- **VAT:** 13% on electricity and the fees that come with it, the network fees and the renewables fee included. Art. 38(3) of the VAT act has listed electricity at 13% since 1 January 2017 ([NN 115/2016](https://narodne-novine.nn.hr/clanci/sluzbeni/2016_12_115_2526.html)), and HEP Elektra's price table of 1 November 2025, still current in October 2026, says the same. No change from October 2025 to September 2026: the VAT amendments of 2025 and 2026 (NN 52/25, NN 32/26) extended the 5% rate on natural gas.
- **Per kWh, the same every hour:** only the renewable energy and cogeneration fee (naknada za OIE), 0.01324 EUR without VAT and 0.01496 with it, for every home since 1 March 2024 ([NN 24/2024](https://narodne-novine.nn.hr/clanci/sluzbeni/2024_02_24_381.html)), and still on HEP Elektra's price page in October 2026. Homes pay no electricity excise: art. 105(8) of the excise act ([NN 106/2018](https://narodne-novine.nn.hr/clanci/sluzbeni/2018_11_106_2058.html)) exempts electricity used in homes, so the 1.00 EUR per MWh rate for non-business use (NN 156/2022) isn't charged, as the Customs Administration's tax expenditure report for 2022 confirms.
- **Monthly fees, left out:** per month without VAT, HEP Elektra's supply fee, 0.982 EUR, and HEP ODS's metering point fee, 1.983 EUR in 2026 and 1.724 in 2025.
- **Energy price:** HEP Elektra's regulated price for Bijeli (and Crveni), for the first 3,000 kWh a metering point uses in each half-year (October - March, April - September): 0.09719 EUR per kWh by day (VT) and 0.04769 at night (NT) without VAT, 0.10982 and 0.05389 with it, from 1 November 2025 to at least 31 March 2027 (the decrees in [NN 121/2025](https://narodne-novine.nn.hr/clanci/sluzbeni/2025_09_121_1723.html), with its correction in NN 122/2025, [NN 29/2026](https://narodne-novine.nn.hr/clanci/sluzbeni/2026_03_29_336.html) and [NN 101/2026](https://narodne-novine.nn.hr/clanci/sluzbeni/2026_09_101_1209.html); [HEP Elektra's table](https://www.hep.hr/elektra/kucanstvo/tarifne-stavke-cijene/1547)). Night is 21:00 - 07:00 in winter time and 22:00 - 08:00 in summer time, every day. October 2025 cost less: 0.08451 and 0.04147 without VAT, 0.09550 and 0.04686 with it.
- **Above 3,000 kWh:** the rest of a half-year's use costs 35% more (50% until September 2025): 0.14826 EUR per kWh by day and 0.07275 at night with VAT. An average Bijeli home uses 3,258 kWh a year (HERA, table 3.1.4), and the car adds about 1,660 kWh a half-year. Worked out from these, such a home goes over by about 300 kWh a half-year, more in winter.
- **The plan:** `hr/hep-ods-bijeli`, the two-rate model of 71% of homes (1,668,164 of 2,349,726 metering points in 2025) and of 78% of their electricity (HERA, table 3.1.4), and the one for an owner who charges at night. Its lower fee starts at 21:00 in winter time and 22:00 on the clock in summer (the plan's `clock: winter`), every day, with no weekend or holiday rule ([HEP ODS's tariff models](https://www.hep.hr/ods/kupci/kucanstvo/tarifni-modeli/34)). Its rates match 2026's on HEP ODS's sheet and in HERA's decisions (NN 150/2025): 0.06570 and 0.02869 EUR per kWh without VAT, 0.07424 and 0.03242 with it; Plavi's and Crveni's match too. They stay until at least 31 March 2027, as NN 101/2026 (art. 9) bars the operators from asking for new fees before then. October to December 2025 had 2025's fees, which 2026 raised by about 13%: 0.06551 by day and 0.02856 at night with VAT ([2025's sheet](https://www.hep.hr/ods/UserDocsImages/dokumenti/NTS01012025/kucanstvo_NTS_01012025.pdf)). Crveni, over 22 kW, has the same energy price and a fee per kW.
- **Prices for cars:** neither HEP ODS nor HEP Elektra has one. Crni, with hours switched remotely on a meter of its own for storage heaters and water heaters, exists only in parts of the Pula and Dubrovnik areas.
- **The example's rates:** worked out from these, energy and network fee together are 0.18407 EUR per kWh by day and 0.08631 at night with VAT, and with the renewables fee 0.19903 and 0.10127 in 2026 (0.17597 and 0.09037 in October 2025, 0.19029 and 0.09740 in November and December). Every night quarter-hour costs the same, so the board saves 49% against charging at once from 18:00, all in day hours, and nothing against the car's own schedule from 22:00.
- **Market prices:** none are needed for the bill. CROPEX's quarter-hour prices for the HR area, in EUR per MWh, come without a key from Energy-Charts, a day or a month per call, the end included: a request for [1 October 2025](https://api.energy-charts.info/price?bzn=HR&start=2025-10-01T00:00%2B02:00&end=2025-10-01T23:45%2B02:00) gives its 96 quarter-hours, and October 2025 has all 2,980, with 100 on 26 October. They match CROPEX's monthly report for March 2026 (average base price 110.12 EUR per MWh, lowest day 59.26, highest 154.81).

## Changes ahead

- **31 March 2027:** the universal service's price holds until then at least; the government sets it by decree for six months at a time.
- **April 2027 at the earliest:** new network fees, by HERA's decisions in Narodne novine, as NN 101/2026 (art. 9) bars the operators from asking for new ones before then.

## Open questions

- **Above 3,000 kWh:** how HEP Elektra splits the kWh over 3,000 between day and night: the decree only says they cost 35% more.
- **A dynamic contract:** how the decree's cap would apply to one, as no home has one.
- **After 31 March 2027:** the energy price and the network fees.

## Notes

- **Energy-Charts:** it answered Too Many Requests to a few calls a minute, and its license says private and internal use only.
- **CROPEX:** its own API needs a token, and its site forbids automatic extraction.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [HEP Elektra's prices](https://www.hep.hr/elektra/kucanstvo/tarifne-stavke-cijene/1547) | the universal service's energy price, the renewables fee, the supply fee | every six months, by the government's decree |
| [The government's decree, NN 101/2026](https://narodne-novine.nn.hr/clanci/sluzbeni/2026_09_101_1209.html) | the universal service's price to 31 March 2027, the cap on market suppliers' prices, the bar on new network fees | every six months |
| [HEP ODS's prices](https://www.hep.hr/ods/korisnici/kucanstvo/tarifne-stavke-cijene/160) | the plans' fees, without VAT | April 2027 at the earliest |
| HERA's decisions in Narodne novine (NN 150/2025 for 2026) | the network fees | April 2027 at the earliest |
| [HERA's annual report](https://www.hera.hr/hr/docs/HERA_izvjesce_2025.pdf) | homes with a contract that follows the market, metering points and energy by tariff model | each year |
| [Energy-Charts' prices for HR](https://api.energy-charts.info/price?bzn=HR&start=2025-10-01T00:00%2B02:00&end=2025-10-01T23:45%2B02:00) | CROPEX's prices, a day or a month a request | daily |
