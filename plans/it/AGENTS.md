# plans/it/

Italy's research, checked in October 2026, for this folder's plan and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On an hourly PUN offer, the board's charging cost 926.22 EUR over the year from 1 October 2025: 13% less than charging at once (1,060.45 EUR), 5% less than the car's own schedule from midnight (971.35 EUR) and 0% less than its best start, 04:00 (926.81 EUR). Confidence in the README is Medium: about 3% of homes pay the hourly price.

## Decisions

- **Contract:** an hourly PUN offer, like Duferco's: the EU's dynamic contract.
- **Area:** the board offers only Italy's north (IT-North), whose price stands in for the national PUN, which only GME serves, behind a session token.
- **Plan:** `it/td`, one fee at every hour, so the car's own schedule starts at midnight.
- **Billing:** each hour at the average of its four quarter-hours, as the hourly offers bill.
- **Margin:** 0.02420 EUR per kWh with VAT, the middle of the big suppliers' PUN offers.
- **The same every hour:** 0.08286 EUR per kWh with VAT: the excise, the system charges and dispatching.
- **Grid losses:** 1.10 on the market price, as the PUN is billed on 10% grid losses too.
- **Left out:** the fixed charges per year and per kW, and the suppliers' fees, as they don't depend on when the car charges.
- **Market prices:** SMARD's series 255, IT-North, as the board downloads it.

## Findings

- **Who pays the market price:** few homes, and they pay each hour's PUN Index GME, Italy's national price, not IT-North's quarter-hours. In ARERA's survey of 2025 ([Relazione annuale 2026](https://www.arera.it/fileadmin/allegati/relaz_ann/26/RA26_Volume_1.pdf), Tav. 2.61, 2.64 and 2.65), 49% of the 24.3 million homes in the free market have a variable price: 89.1% of them follow the month's average PUN and 7.1% the hourly price, the EU's dynamic contract. 69.7% of free-market homes pay one price in every hour. Worked out from these, about 3.5% of free-market homes pay the hourly price. The hourly offers come from Duferco Energia (Evergreen Flex, Pianeta Blu Flex, Domus Flex) and about a dozen small suppliers, among the Portale Offerte's [non-comparable offers](https://www.ilportaleofferte.it/portaleOfferte/it/offerte-non-confrontabili.page?commodityType=ELETTRICO).
- **Outside the free market:** the 3.0 million homes in the Servizio a tutele graduali, until 31 March 2027, pay the month's PUN after the fact, by band for two thirds of them, and the 3.2 million vulnerable ones ARERA's quarterly prices for F1 and F23.
- **Meters and billing:** the hourly offers bill the hour's PUN times 1.10 for grid losses, on the 2G meter's hourly readings. None of them bills quarter-hours. GME has set the PUN Index GME for each quarter-hour since 1 October 2025, as the zones' prices weighted by what each zone buys ([DTF n. 25 MPE](https://www.mercatoelettrico.org/Portals/0/Documents/it-IT/20250101DTF25MPE.pdf)), and its hourly value is the mean of the four (checked for March 2026). Homes pay it all over Italy, whatever their zone.
- **Bands:** monthly offers bill the month's average PUN in each band the meter reads: F1 08:00 - 19:00 Monday to Friday; F2 07:00 - 08:00 and 19:00 - 23:00 Monday to Friday and 07:00 - 23:00 on Saturday; F3 the rest, 23:00 - 07:00 and all of Sundays and national holidays (1 and 6 January, Easter Monday, 25 April, 1 May, 2 June, 15 August, 1 November, 8, 25 and 26 December); F23 is F2 and F3. On those the board saves only what charging in F3 saves, as a car's own schedule from 23:00, where F3 starts, does.
- **VAT:** 10% for homes, on everything, the excise too (DPR 633/1972, Tabella A, parte III; `iva_c` in the Portale Offerte's [parameters](https://www.ilportaleofferte.it/portaleOfferte/resources/opendata/csv/parametriML/2026_10/PO_Parametri_Mercato_Libero_E_20261011.csv)).
- **Grid losses:** every PUN offer, hourly or monthly, bills the PUN on the metered kWh plus 10% grid losses (`lambda` in the same parameters; Enel's, Plenitude's and Duferco's terms say so), so with VAT the market price counts 1.21 times.
- **Per kWh, the same every hour:** in 2026, 0.07533 EUR without VAT and 0.08286 with it, worked out from these three:
  - the excise (accisa), 0.02270 EUR without VAT, 0.02497 with it, for any use in homes ([ADM's rates of 6 October 2026](https://www.adm.gov.it/portale/aliquote-accisa-nazionali)): residents with up to 3 kW are exempt on their first 150 kWh a month, which a home that charges a car most nights doesn't keep;
  - the system charges: ASOS 0.02866 EUR without VAT from January to June and 0.03152 from July to December, ARIM 0.00164 all year ([ARERA](https://www.arera.it/area-operatori/prezzi-e-tariffe/oneri-generali-di-sistema-e-ulteriori-componenti), deliberations 588/2025, 98/2026, 227/2026 and 343/2026): 0.03172 EUR on average, 0.03490 with VAT;
  - dispatching (CDISPD), Terna's dispatching and capacity costs, which free-market contracts pass on at ARERA's monthly value ([ARERA's table](https://www.arera.it/fileadmin/area_operatori/prezzi_e_tariffe/Corrispettivi_libero_elettrico_domestico_2026.xlsx)): 0.01507 to 0.03846 EUR without VAT from January to September 2026, the highest in July, 0.02090 on average, 0.02300 with VAT.
- **Per kWh, October to December 2025:** the same excise, ASOS 0.02968, ARIM 0.00165 and dispatching 0.01692 (the Portale Offerte's figure for that quarter), so 0.07094 EUR, 0.07803 with VAT.
- **Fixed charges, left out:** σ1 23.04 EUR a year, σ2 23.52 EUR and UC6 0.19880 EUR per kW a year, DISPbt 1.107 EUR a year, the supplier's fee, and ASOS's 88.75 to 95.09 EUR a year for second homes. The bonus sociale is a fixed daily discount for low incomes, with 115 EUR more in 2026 (decree-law 21/2026, which also cut ASOS for businesses only). No price cap applies in the free market.
- **Margins:** the big suppliers' offers for homes in the Portale Offerte's [open data](https://www.ilportaleofferte.it/portaleOfferte/it/open-data.page) of 11 October 2026, all on the month's PUN, per kWh without VAT, with their fee a year: [Enel Flex Luce](https://www.enel.it/it-it/offerte-luce/enel-flex-luce) 0.02000 EUR, 144 EUR a year and 2% of the PUN for imbalances, until 22 October 2026; [Plenitude Trend Casa Luce](https://eniplenitude.com/offerta/casa/gas-e-luce/trend) 0.02200 and 144 EUR, 16 September to 14 October 2026; [Edison Dynamic Luce](https://www.edisonenergia.it/edison/casa/luce/dynamic-luce) 0.02300 above 2,200 kWh a year (0.01800 below) and 99 EUR, 24 September to 14 October 2026; [A2A Smart Casa Luce](https://casa.a2aenergia.eu/smart-casa) 0.02500 and 114 EUR, 1 to 28 October 2026; Octopus Flex 0.00880 and 72 EUR. Their middle is 0.02200, 0.02420 with VAT. The median of all 1,413 such offers is 0.02365 and 139 EUR a year. The hourly ones, in October 2026: Duferco's [Evergreen Flex](https://dufercoenergia.com/wp-content/uploads/CTE/EE_EVERGREEN_FLEX.pdf) 0.00770 and 84 EUR; its [Domus Flex](https://dufercoenergia.com/wp-content/uploads/CTE/EE_DOMUS_FLEX.pdf) none and 84 EUR plus an 84 EUR service, for homes using up to 5,000 kWh a year; the small suppliers' 0.00000 to 0.08790 and 143 to 275 EUR. On 1 October 2025 the big offers above were 0.00880 to 0.02600, and the median of all 1,025 such offers 0.02640.
- **The plan:** `it/td`, ARERA's TD tariff, which every home pays for the grid: the grid's own charges per kWh, one fee in every hour, so no cheap fee starts. F1, F2 and F3 are the supplier's. Its 0.01620 EUR with VAT is 2026's σ3, 0.01190 (deliberation 575/2025, on [ARERA's page](https://www.arera.it/area-operatori/prezzi-e-tariffe/tariffe-trasmissione-distribuzione-e-misura-clienti-domestici), which the plan's first line links), with UC3 0.00276 and UC6 0.00007 (588/2025, kept for every quarter of 2026) and 10% VAT, as ARERA's own [table of 2026](https://www.arera.it/fileadmin/area_operatori/prezzi_e_tariffe/E2026-smt.xlsx) adds them: it matches. From October to December 2025 it was 0.01487 with VAT (σ3 0.01189, UC3 0.00156).
- **ARERA's trial for cars:** deliberation 541/2020, extended by 22/2025 to 30 June 2027, with joining until 30 June 2026, lets a home with 2 to 4.5 kW draw up to 6 kW in F3 at no extra cost.
- **Market prices:** SMARD's IT-North, as the board downloads it, a week a file from Monday 00:00 German time: https://www.smard.de/app/chart_data/255/DE/255_DE_quarterhour_1759096800000.json is 29 September to 5 October 2025, and https://www.smard.de/app/chart_data/255/DE/index_quarterhour.json lists the weeks. It equals GME's Nord zone to the cent (checked for a week of October 2025 and one of July 2026).
- **The PUN against IT-North:** worked out from GME's and SMARD's prices from October 2025 to September 2026, without VAT: the PUN averaged 0.13761 EUR per kWh and IT-North 0.13811, and 76% of quarter-hours were within 0.00100 of each other. Each night's four cheapest quarter-hours by IT-North (9.1 kWh at 11 kW, 18:00 - 07:00) cost only 0.00030 more at the PUN than the four cheapest by the PUN. Billing by the hour matters more: their market price averaged 0.12285 EUR at each quarter-hour's PUN but 0.12854 at each hour's, against 0.16138 for charging at once and 0.14050 from 23:00 at the hourly PUN (0.15879 and 0.14243 by the quarter-hour). IT-North's hourly means come within 0.00090 of the hourly PUN's figures.

## Changes ahead

- **Each month:** ARERA's dispatching charge (CDISPD).
- **Each quarter:** UC3 and UC6, in the plan, can change, and ASOS, a system charge, did on 1 July 2026.
- **1 January each year:** ARERA's new TD tariff, for the plan.
- **31 March 2027:** the Servizio a tutele graduali ends.
- **30 June 2027:** ARERA's trial for cars ends.

## Open questions

- **ARERA's 7.1%:** whether it counts all homes on a variable price or only 2025's new contracts.
- **Dispatching and the losses:** whether CDISPD is charged on the metered kWh, as the example assumes, or with the 10% losses too.
- **Dispatching from October to December:** ARERA's monthly CDISPD for October to December of 2025 and 2026 isn't known: the Portale Offerte's quarterly figures, 0.01692 and 0.01678, stand in.
- **The excise exemption:** its exact taper.
- **Charging power:** how many homes can charge at 11 kW, as most have 3 kW and some 4.5 or 6.

## Notes

- **GME's prices:** no plain link. Its site's API answers with the cookie and the `__RequestVerificationToken` of https://www.mercatoelettrico.org/it-it/Home/Esiti/Elettricita/MGP/Esiti/PUN, sent as the headers RequestVerificationToken, ModuleId 530 and TabId 48, as in https://www.mercatoelettrico.org/DesktopModules/GmeEsitiPrezziME/API/item/GetMEPrezzi?DataInizio=20251001&DataFine=20251031&Granularita=h&Mercato=MGP&Zona=PUN&Tipologia=PUN (`Granularita=qh` for quarter-hours, `Zona=NORD&Tipologia=PrezziZonali` for IT-North).

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [ARERA's TD tariff for homes](https://www.arera.it/area-operatori/prezzi-e-tariffe/tariffe-trasmissione-distribuzione-e-misura-clienti-domestici) | the TD tariff's σ3, for the plan | each 1 January |
| [ARERA's system charges and other components](https://www.arera.it/area-operatori/prezzi-e-tariffe/oneri-generali-di-sistema-e-ulteriori-componenti) | UC3 and UC6, for the plan, and ASOS and ARIM | any quarter |
| [ARERA's table of 2026 for the free market](https://www.arera.it/fileadmin/area_operatori/prezzi_e_tariffe/Corrispettivi_libero_elettrico_domestico_2026.xlsx) | the dispatching charge (CDISPD) | each month |
| [ADM's excise rates](https://www.adm.gov.it/portale/aliquote-accisa-nazionali) | the excise | by law |
| [The Portale Offerte's open data](https://www.ilportaleofferte.it/portaleOfferte/it/open-data.page) | the offers' margins and fees, and the parameters (VAT, grid losses) | any time |
| [The Portale Offerte's non-comparable offers](https://www.ilportaleofferte.it/portaleOfferte/it/offerte-non-confrontabili.page?commodityType=ELETTRICO) | the hourly PUN offers | any time |
| [ARERA's Relazione annuale 2026](https://www.arera.it/fileadmin/allegati/relaz_ann/26/RA26_Volume_1.pdf) | homes by kind of price (Tav. 2.61, 2.64 and 2.65) | each year |
| [SMARD's index for IT-North](https://www.smard.de/app/chart_data/255/DE/index_quarterhour.json) | the market prices, a week a file | daily |
