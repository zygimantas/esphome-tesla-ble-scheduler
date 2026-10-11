# plans/no/

Norway's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a spot contract with the state's support (strømstøtte), the default, the board's charging cost 4,243.43 NOK (378.07 EUR) over the year from 1 October 2025: 14% less than charging at once (4,928.07 NOK, 438.29 EUR), 6% less than the car's own schedule from 22:00 (4,519.68 NOK, 402.33 EUR) and 1% less than its best start, 04:00 (4,289.26 NOK, 382.15 EUR). Confidence in the README is Medium: most homes' power is at Norgespris, a fixed price, where the car's own schedule does as well.

## Decisions

- **Contract:** a spot contract with the state's support (strømstøtte), the default; most of NO1's household power is at Norgespris instead.
- **Area:** NO1, Oslo's.
- **Plan:** `no/elvia-households`, as Elvia is Norway's largest grid company. The car's own schedule starts at 22:00, when its night fee does.
- **Billing:** each hour at the average of its four quarter-hours, as meters register hours.
- **Margin:** 0.05900 NOK per kWh with VAT (0.04720 without), the big suppliers' påslag.
- **The same every hour:** 0.10163 NOK per kWh with VAT: the electricity tax and the Enova fee, at 2026's rates all year.
- **Support:** 90% of each hour's spot price above 0.77000 NOK without VAT, with VAT added back, taken off, at 2026's threshold all year.
- **Left out:** the capacity step (kapasitetsledd), which the board doesn't count yet (#82), and monthly fees, as they don't depend on when the car charges.
- **Market prices:** Energy-Charts' NO1 quarter-hours, in euros, at the ECB's daily rate. Its terms allow only private and internal use, and it answers quick repeats with 429 (Too Many Requests).
- **Arva's plan:** without VAT, as its whole area is exempt.

## Findings

- **Who pays the market price:** spot contracts carried 97.6% of what households bought in the second quarter of 2026, fixed prices 1.0% and variable ones 1.4% ([SSB, table 14491](https://www.ssb.no/statbank/table/14491)). But most of NO1's household electricity is on Norgespris, which takes the market price out: Norgespris homes used just under 58% of households' electricity at the end of June 2026, 73% in NO1, 82% in NO2 and 26% in NO3, from Elhub's figures ([SSB, 17 August 2026](https://www.ssb.no/energi-og-industri/energi/statistikk/elektrisitetspriser/artikler/lavere-strompris-for-husholdninger)). Elhub counted just over 1.1 million households with it in NO1, NO2 and NO5 on 31 December 2025 ([TU, 26 January 2026](https://www.tu.no/artikler/norgespris-vs-stromstotte-1-0-i-forste-omgang/567402)).
- **Meters and billing:** spot contracts bill by the hour, not the quarter-hour: homes' meters register hours, and since the market went to quarter-hours a spot contract on such a meter must bill Nord Pool's published hourly price in kroner ([FOR-1999-03-11-301](https://lovdata.no/dokument/SF/forskrift/1999-03-11-301) §§ 3-2 and 7-6). That price is the average of the hour's four quarter-hours (Nord Pool's hourly NO1 index for 10 October 2026), as RME recommended ([report 2/2025](https://publikasjoner.nve.no/rme_rapport/2025/rme_rapport2025_02.pdf)), and Fjordkraft's and Fortum's pages say "time for time".
- **State support and Norgespris:** each meter has one or the other ([LOV-2025-06-20-44](https://lovdata.no/dokument/NL/lov/2025-06-20-44) § 8).
  - Strømstøtte, by default: for each hour, the grid bill gives back 90% of the hour's spot price above a threshold, and 25% more for VAT. A kWh's support is 1.125 × max(0, P - T) kroner, with P the hour's spot price without VAT (its quarter-hours' average, not each quarter-hour's) and T 0.77000 in 2026 and 0.75000 from 15 September to 31 December 2025, for the first 5,000 kWh a month per meter ([FOR-2025-09-08-1791](https://lovdata.no/dokument/SF/forskrift/2025-09-08-1791) § 5; [RME](https://www.nve.no/reguleringsmyndigheten/kunde/stroem/dette-er-stroemstoetteordningen/) writes it as (spot price - 77) × 0.9 × 1.25 øre). The margin gets none.
  - Worked out from Energy-Charts' prices at the ECB's rate: in NO1, 94% of the hours from 22:00 to 07:00 from January to September 2026 were above 0.77000, and 33% of those from October to December 2025 above 0.75000, so in 2026 a home paid only a tenth of most nights' price swings.
  - Norgespris, on request: 0.40000 without VAT, 0.50000 with it, from 1 October 2025 to 31 December 2026. For each hour the grid company pays the spot price above it, or charges what's below it, with VAT, for the first 5,000 kWh a month ([FOR-2025-09-08-1790](https://lovdata.no/dokument/SF/forskrift/2025-09-08-1790) §§ 10, 11 and 13). The meter is then bound to the end of 2026, after 14 days to cancel, and gets no strømstøtte. The supplier's contract, margin and monthly fee stay ([RME](https://www.nve.no/reguleringsmyndigheten/kunde/stroem/dette-er-norgespris/)). So every hour costs 0.50000 plus the margin, and the board can save only the plan's night discount.
- **VAT:** 25%, on the energy, the margin, the grid fee and the charges below; the support adds it back. None for homes in Nordland, Troms and Finnmark ([merverdiavgiftsloven § 6-6](https://lovdata.no/dokument/NL/lov/2009-06-19-58)), roughly NO4, where the board leaves it out.
- **Per kWh, the same every hour:** two charges, billed with the grid fee, 0.10163 NOK with VAT in 2026 and 0.16913 from October to December 2025:
  - the electricity tax (forbruksavgift), 0.07130 without VAT, 0.08913 with it, all of 2026 ([Skatteetaten](https://www.skatteetaten.no/satser/elektrisk-kraft/)); in 2025, 0.16930 from April to September, then 0.12530 (0.15663 with VAT) from October to December ([Skatteetaten, 2025](https://www.skatteetaten.no/satser/elektrisk-kraft/?year=2025)). Homes in Finnmark and seven Nord-Troms municipalities pay none.
  - the Enova fee (Energifondet), 0.01000 without VAT, 0.01250 with it, for homes, in 2025 and 2026; businesses pay 800 kr a year instead (Elvia's sheets, under The plan; [Midt Nett's 2026 price list](https://midtnett.no/wp-content/uploads/2025/12/Prisliste-fra-01.01.2026.pdf)).
- **Margins:** the påslag of standard spot contracts, per kWh with VAT and electricity certificates, checked on 10 and 11 October 2026: Fjordkraft Strøm til Spotpris 0.05900 and 69 kr a month ([price list](https://www.fjordkraft.no/strom/stromavtale/prisliste/), updated 10 October 2026), Fortum Spotpris 0.05900 and 49 kr ([price list](https://www.fortum.com/no/strom/stromavtale/prisliste)), [Lyse Spotpris](https://www.lyse.no/strom/spotpris) 0.05900 and 55 kr, [Motkraft Spot](https://www.motkraft.no/) 0.06900 and 59 kr and [Tibber](https://tibber.com/no/bytte-stromleverandor) 0.03400 and 49 kr. Forbrukerrådet's price portal had 253 hourly spot contracts in Oslo open to anyone without a membership on 11 October 2026: median 0.05490, monthly fee median 39 kr ([its API](https://strom-api.forbrukerradet.no/products/grid/5)). Hafslund and NorgesEnergi have none there.
- **The plan:** Elvia is Norway's largest grid company, in Oslo, Akershus, Østfold and Innlandet ([Elvia](https://www.elvia.no/hva-er-elvia/)). Its night fee runs 22:00 - 06:00 on workdays and all day at weekends and on public holidays. The plan matches Elvia's [prices from 1 July 2026](https://www.elvia.no/siteassets/dokumenter/priser/2026/1-juli-2026/tariffblad_1_0_standard-tariff_privat_20260701.pdf), day and night 0.46400 and 0.31400 with the tax, the Enova fee and VAT: 0.36238 and 0.21238 without the first two. From 1 October 2025 to 30 June 2026 they were 0.26238 and 0.16238 (sheets of [1 October 2025](https://www.elvia.no/siteassets/dokumenter/priser/2025/1-oktober-2025/tariffblad_1_0_standard_tariff_privat_20251001_elavg_inkl_mva.pdf) and [1 January 2026](https://www.elvia.no/siteassets/dokumenter/priser/2026/tariffblad_1_0_standard-tariff_privat_20260101.pdf)), so for 9 of the 12 months the night discount was 0.10000, not the plan's 0.15000.
- **Market prices:** Energy-Charts has NO1's quarter-hours, which the board plans on, in euros per MWh, a month a request. It had all 35,040 quarter-hours from 1 October 2025 to 30 September 2026, but answers "Too Many Requests" to quick repeats, and its terms allow only private and internal use. hvakosterstrommen.no has the hours that are billed, a day a file (24 hours, 23 or 25 when the clocks change), free for any use, in kroner at Norges Bank's rate (its EXR field). Energy-Charts' hourly averages match hvakosterstrommen.no's to 0.01 EUR/MWh, and hvakosterstrommen.no's kroner were about 0.15% under Nord Pool's on 10 October 2026, from Nord Pool's own exchange rate. Nord Pool's API needs an account beyond its last two months.

## Changes ahead

- **Any time:** the companies change their fees at any time at 14 days' notice, as Glitre did on 1 October 2026; Elvia changed its on 1 July 2026.
- **1 January each year:** the electricity tax and the support's threshold, set for each year; the tax also changed during 2025, on 1 October.
- **31 December 2026:** Norgespris's price, 0.40000 without VAT, and a meter's binding to it run until then.

## Open questions

- **Each hour's price for the support and Norgespris:** RME doesn't say how it gets it from the quarter-hours; the billed average is the likely one.
- **How many homes chose Norgespris:** SSB's shares are of households' electricity, not of homes, and Elhub's count of just over 1.1 million households in NO1, NO2 and NO5 has no total beside it.

## Notes

- **The board doesn't know the support:** its scheduler (total_price() in scheduler/tariff.h) leaves it out, so where prices pass the threshold, the plan's night discount matters more than it thinks.
- **Moving holidays:** the plan has 2027's, so the example's year has them on 2027's dates rather than 2026's: 2, 3 and 6 April, 14 May and 25 May.
- **NVE's open data:** it has Elvia at 0.41400 and 0.28900 from 1 July 2026, halfway between Elvia's sheets of 1 January and 1 July 2026, and no holidays. The plan follows Elvia's own sheet and page.
- **regjeringen.no:** behind a bot check, so it couldn't be read.
- **Not done yet:** Linja and the smaller companies, and the capacity step (#82).

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Elvia's prices from 1 July 2026](https://www.elvia.no/siteassets/dokumenter/priser/2026/1-juli-2026/tariffblad_1_0_standard-tariff_privat_20260701.pdf) | the plan's fees, with the electricity tax, the Enova fee and VAT | any time, at 14 days' notice |
| The other companies' own pages, linked in their plans' first lines | the plans' fees | any time, at 14 days' notice, as Glitre's on 1 October 2026 |
| NVE's open data (nettleietariffer.dataplattform.nve.no) | each company's share of homes and its prices by date, to check against | with the companies' prices |
| [SSB, table 14491](https://www.ssb.no/statbank/table/14491) | households' electricity by contract: spot, fixed and variable | each quarter |
| [SSB's electricity prices](https://www.ssb.no/energi-og-industri/energi/statistikk/elektrisitetspriser/artikler/lavere-strompris-for-husholdninger) | Norgespris' share of households' electricity by area, from Elhub | each quarter |
| [Skatteetaten](https://www.skatteetaten.no/satser/elektrisk-kraft/) | the electricity tax | each 1 January, and in 2025 on 1 October too |
| [FOR-2025-09-08-1791](https://lovdata.no/dokument/SF/forskrift/2025-09-08-1791), [RME on the support](https://www.nve.no/reguleringsmyndigheten/kunde/stroem/dette-er-stroemstoetteordningen/) | the support and its threshold | each year |
| [FOR-2025-09-08-1790](https://lovdata.no/dokument/SF/forskrift/2025-09-08-1790), [RME on Norgespris](https://www.nve.no/reguleringsmyndigheten/kunde/stroem/dette-er-norgespris/) | Norgespris | its price holds until 31 December 2026 |
| [Forbrukerrådet's price portal](https://strom-api.forbrukerradet.no/products/grid/5) | the spot contracts' margins and monthly fees in Oslo | any time |
| [Fjordkraft's](https://www.fjordkraft.no/strom/stromavtale/prisliste/) and [Fortum's](https://www.fortum.com/no/strom/stromavtale/prisliste) price lists | the big suppliers' påslag and monthly fees | any time |
| [Energy-Charts](https://api.energy-charts.info/price?bzn=NO1&start=2025-10-01&end=2025-11-01) | the market prices (NO1) in euros, a month a request | daily |
| [hvakosterstrommen.no](https://www.hvakosterstrommen.no/api/v1/prices/2025/10-01_NO1.json) | the billed hours in kroner, a day a file, to check against | daily |
