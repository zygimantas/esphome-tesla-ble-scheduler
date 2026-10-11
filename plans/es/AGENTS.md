# plans/es/

Spain's research, checked in October 2026, for this folder's plan and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On an indexed offer billing OMIE's hourly price at cost, the board's charging cost 476.36 EUR over the year from 1 October 2025: 36% less than charging at once (744.46 EUR), 23% less than the car's own schedule from 00:00 (615.93 EUR) and 12% less than its best start, 04:00 (539.97 EUR). Confidence in the README is High: over a quarter of homes pay each hour's price, most on the regulated PVPC.

## Decisions

- **Contract:** an indexed offer billing OMIE's hourly price at cost. The regulated PVPC, which over a quarter of homes have, keeps each hour's own market price too.
- **Plan:** `es/2-0td`. The car's own schedule starts at 00:00, when its valley period does.
- **Billing:** each hour at the average of its four quarter-hours, as household meters record hours.
- **Margin:** 0.03267 EUR per kWh with VAT, about 0.027 without: balancing services, the energy savings fund, the operators' fees and imbalances.
- **Factors:** 1.167 on the market price, for grid losses at night and the municipal rate, and 1.0511269632 on all of it, for the electricity tax.
- **Left out:** the power term, meter rental and the bono social financing, as they don't depend on when the car charges; the cuts in VAT and the electricity tax from 22 March to 31 May 2026, as the normal rates are used all year.
- **Market prices:** OMIE's files, Spain's column, as the board downloads them.

## Findings

- **Who pays the market price:** on 31 December 2025 about 8.2 million consumers were on the regulated PVPC, 27.4% of household consumers and about 22% of household electricity, and the rest on the free market ([CNMC, page 15](https://www.cnmc.es/sites/default/files/6742152.pdf)). In 2025, 24% of household electricity was billed at a price that changes every hour, 15% at prices that change by period and 61% at one price ([CNMC, page 43](https://www.cnmc.es/sites/default/files/6742152.pdf)). In December 2025 the free market's household electricity was 75% at one fixed price, about 18% fixed by period, about 3% indexed by period or month and about 4% hourly (the chart on [page 44](https://www.cnmc.es/sites/default/files/6742152.pdf): the 75% is labelled, the rest measured from its bars). So free-market offers indexed to the hourly price are only a few percent of household electricity, and the reports give no count of their customers. In the CNMC's consumer panel, 9.4% of those asked said they had a different price each hour and 21.3% didn't know their tariff (fourth quarter of 2023), and 52.2% couldn't tell the free market from the PVPC (second quarter of 2025).
- **The PVPC:** open only to contracts up to 10 kW, so a home that contracts more for an 11 kW charger can only take a free-market indexed offer, and homes with the bono social must be on it ([RD 446/2023](https://www.boe.es/diario_boe/txt.php?id=BOE-A-2023-14048)'s preamble). Its energy cost in 2026 is 45% day-ahead and intraday and 55% futures ([CNMC, page 28](https://www.cnmc.es/sites/default/files/6742152.pdf)), but the futures enter as an adjustment worked out once a day, 3.55 EUR per MWh on average in the first half of 2026, and each hour keeps its own market price in full (RD 446/2023). Worked out from REE's hourly PVPC from January to September 2026 ([REE](https://apidatos.ree.es/es/datos/mercados/precios-mercados-tiempo-real?start_date=2026-09-01T00:00&end_date=2026-09-30T23:59&time_trunc=hour), a month a request) and OMIE's quarter-hours ([OMIE](https://www.omie.es/es/file-download?parents=marginalpdbc&filename=marginalpdbc_20260930.1)): the PVPC rises 1.17 EUR per MWh, grid losses included, for each EUR per MWh by which an hour's day-ahead price, the average of its quarter-hours, is above its day's average, and only 0.53 for each EUR of the day's average. So the difference between a cheap hour and a dear one reaches a PVPC home in full.
- **Meters and billing:** household meters record hours, not quarter-hours: practically all have hourly measurement ([CNMC, 18 December 2025, page 4](https://www.cnmc.es/sites/default/files/6340245.pdf)), and the system operator asks for quarter-hours only from the larger point types ([BOE](https://www.boe.es/diario_boe/txt.php?id=BOE-A-2025-6693)), although the day-ahead market has priced every quarter-hour since 1 October 2025 ([OMIE](https://www.omie.es/sites/default/files/2025-10/2501001_go_live_mtu15_md_es_vf.pdf)). So the PVPC bills each hour at the average of its four quarter-hour prices, which REE's PVPC series follows rather than the hour's first quarter, and so does Gana Energía's indexed offer ([Gana Energía, 2 October 2025](https://ganaenergia.com/blog/nuevo-mercado-cuarto-horario-de-luz-2025-como-afecta-a-mi-factura/)): the board's choice of quarter-hours within an hour changes nothing on the bill. Lucera bills the month's average per 2.0TD period instead.
- **VAT:** 21%, but 10% for contracts under 10 kW from 22 March to 31 May 2026 (RDL 7/2026, ended early by RDL 18/2026).
- **Electricity tax (IEE):** 5.11269632% of the supply before VAT, with VAT then on top (Ley 38/1992, art. 99): a factor of 1.0511269632 on everything else. From 22 March to 31 May 2026 it was 0.5%, with a minimum of 1 EUR per MWh.
- **Per kWh, besides the market price:** what indexed offers add, without VAT, together about 0.027 EUR plus 16.7% of the market price, 0.040 EUR at the valley period's average price and 0.048 with VAT:
  - grid losses, about 15% of the market price at night: REE sets them by the hour, and Octopus uses 14.4% in the valley period;
  - the municipal rate, 1.5% of the energy part;
  - balancing services, 20.8 EUR per MWh on average from October 2025 to September 2026 ([REE's price components](https://apidatos.ree.es/es/datos/mercados/componentes-precio-energia-cierre-desglose?start_date=2025-10-01T00:00&end_date=2026-09-30T23:59&time_trunc=month)), about 0.024 EUR per kWh at the meter;
  - the energy savings fund (FNEE), 0.0024 EUR from 28 February 2026;
  - the system and market operators' fees, 0.00024 EUR;
  - imbalances, about 0.0005 EUR;
  - capacity payments, 0 in the valley period.
- **Margins:** the indexed offers pass OMIE's price on at cost (Octopus Flexi, niba Flex, Lucera, Gana Energía, Som Energia, Factor Energía), mostly with no margin per kWh but a fee of 3.7 to 5.1 EUR a month. Som Energia adds 0.020 EUR (0.0242 with VAT) and Factor Energía 0.0102 EUR.
- **The plan:** `es/2-0td`, the access toll every household up to 15 kW pays, whose valley period starts at 00:00 on weekdays and lasts all weekend and on the fixed national holidays. Its prices are 2026's tolls ([CNMC](https://www.boe.es/diario_boe/txt.php?id=BOE-A-2025-26348)) and charges ([Orden TED/1524/2025](https://www.boe.es/diario_boe/txt.php?id=BOE-A-2025-26705)) with 21% VAT, unchanged from January to September 2026; 2025's were 0.11197, 0.03412 and 0.00362 EUR with VAT.
- **Other charges:** the power term, meter rental (0.81 EUR a month) and the bono social financing (0.019 to 0.025 EUR a day).
- **Prices by the hour:** worked out from OMIE's files and REE's PVPC, January to September 2026: the day-ahead price averaged about 20 EUR per MWh from 11:00 to 17:00, 98 from 00:00 to 08:00 and 125 to 133 from 20:00 to 22:00, so night charging saves mostly by leaving the evening and through 2.0TD's valley tolls. On weekdays from 00:00 to 08:00 the dearest night hour averaged 40 EUR per MWh above the cheapest in the PVPC, and 28 in the day-ahead price.
- **Market prices:** OMIE's daily files, as the board downloads them, a line per quarter-hour of the CET day, like https://www.omie.es/es/file-download?parents=marginalpdbc&filename=marginalpdbc_20260101.1.

## Changes ahead

- **1 January each year:** the CNMC's tolls and the ministry's charges, which the plan sums.

## Open questions

- **Grid losses:** there's no regulated table for 2.0TD, as REE sets them by the hour.
- **Balancing costs:** how each supplier passes them on.
- **How many homes pay each hour's price on the free market:** the CNMC reports energy, not customers, and its figures don't reconcile: 31% of household electricity hourly in 2024 ([CNMC, July 2025, page 41](https://www.cnmc.es/sites/default/files/6099794.pdf)) and 24% in 2025, with the PVPC at 22% in both. 2025's imply about 2 to 3% of household electricity: 24% at hourly prices less the PVPC's 22%, or 4% of the free market's in its chart.
- **Homes on the PVPC:** the 8.2 million PVPC consumers are supply points, not households (30.7 million supply points in all against 19.7 million households, [INE](https://ine.es/dyngs/Prensa/ECP3T25.pdf)), so no share of households follows.
- **Quarter-hour meters in homes:** no date found.
- **Other indexed offers:** whether those other than Gana Energía's average an hour's quarter-hours.
- **The PVPC's futures adjustment:** whether the CNMC's suggestion in its July 2025 report to drop it ([CNMC, page 56](https://www.cnmc.es/sites/default/files/6099794.pdf)) goes ahead.

## Notes

- **REE's spot price:** REE's apidatos "Precio mercado spot" hourly series has been the hour's first quarter-hour since October 2025, not its average: at 18:00 on 8 October 2026, 5.00 EUR per MWh against an average of 89.56. Use OMIE's files.
- **OMIE's columns:** each line has Portugal's price, then Spain's. They differed in 10.4% of the quarter-hours from October 2025 to September 2026 (plans/pt/AGENTS.md), so figures from the wrong one look right.
- **OMIE's corrections:** OMIE keeps only a day's latest file, numbered .2 or .3 when it corrects one: 30 October 2025's is .3.
- **OMIE's site:** its web pages answer scripts with 403, while its file downloads work.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [The CNMC's 2.0TD tolls for 2026](https://www.boe.es/diario_boe/txt.php?id=BOE-A-2025-26348) | the plan's tolls | each 1 January |
| [Orden TED/1524/2025](https://www.boe.es/diario_boe/txt.php?id=BOE-A-2025-26705) | the plan's charges | each 1 January |
| [The CNMC's report on the retail markets in 2025](https://www.cnmc.es/sites/default/files/6742152.pdf) | consumers on the PVPC, household electricity by kind of price, the PVPC's energy cost | each year; 2024's came in July 2025 |
| [RD 446/2023](https://www.boe.es/diario_boe/txt.php?id=BOE-A-2023-14048) | the PVPC's rules: who may have it and its share of futures | when amended |
| Ley 38/1992, art. 99 | the electricity tax | with a new law, as RDL 7/2026 did |
| [REE's price components](https://apidatos.ree.es/es/datos/mercados/componentes-precio-energia-cierre-desglose?start_date=2025-10-01T00:00&end_date=2026-09-30T23:59&time_trunc=month) | balancing services and the other costs per MWh | each month |
| [REE's PVPC](https://apidatos.ree.es/es/datos/mercados/precios-mercados-tiempo-real?start_date=2026-09-01T00:00&end_date=2026-09-30T23:59&time_trunc=hour) | the PVPC by the hour, a month a request | daily |
| [OMIE's daily files](https://www.omie.es/es/file-download?parents=marginalpdbc&filename=marginalpdbc_20260101.1) | the market prices, a day a file | daily |
