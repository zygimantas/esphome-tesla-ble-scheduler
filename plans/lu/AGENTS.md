# plans/lu/

Luxembourg's research, checked in October 2026, for this folder's plan and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a dynamic contract billing EPEX's DE-LU price by the hour, the board's charging cost 602.32 EUR over the year from 1 October 2025: 20% less than charging at once (756.45 EUR), 8% less than the car's own schedule from midnight (651.67 EUR) and 2% less than its best start, 03:00 (612.97 EUR). Confidence in the README is Low: few homes pay the market price.

## Decisions

- **Contract:** a dynamic contract billing EPEX's DE-LU price, as the Encevo group's and SUDenergie's do.
- **Plan:** `lu/client-standard`, the same fee at every hour, so the car's own schedule starts at midnight, where the suppliers' fixed EV hours start too.
- **Billing:** each hour at the average of its four quarter-hours, as the Encevo group billed until 31 May 2026.
- **Margin:** 0.02160 EUR per kWh with VAT (0.02000 without), the commission of both dynamic offers that fit.
- **The same every hour:** nothing: the electricity tax and the contribution to the compensation mechanism cancel out from January to July 2026, the rate used for the whole year.
- **Left out:** the network's fixed fee, metering and the supplier's monthly fee, as they don't depend on when the car charges, and the supplement above the reference power, which depends on power, not the hour (#82).
- **Market prices:** SMARD's series 4169.

## Findings

- **Who pays the market price:** few homes, though nearly every home can have a dynamic contract. A supplier with more than 15,000 customers must offer one (law of 9 June 2023), and the first offer came in July 2024; no count of dynamic contracts is published. The Encevo group sold 92.4% of the electricity homes bought in 2024. Most homes stay with their incumbent on a price the supplier can change at 30 days' notice: 96.0% of household contracts at the end of 2024, while 4.0% had a price fixed for a term. Only 1.0% of customers switched supplier in 2024. The ILR judged the first dynamic offer's 7.5 ct margin a way to meet the legal duty rather than to win customers ([ILR's report for 2024, November 2025](https://www.ceer.eu/wp-content/uploads/2025/11/Benchmark-Report-2025-base-2024_final.pdf), pages 8 and 53 to 62).
- **Meters and billing:** a dynamic contract needs a smart meter (Smarty). The Encevo group bills EPEX's DE-LU day-ahead price plus a commission, per quarter-hour since 1 June 2026 and per hour before; SUDenergie bills the same price per quarter-hour, and Sudstroum per hour. Negative prices are passed on.
- **The offers:** the Encevo group sells one product under four names: dynamic naturstroum home from Enovos, Leo, Nordenergie and Steinergy. SUDenergie sells Mäi Stroum dynamic, since 1 May 2026, and Sudstroum TERRA DYNAMIC, an offer its own page calls unattractive. The ILR's comparator, [calculix.lu](https://www.calculix.lu), lists just these, and no offer from Electris, now in the Encevo group, or from Eida.
- **VAT:** 8% on the whole bill, market price included, in 2025 and 2026. The customs administration's [2026 excise rates](https://douanes.public.lu/content/dam/douanes/fr/accises/taux-accises-lu-01012026.pdf) give 8% VAT for electricity, and every supplier sheet quotes prices without VAT (8%).
- **Per kWh, the same every hour:** with VAT, 0.00000 EUR from 1 January to 31 July 2026, -0.03899 from 1 August 2026 and -0.03953 from October to December 2025, as Enovos' sheets of [1 January 2026](https://www.enovos.lu/fr/?wpdmdl=897852), [1 August 2026](https://www.enovos.lu/fr/?wpdmdl=901309) and [2025](https://www.enovos.lu/fr/?wpdmdl=896078) show, and SUDenergie's too:
  - the electricity tax (taxe électricité), category A (up to 25,000 kWh a year): 0.00100 EUR without VAT, 0.00108 with it, that is 1.00 EUR/MWh in the [2026 excise rates](https://douanes.public.lu/content/dam/douanes/fr/accises/taux-accises-lu-01012026.pdf), unchanged on [1 July 2026](https://douanes.public.lu/content/dam/douanes/fr/accises/taux-accises-lu-01072026.pdf) and the same in 2025;
  - the contribution to the compensation mechanism, category A: -0.00100 EUR without VAT, -0.00108 with it, from 1 January to 31 July 2026 ([Règlement ILR/E25/40](https://legilux.public.lu/eli/etat/leg/rilr/2025/12/19/a624/jo)). The state now pays for the mechanism. From 1 August 2026 it's -0.03710 without VAT, -0.04007 with it ([Règlement ILR/E26/28](https://legilux.public.lu/eli/etat/leg/rilr/2026/07/16/a353/jo)), carrying the [Resilienzpak 2026 subsidy](https://meco.gouvernement.lu/fr/domaines-activites/energie/marches-de-lenergie/subvention-electricite.html) of 4 ct with VAT, for homes under 25,000 kWh a year, until 31 December 2026. In 2025 it was -0.03760 without VAT, -0.04061 with it ([Règlement ILR/E24/56](https://legilux.public.lu/eli/etat/leg/rilr/2024/12/19/a609/jo)).
- **Altogether:** worked out from these, a kWh on the Encevo group's dynamic contract cost 0.07668 EUR with VAT on top of the market price from January to July 2026 (the plan, the margin and the sum above), 0.03769 from August 2026, and 0.09104 from October to December 2025.
- **Monthly fees and the supplement:** the network's fixed fee by reference power is 7.42 EUR a month without VAT at 3 kW and 19.61 at 12 kW, and metering 5.72 EUR a month up to 50 A; the suppliers' monthly fees are under Margins. The supplement of 0.07650 EUR without VAT, 0.08262 with it, on the kWh above the reference power in each quarter-hour, depends on power, not the hour (#82).
- **Margins:**
  - the Encevo group's dynamic naturstroum home charges 0.02000 EUR per kWh without VAT plus 13.00 EUR a month from 1 January 2026, with 2.50 EUR off for e-mailed bills and direct debit, per quarter-hour from 1 June 2026 ([Enovos' sheet of 1 January 2026](https://www.enovos.lu/fr/?wpdmdl=897428), [of 1 June 2026](https://www.enovos.lu/fr/?wpdmdl=900486)); Nordenergie's and Steinergy's sheets are the same. In 2025 the commission was 0.04500 without VAT, 0.04860 with it ([sheet](https://www.enovos.lu/fr/?wpdmdl=897288)), and from July 2024 0.07500;
  - [SUDenergie's Mäi Stroum dynamic](https://www.sudenergie.lu/fileadmin/documents/Electricite/Consommation/Residentiel/SUDenergie_Conditions_tarifaires_de_fourniture_electricite_residentiel_MaeiStroum_dynamic_20260501.pdf) charges 0.02000 per kWh plus 10.00 EUR a month, with 1.50 EUR off, from 1 May 2026, unchanged on 1 August 2026;
  - [Sudstroum's TERRA DYNAMIC](https://www.sudstroum.lu/fr/produits/nouveau-produit-terra-dynamic/), left out as an outlier, charges 0.10000 per kWh plus 30.00 EUR a month, hourly, unchanged since July 2024: 0.10800 with VAT, if the 0.10000 is without VAT like Sudstroum's other prices.
- **The plan:** `lu/client-standard`, Creos' client standard, is for every home with a smart meter on any of the four networks, as the fee is national: 0.05100 EUR without VAT, 0.05508 with VAT, in every hour of every day, so no cheap fee starts at any hour. It matches the [Creos sheet](https://www.creos-net.lu/fileadmin/dokumente/downloads/fr_tarif_electricite_2026.pdf) of 26 November 2025, in the plan's first line, and the 2026 sheets of Enovos, SUDenergie and Sudstroum. Nothing changed during 2026: Enovos' sheet of 1 August 2026 has the same fee. In 2025 it was 0.07590, 0.08197 with VAT ([Creos' 2025 sheet](https://www.creos-net.lu/fileadmin/dokumente/downloads/fr_tarif_electricite_2025.pdf)). The drop came from 150 million euros the state pays toward 2026's network costs (law of 19 December 2025). Only homes with storage heating and a calendar pay less at night, and only on the supplement: 0.00760 instead of 0.07650, from 22:00 to 06:00.
- **The car's own schedule:** 00:00 is where the suppliers' fixed EV hours start. The Encevo group's naturstroum home drive charges 0.09213 EUR without VAT from 00:00 to 06:00, against 0.13213 otherwise, and SUDenergie's Mäi Stroum smart, from 1 August 2026, has a winter rate of 0.08440 from 00:00 to 06:00. On SMARD's prices for the year, without VAT, an hour of charging from 18:00 averaged 0.13913, from 00:00 0.11002, and from 03:30, the best start, 0.09875.
- **Market prices:** SMARD's chart_data JSON for series 4169, DE-LU's day-ahead price, in EUR/MWh, per quarter-hour from 1 October 2025: a file a week, from Monday 00:00 German time, listed in the [index](https://www.smard.de/app/chart_data/4169/DE/index_quarterhour.json), like https://www.smard.de/app/chart_data/4169/DE/4169_DE_quarterhour_1759096800000.json, the week from 29 September 2025. smard_url(4169, ...) and PriceTable::add_smard() in scheduler/market.h build and read the files. All 35,068 quarter-hours from 1 October 2025 00:00 to 1 October 2026 07:00 are there. 15 September 2026 matched Nord Pool's GER area, the board's source for LU, in all 96 quarter-hours. This is the price the dynamic contracts bill: EPEX's DE-LU day-ahead price is the coupled auction's, the same on every exchange.

## Changes ahead

- **Each year:** the operators publish the next year's prices by 15 October, and the ILR approves them in December, for 1 January. 2027's network fees aren't out yet, due by 15 October 2026.
- **31 December 2026:** the Resilienzpak 2026 subsidy, which the contribution to the compensation mechanism carries from 1 August 2026, ends.
- **2027:** 2026's fees include a state contribution for 2026 only, the 150 million euros of the law of 19 December 2025. In August 2026 the government said it would help with network costs for at least three years, and pay for the compensation mechanism from 2026 to 2028.

## Open questions

- **Hourly billing until 31 May 2026:** which hourly price the Encevo group billed from 1 October 2025, when the auction already settled quarter-hours. Most likely each hour's average, so charging by the quarter-hour then saved a little less.
- **How many homes have a dynamic contract:** no count was found.
- **Sudstroum's VAT:** whether TERRA DYNAMIC's 0.10000 is without VAT, like its other prices.

## Notes

- **Leo:** its own site couldn't be read, as its certificate failed. Its offer comes from calculix.lu and Encevo's shared product.
- **Not dynamic:** Energy Revolt's Bee Flexible has fixed prices for four time bands.
- **Two best starts:** 03:30, under The car's own schedule, is the best start for an hour of the market price alone; the example's, with each hour billed at its average, is 03:00.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Creos' price sheet for 2026](https://www.creos-net.lu/fileadmin/dokumente/downloads/fr_tarif_electricite_2026.pdf) | the plan's fee, the supplement, the fixed fee and metering | each 1 January: published by 15 October, approved by the ILR in December |
| [calculix.lu](https://www.calculix.lu), the ILR's comparator | the dynamic offers | any time |
| [Enovos' sheet for dynamic naturstroum home](https://www.enovos.lu/fr/?wpdmdl=900486), [SUDenergie's for Mäi Stroum dynamic](https://www.sudenergie.lu/fileadmin/documents/Electricite/Consommation/Residentiel/SUDenergie_Conditions_tarifaires_de_fourniture_electricite_residentiel_MaeiStroum_dynamic_20260501.pdf) | the margins and monthly fees | any time |
| [The customs administration's excise rates](https://douanes.public.lu/content/dam/douanes/fr/accises/taux-accises-lu-01072026.pdf) | the electricity tax and VAT | with each new edition, as on 1 January and 1 July 2026 |
| [Règlement ILR/E26/28](https://legilux.public.lu/eli/etat/leg/rilr/2026/07/16/a353/jo) | the contribution to the compensation mechanism, with the state's subsidy | each 1 January, set in December, and in 2026 again on 1 August |
| [ILR's report for 2024](https://www.ceer.eu/wp-content/uploads/2025/11/Benchmark-Report-2025-base-2024_final.pdf) | market shares, the kinds of contracts, switching | each year, around November |
| [SMARD's index for series 4169](https://www.smard.de/app/chart_data/4169/DE/index_quarterhour.json) | the market prices, a week a file | daily |
