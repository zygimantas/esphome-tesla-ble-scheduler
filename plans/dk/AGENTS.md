# plans/dk/

Denmark's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a variable price that follows the day-ahead price, the board's charging cost 4,145.62 DKK (554.79 EUR) over the year from 1 October 2025: 48% less than charging at once (7,992.38 DKK, 1,069.69 EUR), 9% less than the car's own schedule from 00:00 (4,536.60 DKK, 607.11 EUR) and 1% less than its best start, 04:00 (4,202.56 DKK, 562.41 EUR). Confidence in the README is High: most homes pay the market price.

## Decisions

- **Contract:** a variable price that follows the day-ahead price of the home's area, plus the supplier's margin.
- **Area:** DK2, east of the Great Belt, Radius's.
- **Plan:** `dk/radius-nettarif-c`, as Radius Elnet is the biggest grid company. The car's own schedule starts at 00:00, when its low fee does.
- **Billing:** each hour at the average of its four quarter-hours, as Radius reads meters by the hour unless asked.
- **Margin:** 0.10000 DKK per kWh with VAT (0.08000 without), about the middle of the big suppliers'.
- **The same every hour:** 0.15375 DKK per kWh with VAT: the electricity tax at its 2026 rate and Energinet's net and system tariffs.
- **Left out:** the subscriptions and the supplier's monthly fee, as they don't depend on when the car charges; the higher tax and tariffs of October to December 2025, as 2026's rates are used all year; the tax refund for chargers run by a charging operator, 0.4 øre in 2026.
- **Market prices:** SMARD's series 253 (DK2), in euros, at the ECB's daily rate.

## Findings

- **Who pays the market price:** most homes pay a variable price: the day-ahead price of their area, DK1 west of the Great Belt or DK2 east of it, plus the supplier's margin. Suppliers with over 200,000 customers must offer one, and it must follow the spot market in steps no longer than the market's ([BEK nr. 1810 af 29. december 2025](https://www.retsinformation.dk/eli/lta/2025/1810), § 2 and § 26). There's no official share for homes: ACER's country sheet says Denmark doesn't monitor it. Among small business customers, under 20 MWh a year, 83% had a variable price in the second half of 2025 ([Energistyrelsen](https://ens.dk/media/8491/download)).
- **Meters and billing:** the market went to quarter-hours on 1 October 2025, but the bill follows the readings the grid company sends: hourly readings pay the average of the hour's four quarter-hour prices, quarter-hour readings each quarter-hour's own ([Andel Energi](https://andelenergi.dk/el/flexenergi/), [Forbrugerrådet Tænk, 29 September 2025](https://taenk.dk/forbrugerliv/bolig/flere-elpriser-i-doegnet-giver-minimale-besparelser)). Radius's meters read by the hour unless the customer asks the supplier for quarter-hours, which can't be undone ([Radius](https://radiuselnet.dk/din-tilslutning/elmaaler/)).
- **VAT:** 25% on the whole bill: the market price, the margin, the grid fees and the tax. There's no lower rate for electricity. Skatteministeriet counts its tax cut of 71.9 øre on 4,000 kWh as about 3,600 kr with VAT, EWII gives the tax as 90 øre with VAT in 2025 and 1 øre in 2026 (72 and 0.8 øre without), and Radius's price page multiplies DataHub's prices by 1.25.
- **Per kWh, the same every hour:** 0.12300 DKK without VAT, 0.15375 with it, in 2026. The plans have none of these, only the grid company's own fee:
  - the electricity tax (elafgift), 0.00800 without VAT, 0.01000 with it, the EU minimum, from 1 January 2026 to 31 December 2027 ([Skattestyrelsen](https://skat.dk/erhverv/afgifter-paa-varer-og-ydelser-punktafgifter/nyhedsbrev-afgifter/midlertidig-nedsaettelse-af-elafgiften-i-2026-og-2027), [Skatteministeriet's fact sheet, August 2025](https://svmn.dk/media/fg2pvujk/faktaark-om-lempelse-af-elafgift.pdf), DataHub's EA-001);
  - Energinet's net tariff (nettarif, for the distribution grid), 0.04300 without VAT, 0.05375 with it, and its system tariff (systemtarif), 0.07200 without VAT, 0.09000 with it, both for all of 2026 ([Energinet's tariff catalogue for 2026](https://www.energinet.dk/media/ikylsqle/energinets-tarifkatalog-2026.pdf), page 7, and DataHub's 40000 and 41000);
  - from October to December 2025, the tax was 0.72000, the net tariff 0.06100 and the system tariff 0.07400 without VAT: 1.06875 with VAT, 0.91500 more per kWh than in 2026. As they're the same every hour, they change the costs and the percentages, not the saving in kroner.
- **Fixed fees:** Energinet's system subscription, 187 kr a year per meter, Radius's subscription, 441 kr a year without VAT and 490 kr from April 2026, and the supplier's monthly fee.
- **State support:** no price cap, state support or grid tariff for cars was found for 2025 or 2026. Until 2025, a home charger run by a registered charging operator, like Clever's, got most of the tax back; in 2026 that refund is 0.4 øre ([Clever's 0.004 kr for September 2026](https://clever.dk/satser-tillaeg-og-priser/tilbagebetaling/)).
- **Margins:** the big suppliers' spot offers, per kWh with VAT, read on 11 October 2026: [Andel Energi FlexEnergi](https://andelenergi.dk/el/flexenergi/) 0.14630 for new customers and 20 kr a month, reset on 1 January and 1 July; [Norlys FlexEl](https://norlys.dk/el/flexel/) 0.09700 (0.00700 trading costs and 0.09000 administration) from 1 July to 31 December 2026 and 29 kr a month, by the hour; [Modstrøm EnkEL](https://modstroem.dk/) 0.10000 and 10 kr a month; [EWII Plus El](https://www.ewii.dk/privat/el/plus-el/) 0.06500 and 29 kr a month, by quarter-hour where the meter allows; [OK El](https://www.ok.dk/privat/produkter/el/bestil-ok-el/ok-el-hoejt-forbrug) 0.00000 and 49 kr a month above 3,100 kWh a year, or 0.19000 and no monthly fee below. Small suppliers go lower, like [Vindstød's Go'Vind](https://vindstoed.dk/elaftaler) at 0.01000. Prices for homes are given with VAT in Denmark, though only Andel's and OK's pages say so.
- **The plan:** Radius Elnet is the biggest grid company, with about 1,100,000 homes and businesses around Copenhagen and in North and Central Zealand ([Radius](https://radiuselnet.dk/priser/)); N1 in Jutland, DK1 (`dk/n1-nettarif-c`), comes next with over 800,000 ([N1](https://n1.dk/om-n1)). Nettarif C's low fee runs 00:00 - 06:00 every day; summer is April to September, winter October to March. There are no weekend or holiday rules: DataHub holds the C tariff as one day's profile per season, while tariffs that change at weekends, like Radius's A høj, get separate weekday and weekend records. Its C tariff is DataHub's DT_C_01, which Radius's price page shows ([its data](https://cerius-radius-web-server-prod.azurewebsites.net/api/datahubprice?grid=radius&validYear=2026)); low, high and peak without VAT, then with it:
  - 1 October 2025 to 31 March 2026: 0.09760, 0.29290 and 0.87880, then 0.12200, 0.36613 and 1.09850, unchanged since December 2024;
  - 1 April to 30 September 2026: 0.10618, 0.15926 and 0.41408, then 0.13272, 0.19908 and 0.51760: the plan's summer;
  - from 1 October 2026: 0.10618, 0.31852 and 0.95557, then 0.13272, 0.39816 and 1.19447: the plan's winter;
  - worked out from these: the plan's summer is 2026's, but its winter is the one from 1 October 2026, 8.8% dearer than the winter of October 2025 to March 2026 in the example's year.
- **Market prices:** SMARD has the day-ahead prices in euros, series 252 for DK1 and 253 for DK2, a week a file. Energi Data Service's DayAheadPrices has the same, without an account or key: quarter-hours from 1 October 2025 (the hours before are in Elspotprices), in DKK and EUR per MWh, with start and end in Danish time, all 35,040 quarter-hours of the year for each area. Its DKK is the EUR price at Danmarks Nationalbank's rate, and it answers 429 to quick bursts. Both give 102.44, 92.10, 85.90 and 85.27 EUR/MWh for DK2's first quarter-hours of 1 October 2025. These are the prices variable contracts bill, as the hour's average where the meter reads by the hour.

## Changes ahead

- **Each month:** Konstant's discount (rabat) on Nettarif C can change from one month to the next.
- **1 January and 1 July each year:** Andel's and Norlys's margins.
- **1 January each year:** Energinet's net and system tariffs.
- **Summer prices:** next year's aren't out in the autumn, so the plans keep the last summer's until the grid companies publish them. Radius changed its fees on 1 April 2026, not on 1 January.
- **1 January 2028:** the electricity tax's temporary cut to the EU minimum, 0.00800 without VAT, ends after 31 December 2027.

## Open questions

- **How many homes pay the market price:** there's no official share, as ACER's country sheet says Denmark doesn't monitor it; Energistyrelsen's 83% counts only small business customers.
- **VAT in the margins:** whether Norlys's, Modstrøm's and EWII's include VAT, as their pages don't say.
- **Radius's overview:** why its prices for January 2026 differ from DataHub's.
- **Quarter-hour readings:** how many Radius homes have them.
- **Margins earlier in the year:** the ones in October to December 2025, as Andel and Norlys change theirs on 1 January and 1 July.

## Notes

- **Radius's overview:** its prices "pr. 1. januar 2026" ([historical prices](https://radiuselnet.dk/historiske-priser/)) give C 9.71, 29.12 and 87.36 øre in winter and 9.71, 14.56 and 37.86 øre in summer, which DataHub never had. The plan follows DataHub's DT_C_01, which Radius's price page shows.
- **Konstant's discount:** its discount (rabat) on Nettarif C is a record of its own in DataHub (C_FBTNTR_R), and the plan's prices are after it.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Radius's prices](https://radiuselnet.dk/priser/), from [its data](https://cerius-radius-web-server-prod.azurewebsites.net/api/datahubprice?grid=radius&validYear=2026) | the plan's fees, DataHub's DT_C_01 | any time: Radius changed its on 1 April 2026 |
| Energi Data Service's DatahubPricelist, like `https://api.energidataservice.dk/dataset/DatahubPricelist?filter={"ChargeTypeCode":["40000","41000","EA-001"]}&start=2025-01-01` | every grid company's fees, by ChargeOwner and the C tariff's code, like DT_C_01, Energinet's tariffs (40000 and 41000) and the electricity tax (EA-001), in kroner without VAT | as the companies change them; Konstant's discount each month |
| [Energinet's tariff catalogue for 2026](https://www.energinet.dk/media/ikylsqle/energinets-tarifkatalog-2026.pdf) | the net and system tariffs, page 7 | each 1 January |
| [Skattestyrelsen](https://skat.dk/erhverv/afgifter-paa-varer-og-ydelser-punktafgifter/nyhedsbrev-afgifter/midlertidig-nedsaettelse-af-elafgiften-i-2026-og-2027) | the electricity tax | its 2026 rate holds until 31 December 2027 |
| The big suppliers' offers: [Andel Energi](https://andelenergi.dk/el/flexenergi/), [Norlys](https://norlys.dk/el/flexel/), [Modstrøm](https://modstroem.dk/), [EWII](https://www.ewii.dk/privat/el/plus-el/), [OK](https://www.ok.dk/privat/produkter/el/bestil-ok-el/ok-el-hoejt-forbrug) | the margins and monthly fees | Andel's and Norlys's each 1 January and 1 July, the others any time |
| [Energistyrelsen](https://ens.dk/media/8491/download) | the share of small business customers with a variable price | each half year |
| SMARD's series 253, `https://www.smard.de/app/chart_data/253/DE/253_DE_quarterhour_<ms>.json`, the weeks listed in `index_quarterhour.json` | the market prices (DK2) in euros, a week a file | daily |
| Energi Data Service's DayAheadPrices, `https://api.energidataservice.dk/dataset/DayAheadPrices?start=2025-10-01T00:00&end=2025-10-02T00:00&filter={"PriceArea":["DK2"]}&sort=TimeUTC%20asc` | the market prices in DKK and EUR, to check against | daily |
