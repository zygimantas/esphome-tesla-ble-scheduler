# plans/be/

Belgium's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a dynamic contract in Flanders billing Belgium's day-ahead price, the board's charging cost 791.67 EUR over the year from 1 October 2025: 15% less than charging at once (928.14 EUR), 7% less than the car's own schedule from midnight (847.33 EUR) and 1% less than its best start, 04:00 (799.78 EUR). Confidence in the README is Low: 0.4% of homes pay the market price, and Flanders' fee per kW isn't counted.

## Decisions

- **Contract:** a dynamic contract billing Belgium's day-ahead price, in Flanders.
- **Plan:** `be/fluvius-imewo`, Fluvius' largest area. Its fee is the same at every hour, so the car's own schedule starts at midnight.
- **Billing:** each hour at the average of its four quarter-hours, as Luminus, TotalEnergies, Eneco and Mega bill.
- **Margin:** 0.01676 EUR per kWh with VAT, the CREG's average over all dynamic products in 2025, with its coefficient on the market price taken at 86.08 EUR per MWh.
- **The same every hour:** 0.09530 EUR per kWh with VAT over the year, at 2026's values: the public service obligations, the surcharges, the federal excise (cut on 1 August 2026) and the certificates.
- **Left out:** the capacity tariff, which the board doesn't count ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)), and data management and the suppliers' fixed fees, which don't depend on when the car charges.
- **Market prices:** SMARD's series 4996.

## Findings

- **Who pays the market price:** dynamic contracts are offered widely and taken rarely. On 30 September 2025 households had 20,552 of them, 0.40% of about 5.2 million contracts: 20,453 in Flanders (0.70%), 93 in Brussels and 6 in Wallonia ([CREG RA3159](https://www.creg.be/sites/default/files/assets/Publications/Reports/RA3159NL.pdf), 15 January 2026). The VREG's market monitor 2025 agrees, with 0.7% of Flemish households in June 2025, but now answers 404. Three households in four have a variable price, indexed each month or quarter on the market (46% of all on spot indexes), and one in four a fixed price ([CREG F3188](https://www.creg.be/sites/default/files/assets/Publications/Studies/F3188NL.pdf), 12 March 2026). At the end of 2025, 16 suppliers offered a dynamic contract, among them the five with over 200,000 customers, which must: Engie, Luminus, Eneco, TotalEnergies and Mega.
- **Meters and billing:** a dynamic contract needs a digital meter that reports quarter-hours (SMR3). All bill Belgium's day-ahead price, one price in EPEX SPOT's and Nord Pool's coupled auction: by quarter-hour at Engie (for contracts from 1 October 2025) and Bolt, by hour (Belpex H) at Luminus, TotalEnergies, Eneco and Mega, as their October 2026 price sheets say. With the hourly ones, the quarter-hours within an hour cost the same.
- **VAT:** 6% for households all through October 2025 to September 2026, on energy, network fees and federal excise alike, permanent since 1 April 2023 (law of 19 March 2023 reforming the taxes on the energy bill); 21% for business use.
- **Per kWh, the same every hour:** in Fluvius Imewo, without and with VAT:
  - public service obligations, 0.02492 and 0.02641 EUR, and surcharges, 0.00144 and 0.00152 EUR, in 2026 ([Fluvius Imewo's 2026 sheet](https://www.fluvius.be/sites/fluvius/files/2025-11/distributienettarieven-elektriciteit-afname-fluvius-imewo-01012026-31122026.xlsx), whose kWh-tarief is the plan; transmission is included);
  - the federal excise: until 31 July 2026, as in October to December 2025, the special excise, 0.04748 EUR, and the energy contribution, 0.00193 EUR, together 0.04941 and 0.05237 EUR; from 1 August 2026 the special excise alone, 0.04600 and 0.04876 EUR, as the programme law of 30 May 2026 cut it, merged the energy contribution into it and dropped the brackets by consumption ([customs circular 2026/C/82](https://blog.forumforthefuture.be/nl/article/circulaire-2026c82-betreffende-energieproducten-en-elektriciteit/32514); Fluvius' sheets for [July](https://www.fluvius.be/sites/fluvius/files/2026-06/tarieven-fraude-en-zegelbreuk-elektriciteit-fluvius-antwerpen-01072026-30092026.pdf) and [August](https://www.fluvius.be/sites/fluvius/files/2026-07/tarieven-fraude-en-zegelbreuk-elektriciteit-fluvius-imewo-01082026-30092026.pdf); 0.04876 EUR with VAT on Engie's, Luminus', Eneco's, Mega's and Bolt's October 2026 sheets);
  - green power and CHP certificates, which every supplier passes on per kWh: 0.01472 and 0.01560 EUR in 2026, 0.01590 EUR with VAT in October to December 2025 ([Luminus' table](https://www.luminus.be/nl/prive/hulp-en-contact/hoe-worden-de-tarieven-voor-groene-stroom-en-wkk-bepaald-nl/)), and with VAT from 0.01510 EUR at Eneco to 0.01582 EUR at Engie in October 2026. They differ by supplier and change by quarter.

  Worked out from these, with VAT: 0.09591 EUR until July 2026 and 0.09230 EUR from August, so 0.09530 EUR over the 365 nights, 304 and 61. October to December 2025 had Fluvius' 2025 obligations and surcharges, 0.03635 EUR with VAT, so 0.10462 EUR. No charge is a share of the bill but VAT.
- **Fixed charges:** the Flemish Energiefonds levy, 0 for a home and 10.07 EUR a month without VAT for a second home or an extra meter in a garage ([Vlaanderen](https://www.vlaanderen.be/belastingen-en-begroting/vlaamse-belastingen/energieheffingen/bijdrage-energiefonds-heffing-op-afnamepunten-van-elektriciteit/tarief-van-de-bijdrage-energiefonds)); data management, 18.92 EUR a year; the suppliers' fixed fees, under Margins.
- **Capacity tariff:** Fluvius also bills the average of each month's highest quarter-hour, at least 2.5 kW: in Imewo 54.20098 EUR per kW a year without VAT, 57.45304 with it, in 2026 (53.24224 without VAT in 2025). The board doesn't count it ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). Charging at 11 kW can add several hundred euros a year, whenever the car charges, unless it charges slower.
- **Margins:** the CREG's average over all dynamic products in 2025 (RA3159) is a mark-up of 0.01273 EUR plus a coefficient of 1.0358 on the market price, and its average fixed fee was 60 EUR a year. With the 3.58% taken at 86.08 EUR per MWh, about what the board's quarter-hours cost (the four cheapest from 18:00 to 07:00, every night of the year), that's 0.01676 EUR with VAT (0.01581 without). Followed exactly, it's 9.795% on the market price with VAT (1.06 x 1.0358) and 0.01349 EUR. Six offers for Flanders in October 2026, with the adder without and with VAT, the coefficient and the fixed fee with VAT:
  - [Engie Dynamic](https://www.engie.be/api/engie/be/ms/pricing/v1/public/pricesAndConditionsPDF?document=E_DYNAMIC_R_GREY_C_I_12_V_N&monthOffset=0&segment=R&language=N): 0.00713 and 0.00756 EUR, 1.039 on the quarter-hour price, 100.70 EUR a year;
  - [Luminus Dynamic Online](https://www.luminus.be/api-next/get-pricelist?documentSlug=dynamic&energyType=electricity&language=nl&tabValue=Flanders): 0.01959 and 0.02077 EUR, 1.019 on the hourly price, 75 EUR;
  - [TotalEnergies myDynamic](https://totalenergies.be/static/marketing-documents/b2c/tariff-card/latest/MYDYNAMIC_ELECTRICITY_VL_NL.pdf): 0.03410 and 0.03615 EUR, which include the certificates' 0.01570 EUR with VAT, so 0.01929 and 0.02045 EUR without them, 1.031 hourly, 90 EUR;
  - [Eneco Zon & Wind Dynamisch](https://cdn.eneco.be/downloads/nl/general/tk/BC_032_012610_NL_ENECO_POWER_DYNAMIC.pdf): 0.00485 and 0.00514 EUR, 1.02 hourly, 15 EUR;
  - [Mega Dynamic](https://my.mega.be/resources/tarif/Mega-NL-EL-B2C-VL-102026-Dynamic0110.pdf): 0.00600 and 0.00636 EUR, 1.05 hourly, 85 EUR, waived the first year;
  - [Bolt Variabel Online](https://files.boltenergie.be/pricelists/var/online_res_el_nl_14.pdf) with its dynamic option: 0.01690 and 0.01791 EUR, 1.168 on the quarter-hour price, 5.99 EUR a month.

  Worked out at 86.08 EUR per MWh, they come to 0.00697 to 0.03324 EUR with VAT, Engie's 0.01112, with a median of 0.01681.
- **The plan:** Imewo (Ghent, Bruges, Ostend) is Fluvius' largest area, with 532,287 of the 2,676,055 low-voltage offtake points in September 2026 (19.9%), ahead of Antwerpen (18.4%) and West (15.2%) ([Fluvius' open data](https://opendata.fluvius.be/), datasets 1_17 and 1_23); the count includes small businesses on low voltage. Its fee, 0.02749 EUR with VAT, is the same every hour: there's no cheap fee at night. It was 0.02609 EUR in 2025.
- **The plans' fees:** all eight Fluvius plans match the 2026 sheets in their first line (the kWh-tarief for a digital meter, with 6%), valid all year, and October 2026's supplier sheets have the same totals.
- **Other contracts:** a day/night meter's night rate starts at 22:00 on weekdays and lasts the weekend, but only fixed and variable contracts price it lower. Engie's Empower Flextime, a variable contract with about 25,000 customers in under a year, prices energy lowest from 01:00 to 07:00 every day ([Engie](https://www.engie.be/nl/empower-flextime/)).
- **Wallonia:** ORES Bihoraire is off-peak 22:00 - 07:00 and 11:00 - 17:00 every day, so its cheap fee starts at 22:00 (Impact's ECO at 01:00), at 0.07390 against 0.13270 EUR at peak for the whole network fee, plus 0.02740 EUR transport and 0.03030 EUR certificates, with VAT; but only 6 households there had a dynamic contract.
- **Market prices:** SMARD's series 4996 (Belgium), a week a file from Monday 00:00 German time, in EUR per MWh, like https://www.smard.de/app/chart_data/4996/DE/4996_DE_quarterhour_1766962800000.json (29 December 2025 to 4 January 2026), with the weeks in https://www.smard.de/app/chart_data/4996/DE/index_quarterhour.json. It has all 35,040 quarter-hours from 1 October 2025 to 30 September 2026 and matched Nord Pool's BE prices, which the board downloads, on all 96 quarter-hours of 15 September 2026; Nord Pool wants an account for older days (401 for 15 November 2025). It's the price dynamic contracts bill; the hourly ones bill the hour's value.

## Changes ahead

- **Each quarter:** the certificates, which differ by supplier.
- **1 January each year:** the operators' new prices, which the regulators approve: the Vlaamse Nutsregulator for Fluvius, the CWaPE for ORES and RESA, and Brugel for Sibelga. Fluvius' sheets bring new public service obligations, surcharges and capacity tariffs with them.
- **2028:** Brussels' fees change structure: Brugel brings a fee by the connection's power in 2028 and three periods a day in 2030.

## Open questions

- **The excise in the law:** the law's own rates weren't read, as the government's and the Chamber's sites refused automated requests. 46.00 EUR per MWh from August is Fluvius' figure without VAT and five suppliers' with it, and 47.48 EUR before is Fluvius' figure to three decimals.
- **Belpex H:** whether it's the mean of the hour's four quarter-hours since October 2025.
- **Dynamic contracts since September 2025:** no count after the CREG's of 30 September 2025.

## Notes

- **Operators' own sites:** RESA's own page renders its prices with scripts, so its plans come from the CWaPE site, and Sibelga's site refuses automated requests, so its plans come from Brugel's decisions.
- **TotalEnergies' sheet:** its adder includes the certificates (Margins), and it still lists the old 0.20-cent energy contribution.
- **The margin's coefficient:** folding it in at the board's price understates what charging at once pays (at 131.03 EUR per MWh, 0.00171 EUR more per kWh with VAT), so the example slightly understates the saving.
- **Not done yet:** plans for AIEG, AIESH and REW, the small Walloon operators.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Fluvius' grid fees](https://www.fluvius.be/nl/factuur-en-tarieven/netkosten/netkosten-afname-elektriciteit) | each area's tariff list: the plans' kWh-tarief, the public service obligations, the surcharges and the capacity tariff | each 1 January |
| [ORES's 2026 prices](https://media.ores.be/ores-cms/ptefg3n3/ed_pr%C3%A9l%C3%A8vement_2026.pdf) | the ORES plans, which the CWaPE site has too | each 1 January |
| [RESA's 2026 prices, on the CWaPE site](https://www.cwape.be/sites/default/files/cwape-documents/RESA%20-%20Tarifs%20p%C3%A9riodiques%20de%20pr%C3%A9l%C3%A8vement%202026.pdf) | the RESA plans | each 1 January |
| [Brugel's decision on Sibelga's 2026 prices](https://brugel.brussels/publication/document/decisions/2025/fr/DECISION-370-ADAPTATIONS-TARIFAIRES-2026-SIBELGA.pdf) | the Sibelga plans | each 1 January |
| [CREG RA3159](https://www.creg.be/sites/default/files/assets/Publications/Reports/RA3159NL.pdf) | the count of dynamic contracts, their average margin and fixed fee | with each report |
| The suppliers' sheets: [Engie](https://www.engie.be/api/engie/be/ms/pricing/v1/public/pricesAndConditionsPDF?document=E_DYNAMIC_R_GREY_C_I_12_V_N&monthOffset=0&segment=R&language=N), [Luminus](https://www.luminus.be/api-next/get-pricelist?documentSlug=dynamic&energyType=electricity&language=nl&tabValue=Flanders), [TotalEnergies](https://totalenergies.be/static/marketing-documents/b2c/tariff-card/latest/MYDYNAMIC_ELECTRICITY_VL_NL.pdf), [Eneco](https://cdn.eneco.be/downloads/nl/general/tk/BC_032_012610_NL_ENECO_POWER_DYNAMIC.pdf), [Mega](https://my.mega.be/resources/tarif/Mega-NL-EL-B2C-VL-102026-Dynamic0110.pdf) and [Bolt](https://files.boltenergie.be/pricelists/var/online_res_el_nl_14.pdf) | the margins, coefficients and fixed fees, and the excise and certificates they bill | each month |
| [Luminus' certificates table](https://www.luminus.be/nl/prive/hulp-en-contact/hoe-worden-de-tarieven-voor-groene-stroom-en-wkk-bepaald-nl/) | the green power and CHP certificates | each quarter |
| [Customs circular 2026/C/82](https://blog.forumforthefuture.be/nl/article/circulaire-2026c82-betreffende-energieproducten-en-elektriciteit/32514) | the federal excise | with the law, last on 1 August 2026 |
| [SMARD's weeks for series 4996](https://www.smard.de/app/chart_data/4996/DE/index_quarterhour.json) | the market prices, a week a file | daily |
