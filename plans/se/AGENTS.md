# plans/se/

Sweden's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a quarter-hour price contract (kvartspris) in SE3, the board's charging cost 4,974.15 SEK over the year from 1 October 2025: 34% less than charging at once (7,542.59 SEK), 20% less than the car's own schedule from 22:00 (6,197.20 SEK) and 5% less than its best start, 03:30 (5,213.33 SEK). At the ECB's daily rate, those are 456.55, 692.09, 568.27 and 478.40 EUR. Confidence in the README is High: one contract in seven pays each quarter-hour's or hour's price, most the month's average.

## Decisions

- **Contract:** a quarter-hour price contract (kvartspris) billing Nord Pool's SE3 price.
- **Area:** SE3, Stockholm's.
- **Plan:** `se/vattenfall-tidstariff`. The car's own schedule starts at 22:00, when its low fee does.
- **Billing:** each quarter-hour at its own price.
- **Margin:** 0.12 SEK per kWh with VAT (0.096 without), about the middle of the five largest suppliers' variable costs.
- **The same every hour:** the energy tax, 0.450 SEK per kWh with VAT, at its 2026 rate.
- **Left out:** the suppliers' monthly fees, as they don't depend on when the car charges, and SE3's state support of January and February 2026, paid apart from the bill.
- **Market prices:** elprisetjustnu.se's `EUR_per_kWh`, at the ECB's daily rate.

## Findings

- **Who pays the market price:** a minority, about one in seven contracts. SCB counted 14.1% of retail contracts as quarter-hour or hourly priced in August 2026 (14.4% in SE3), against 61.0% on a monthly price and 7.5% on a fixed price of one to three years ([SCB](https://www.statistikdatabasen.scb.se/pxweb/sv/ssd/START__EN__EN0301__EN0301A/SSDManadElAvtalstyp/)); the share stayed between 12.5% and 15.1% in the twelve months to August 2026. SCB first reported quarter-hour and hourly contracts as a category in February 2023, at 10.4%, and 13.4% in October 2024; since 24 February 2025 it reports them as one group. Ei's report R2025:12 (31 October 2025) gave 13.6% for December 2024, and counts about 5.6 million electricity customers, 4.7 million of them households ([Ei](https://ei.se/download/18.3f4d315019a0f5489b07296/1761916461843/Sveriges-el-och-naturgasmarknad-2024-Ei-R2025-12.pdf)). Energiföretagen's Energiåret 2025 (11 May 2026) gives 70% of contracts as variable in January 2026, rounded, from SCB, without an hourly split.
- **Meters and billing:** nearly all of Sweden's 5.7 million metering points were equipped for quarter-hours in May 2025 ([Ei](https://ei.se/om-oss/nyheter/2025/2025-05-21-ei-har-granskat-elnatsforetagens-formaga-att-mata-och-rapportera-kvartsmatvarden)), and hourly metering ended on 1 January 2025 (Ei's questions and answers). But by default a home with a main fuse up to 63 A is metered and reported once a month, unless its contract needs quarter-hours or it asks ([Förordning 1999:716](https://www.riksdagen.se/sv/dokument-och-lagar/dokument/svensk-forfattningssamling/forordning-1999716-om-matning-berakning-och_sfs-1999-716/), 6 b and 6 c §§). Since Nord Pool went to quarter-hours on 1 October 2025, every supplier with over 200,000 customers must offer a quarter-hour contract ([Ei](https://ei.se/konsument/el/elmatare-och-matning-av-din-elanvandning/fragor-och-svar-om-kvartsmatning)). Vattenfall asks the grid company to change the meter at no cost and bills the monthly price until it has ([Vattenfall](https://www.vattenfall.se/kundservice/fragor-och-svar/elavtal-och-elpriser/)); it says the change takes a couple of weeks, at most forty days. The grid company must report the change to the supplier within 40 days of the supplier's request ([Ei](https://ei.se/bransch/matning-av-el/fragor-och-svar-om-elmatning)).
- **The monthly price:** set after the month. At Vattenfall it comes from the month's spot prices and the use of all its monthly-metered customers in the area, plus a profile cost they share, so a home pays for the group's pattern, not its own ([Vattenfall](https://www.vattenfall.se/fokus/tips-rad/skillnad-kvartspris-rorligt-elpris/)).
- **VAT:** 25%.
- **Per kWh, the same every hour:** the energy tax, billed by the grid company: 0.360 SEK without VAT and 0.450 with it in 2026 ([Skatteverket](https://www.skatteverket.se/foretag/skatterochavdrag/punktskatter/energiskatter/skattpael), SFS 2025:1357), 0.439 and 0.54875 in 2025, and 0.096 less without VAT in some northern municipalities. Electricity certificates cost about 0.0002 SEK per kWh, inside suppliers' prices; the authority fees are yearly.
- **State support:** households in SE3 got back 0.26 SEK per kWh used in January and February 2026 (SFS 2026:502), paid apart from the bill, so it doesn't change which hours are cheapest.
- **Margins:** with the variable costs suppliers add, the five largest asked Tibber 0.119, Vattenfall 0.134, E.ON 0.099, Fortum 0.112 and Göteborg Energi 0.135 SEK per kWh with VAT for September 2026, with monthly fees of 45 to 69 SEK ([Ei's Elpriskollen](https://elpriskollen.se/)). Elpriskollen listed 132 quarter-hour and 5 hourly contracts for an SE3 postcode on 11 October 2026.
- **The plan:** Vattenfall Eldistribution is the largest grid company. Its Tidstariff costs more on weekdays from 06:00 to 22:00, November to March, so its low fee starts at 22:00. Its prices match the [2026 price list](https://www.vattenfalleldistribution.se/globalassets/1.-privat/abonnemang-och-avgifter/om-avtal-och-avgifter/elnatsavgifter/sakringsabonnemang/prislista-sakringsabonnemang-privat-2026-01-01.pdf): 0.305 and 0.765 SEK per kWh with VAT. In 2025 they were 0.268 and 0.670, and weekday holidays counted as high load.
- **Power fees:** the government stopped the requirement for them on 13 March 2026, and Ellevio and Mälarenergi went back to prices by fuse size.
- **Market prices:** elprisetjustnu.se serves them a day a file, like [1 January 2026's](https://www.elprisetjustnu.se/api/v1/prices/2026/01-01_SE3.json). Its `EUR_per_kWh` matched Nord Pool's SE3 prices in euros on all 4,992 quarter-hours from 10 August to 30 September 2026. Its own kronor come at another rate, so the example converts the euros at the ECB's daily rate, within half a percent of Nord Pool's kronor.

## Changes ahead

- **Each month:** the suppliers' variable costs.
- **Each 1 January:** most grid companies' new prices, some also during the year, and the energy tax's rate for the year.
- **1 January 2027:** the Elsystemförordning (SFS 2026:1522) comes into force, repeating the metering rule in 10 kap. 7 to 9 §§.
- **By 12 April 2027:** Ei's proposal for a new model, after the requirement for power fees went.
- **By 1 January 2028:** E.ON's three areas move to one price list, as this folder's README.md says, with no source recorded.

## Open questions

- **Households or all customers:** SCB does not say whether its contract counts cover homes only. Its monthly survey asks suppliers for contracts by type and area, with no split by customer, and it warns that a wrong contract type in a supplier's report is hard to spot ([SCB's quality declaration](https://www.scb.se/contentassets/1e73cca715aa4bfba4f951d86537bf59/en0301/en0301_kd_2026_v1_20260522.pdf)), while Ei's report R2025:12 calls the same figures household customers'.
- **Electric car owners:** no figure was found for their share on quarter-hour or hourly contracts.
- **Homes waiting for a meter change:** no count after May 2025, when Ei counted 15,849 low-voltage metering points without a smart meter and 13 grid companies not yet reporting quarter-hour values for points settled on those values, which they promised to fix by 11 June 2025 ([Ei](https://ei.se/om-oss/nyheter/2025/2025-05-21-ei-har-granskat-elnatsforetagens-formaga-att-mata-och-rapportera-kvartsmatvarden)).
- **Margins over the year:** the variable costs above are September 2026's, and they change monthly.

## Notes

- **SCB's energy by contract type:** its table of delivered energy puts quarter-hour or hourly contracts at 17.8% of energy in July 2026, 28.3% in January 2026 and 35.5% in August 2025, but SCB's quality declaration says volumes aren't published for lack of quality: treat them as unreliable.
- **Ei's questions and answers:** they say monthly metering ends on 1 January 2027, which no longer holds since the power fee requirement went on 13 March 2026.
- **Not done yet:** Sweden's smaller companies; the share of the market price that Öresundskraft (5.57% of the month's average SE4 price) and Kraftringen (5% of each hour's) add per kWh; and some companies' power fees, which the plans leave out (#82).

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Vattenfall Eldistribution's 2026 price list](https://www.vattenfalleldistribution.se/globalassets/1.-privat/abonnemang-och-avgifter/om-avtal-och-avgifter/elnatsavgifter/sakringsabonnemang/prislista-sakringsabonnemang-privat-2026-01-01.pdf) | the example's plan's fees | each 1 January |
| Each company's page, linked in the first line of its plans; there's no central source | the other plans' fees and hours | each 1 January, some also during the year |
| [Ei's Elpriskollen](https://elpriskollen.se/) | the suppliers' variable costs and monthly fees, and the contracts on offer | each month |
| [SCB's contracts by type](https://www.statistikdatabasen.scb.se/pxweb/sv/ssd/START__EN__EN0301__EN0301A/SSDManadElAvtalstyp/) | the shares of contracts by type and area | each month |
| [Ei's report R2025:12](https://ei.se/download/18.3f4d315019a0f5489b07296/1761916461843/Sveriges-el-och-naturgasmarknad-2024-Ei-R2025-12.pdf) | the customers, households among them, and the share on quarter-hour or hourly contracts | each year, around October |
| [Skatteverket's energy tax rates](https://www.skatteverket.se/foretag/skatterochavdrag/punktskatter/energiskatter/skattpael) | the energy tax | each 1 January |
| [elprisetjustnu.se](https://www.elprisetjustnu.se/api/v1/prices/2026/01-01_SE3.json) | the market prices, a day a request | daily |
