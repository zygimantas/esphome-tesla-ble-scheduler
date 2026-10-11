# plans/lv/

Latvia's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On an exchange-price contract, the board's charging cost 418.39 EUR over the year from 1 October 2025: 38% less than charging at once (679.29 EUR), 25% less than the car's own schedule from midnight (560.81 EUR) and 15% less than its best start, 04:30 (492.50 EUR). Confidence in the README is High: one home in six pays the market price.

## Decisions

- **Contract:** an exchange-price (biržas cena) contract billing Nord Pool's LV price by the quarter-hour: the price the board downloads.
- **Plan:** `lv/sadales-tikls-pamata`, with the same fee at every hour, so the car's own schedule starts at midnight.
- **Billing:** each quarter-hour at its own price. Sadales tīkls' real quarter-hour data started on 1 February 2026; the hours it split evenly before that aren't modelled.
- **Margin:** 0.014 EUR per kWh with VAT (0.01157 without), AST's balancing fee included.
- **The same every hour:** nothing.
- **Left out:** Sadales tīkls' capacity fee and the suppliers' monthly fees, as they don't depend on when the car charges.
- **Market prices:** Elering's dashboard, field `data.lv`, which republishes Nord Pool's prices without an account.

## Findings

- **Who pays the market price:** about one home in six. At the end of June 2026, 15.73% of homes (143,372) had an exchange-price (biržas cena) contract, 75.69% a fixed price and 8.58% the universal service, also fixed ([SPRK's review of the second quarter, 8 September 2026](https://www.sprk.gov.lv/lv/jaunums/dubultojies-latvijas-eksportetais-elektroenergijas-daudzums-elektroenergijas-tirgus-tendencu-2026-gada-2-ceturksna-apskats)); 16.02% had an exchange-price contract at the end of 2025, in its review of 25 February 2026. A supplier with over 200,000 customers must offer one (Electricity Market Law, art. 32.3).
- **Meters and billing:** the contracts bill Nord Pool's day-ahead price for the LV area in each quarter-hour, weighted by the home's metered use in it, plus the supplier's fee: Elektrum, Enefit, Tet, Ignitis and Virši all say the price changes every 15 minutes, as does Sadales tīkls' tariff page. But from 1 October 2025 to 31 January 2026 Sadales tīkls split each hour's metered kWh into four equal quarter-hours, and that data stays ([Sadales tīkls, 27 January 2026](https://sadalestikls.lv/lv/preses-relizes/no-februara-sadales-tikls-klientiem-nodrosinas-faktiskos-15-minusu-uzskaites-datus)), so those four months were billed at each hour's average of its four prices. Real quarter-hour data started on 1 February 2026, first billed in March. Worked out from the market prices, on those 122 nights the cheapest quarter-hours of each night cost 12% more at their hours' averages, about 8 EUR with VAT in all, while charging at once barely changes.
- **VAT:** 21%, the standard rate, on every part of the bill, also from October to December 2025 ([Pievienotās vērtības nodokļa likums](https://likumi.lv/ta/id/253451-pievienotas-vertibas-nodokla-likums), art. 41). Its 12% rate for homes covers heat, not electricity (art. 42), and the suppliers' pages say "ar PVN 21%".
- **Per kWh, the same every hour:** nothing besides the margin, 0.00000 EUR with VAT:
  - the mandatory procurement component (OIK), 0.00000 EUR without and with VAT: 0 since 1 September 2022, and since 1 May 2023 the state budget covers its costs instead of the bills ([Electricity Market Law](https://likumi.lv/ta/id/108834-elektroenergijas-tirgus-likums), art. 33(6); [the public trader](https://www.eptirgotajs.lv/vairak-par-oi/));
  - the electricity tax, 0.00101 EUR without VAT, but 0.00000 for homes, which are exempt ([Elektroenerģijas nodokļa likums](https://likumi.lv/ta/id/150692-elektroenergijas-nodokla-likums), art. 6(2)(4));
  - the transmission grid's fee, which is inside Sadales tīkls' fee, so in the plan;
  - the balancing capacity fee (balansēšanas jaudas uzturēšanas maksa) of AST, the transmission operator, set for each month: in 2026 0.00122 to 0.00387 EUR without VAT, 0.00252 on average from January to September, 0.00305 with VAT; from October to December 2025 0.00300, 0.00363 with VAT ([AST](https://www.ast.lv/lv/content/regulesanas-sistemas-nebalansa-un-balansesanas-jaudas-uzturesanas-cenas)). Elektrum includes it in its fee, while Enefit, Virši and Ignitis add it as a line of its own, so the margin includes it: listed apart, it's 0.00305 EUR with VAT, and the margin without it about 0.01100.
- **Monthly fees:** Sadales tīkls' capacity fee by the main fuse, for Pamata 0.34 EUR per ampere on one phase and 0.82 on three, without VAT, which state support cut for homes of 16 to 25 A until 31 December 2025; the suppliers' monthly fees, under Margins; the protected users' discount of 20 to 25 EUR.
- **State support:** no price cap or support per kWh was found for 2025 or 2026.
- **Margins:** the offers on 11 October 2026, in EUR per kWh with VAT (without VAT in brackets). Most homes buy from Elektrum: SPRK's concentration index for homes, 0.52, means one supplier has more than half. The others come to 0.00944 to 0.01430 with the balancing fee.
  - Elektrum Dinamiskais: 0.01498 (0.01238) from 50 kWh a month, balancing included, and 1.00 to 2.50 EUR a month, 1.00 above 400 kWh ([Elektrum](https://www.elektrum.lv/lv/majai/klientiem/elektribas-podukta-izvele/elektroenergijas-produkti/)).
  - Enefit Dinamiskais: 0.01100 (0.00909), and the balancing fee on the bill, 0.01405 (0.01161) in all; its page shows no monthly fee ([Enefit](https://www.enefit.lv/lv/majai/elektriba)).
  - Tet Dinamiskais+: 0.00944 (0.00780) and 2.00 EUR a month; the older Tet Dinamiskais 0.01430 (0.01182) and none. Tet lists no balancing fee apart ([price list from 10 August 2026](https://www.tet.lv/par-mums/cenradis)).
  - Ignitis Aktīvais: 0.00970 (0.00802) and 2.00 EUR a month, until 3 November 2026, and the balancing fee, 0.01275 (0.01053) in all ([Ignitis](https://ignitis.lv/elektriba-majsaimniecibam)).
  - Virši Dinamiskā elektrība: 0.00920 (0.00760) and 1.98 EUR a month, and the balancing fee on the bill, 0.01225 (0.01012) in all ([Virši](https://www.virsi.lv/lv/privatpersonam/elektriba/elektriba)).
  - energija24.lv's comparison of 5 October 2026 has the same fees.
  - Ignitis also sells Naksnīgais to EV owners: a price fixed for 12 months, 0.07600 at 00:00 - 06:00 every night and 0.16110 at other hours, and 2.00 EUR a month. For the car alone, that's about what the cheapest quarter-hours of each night cost on an exchange contract with this margin: 0.07585, worked out from the year's market prices.
- **The plan:** about 97% of homes are on Pamata (Sadales tīkls, 5 December 2025), and every new connection starts on it. Its fee is the same in every hour of every day, with no holiday rules, so no cheap fee starts at night: 0.03962 EUR per kWh without VAT, 0.04794 with VAT, since 1 January 2024 and all of 2026 ([Sadales tīkls](https://sadalestikls.lv/lv/tarifi)). That matches the plan, and Speciālais' 0.15848 (0.19176 with VAT) matches its own; neither changed during 2026. With one grid fee at all hours, the car's own schedule has no natural start: the night's prices fall from midnight, and on the market prices the cheapest fixed start over the year was 04:30.
- **Market prices:** Elering's dashboard API republishes Nord Pool's prices without an account, a month a file, field `data.lv`, in EUR per MWh, each entry at its quarter-hour's start in Unix seconds, the end's included. October 2025 to September 2026 is complete, 96 quarter-hours a day. It's the price the exchange contracts bill: it matched Nord Pool's own LV prices on 15 and 19 September 2026 in all 96 quarter-hours.

## Changes ahead

- **Each month:** AST's balancing capacity fee, which the margin includes.
- **3 November 2026:** Ignitis Aktīvais' price, under Margins, holds until then.
- **1 January 2027:** Pamata-1 and Pamata-2 become one Pamata, and a new plan, Jaudīgais, starts ([Latvijas Vēstnesis 2026/145](https://www.vestnesis.lv/op/2026/145.DA8), which SPRK accepted on 19 August 2026). Pamata's fee drops to 0.03865 EUR per kWh without VAT, 0.04677 with it, as the plan's comment says. Jaudīgais has 0.02010 EUR per kWh without VAT, 0.02432 with it, and 1.40 EUR per ampere a month on three phases, against Pamata's 0.82, without VAT. Worked out from these, it's cheaper than Pamata above about 625 to 1,000 kWh a month on 20 to 32 A, which a home that charges a car may reach.

## Open questions

- **Margins in late 2025:** the suppliers' fees from October to December 2025, as only October 2026's offers were found.
- **Enefit's monthly fee:** its page shows none.
- **The balancing fee in Tet's and Ignitis' prices:** whether Tet's fees include AST's balancing fee, as energija24.lv says of its 0.01430, and whether Ignitis bills it apart, as energija24.lv's 0.00363 for it suggests.
- **The suppliers' shares:** how the exchange contracts split between suppliers, as SPRK gives only the total.
- **Negative prices:** whether every supplier bills a negative price as negative.

## Notes

- **Read data.lv, not data.lt:** the two areas split at times, as in 517 of March 2026's 2,977 quarter-hours and 27 on 19 September.
- **Pages that refuse scripts:** sadalestikls.lv answered an automated fetch with HTTP 451, while curl with a browser's user agent got the page; enefit.lv and ast.lv refuse curl and open in a browser.
- **Not done yet:** a plan for Jaudīgais, until 2027.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Sadales tīkls' prices](https://sadalestikls.lv/lv/tarifi) | the plans' fees and the capacity fee by main fuse | when SPRK accepts new ones, next on 1 January 2027 |
| [SPRK's quarterly review](https://www.sprk.gov.lv/lv/jaunums/dubultojies-latvijas-eksportetais-elektroenergijas-daudzums-elektroenergijas-tirgus-tendencu-2026-gada-2-ceturksna-apskats) | the shares of homes on exchange-price, fixed and universal service contracts | each quarter |
| The suppliers' offers: [Elektrum](https://www.elektrum.lv/lv/majai/klientiem/elektribas-podukta-izvele/elektroenergijas-produkti/), [Enefit](https://www.enefit.lv/lv/majai/elektriba), [Tet](https://www.tet.lv/par-mums/cenradis), [Ignitis](https://ignitis.lv/elektriba-majsaimniecibam), [Virši](https://www.virsi.lv/lv/privatpersonam/elektriba/elektriba) | the margins and monthly fees | any time |
| [AST's balancing prices](https://www.ast.lv/lv/content/regulesanas-sistemas-nebalansa-un-balansesanas-jaudas-uzturesanas-cenas) | the balancing capacity fee | each month |
| [Electricity Market Law](https://likumi.lv/ta/id/108834-elektroenergijas-tirgus-likums), [the public trader](https://www.eptirgotajs.lv/vairak-par-oi/) | the mandatory procurement component (OIK) | when the law changes |
| [Elektroenerģijas nodokļa likums](https://likumi.lv/ta/id/150692-elektroenergijas-nodokla-likums) | the electricity tax and the homes' exemption | when the law changes |
| [Elering's dashboard](https://dashboard.elering.ee/api/nps/price?start=2026-01-01T00:00:00.000Z&end=2026-02-01T00:00:00.000Z) | the market prices, a month a request | daily |
