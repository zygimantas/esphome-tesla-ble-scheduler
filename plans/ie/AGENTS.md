# plans/ie/

Ireland's research, checked in October 2026, for the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On Electric Ireland's Night Boost, a fixed plan for electric cars with a cheap night, the board's charging cost 362.67 EUR over the year from 1 October 2025: 71% less than charging at once (1,253.33 EUR), and the same as the car's own schedule from 02:00, which is also its best start (362.67 EUR). Confidence in the README is High: electric car owners usually take a fixed plan with a cheap night, cheaper than dynamic contracts.

## Decisions

- **Contract:** Electric Ireland's Night Boost, a fixed plan for electric cars with a cheap night, at its rates from 1 July 2026 all year. Electric car owners usually take such a plan, and dynamic contracts, offered only since mid-2026, cost more: each night's two cheapest half-hours cost 0.19944 EUR per kWh on Electric Ireland's dynamic plan, against 0.10880 at Night Boost's rate.
- **Plan:** Night Boost as a custom plan: boost 02:00 - 04:00 0.10880, night 23:00 - 08:00 0.18540 and day 0.37600 EUR per kWh, with VAT and its discount. The car's own schedule starts at 02:00, when the boost does.
- **Margin:** none on top, as it's in Night Boost's rates.
- **The same every hour:** nothing: homes pay no Electricity Tax, and the PSO levy is per month.
- **Left out:** the standing charge and the PSO levy, as they're per month and don't depend on when the car charges.
- **Market prices:** none, as Night Boost's rates are fixed. SEMOpx's prices for the year served to compare it with the dynamic contracts.

## Findings

- **Who pays the market price:** nearly all homes pay a supplier's fixed unit rates: 24 hours, day and night, or smart time of use. Dynamic contracts are rare, and on offer only since May and June 2026. Neither the CRU nor a supplier has published how many households use one.
- **Dynamic contracts:** the CRU's decision CRU2024121 (26 September 2024) obliges the five suppliers with over 200,000 electricity customers to offer households a Standard Dynamic Price Contract: Bord Gáis Energy, Electric Ireland, Energia, PrePay Power / Yuno Energy and SSE Airtricity ([the CRU's decision](https://cruie-live-96ca64acab2247eca8a850a7e54b-5b34f62.divio-media.com/documents/Dynamic_Electricity_Price_Tariffs_-_Decision_Paper_002.pdf)). The deadline was 1 October 2025; CRU202517 moved it to 1 June 2026 ([the CRU's notice to suppliers](https://cruie-live-96ca64acab2247eca8a850a7e54b-5b34f62.divio-media.com/documents/Notification_to_Suppliers_extending_timeline_for_introduction_of_Standard_Dynamic_Pric.PDF)), so none was on offer from October 2025 to mid-May 2026. In October 2026 Electric Ireland (base rates from 19 May 2026, sign-up by phone only), Energia (Smart Track, from 2 June 2026), Bord Gáis Energy (Smart Dynamic, June 2026), Yuno Energy and PrePayPower (from June 2026) offer one; SSE Airtricity's site still says its plan isn't available yet ([SSE Airtricity](https://www.sseairtricity.com/ie/home/help-centre/dynamic-tariffs/when-will-the-dynamic-tariff-plan-be-available)). The bill has a standing charge, a base unit rate (flat, or day, night and peak) and a dynamic unit rate: SEMOpx's half-hourly day-ahead price for ROI-DA, capped at €0.50 per kWh without VAT (the floor of the capacity market's Reliability Option strike price), with no floor, so negative prices pass through. The cap never applied: the year's dearest half-hour was €465.77 per MWh, and 59 of its 17,568 were negative. The base unit rate carries the network charges and the supplier's other costs and margin; the PSO levy and VAT come on top.
- **Meters and billing:** a dynamic contract needs a smart meter with a strong signal (CTF 4) and online billing. The standard smart bands are night 23:00 - 08:00, peak 17:00 - 19:00 and day the rest, on local time every day (ESB Networks' statement of charges; Energia says the same). Day and night meters without smart services keep night at 23:00 - 08:00 winter time, so 00:00 - 09:00 in summer.
- **VAT:** 9% on household electricity all year, the dynamic unit rate included: Bord Gáis shows the [day-ahead prices](https://www.bordgaisenergy.ie/day-ahead-market-prices) without VAT and adds it on the bill. The reduced rate was to end on 1 November 2025; Budget 2026 extended it to 31 December 2030 ([Budget 2026's tax policy changes, 2.6](https://assets.gov.ie/static/documents/15569ee3/Budget_2026_Tax_Policy_Changes_Book.pdf)), and Budget 2027, in October 2026, left it. Suppliers' prices agree: Bord Gáis's 17.45 c without VAT is 19.02 c with it, and Electric Ireland's July 2026 notice names the 9% rate.
- **Per kWh, the same every hour:** nothing, 0.00000 with and without VAT. Electricity Tax doesn't apply to domestic use ([Revenue](https://www.revenue.ie/en/companies-and-charities/excise-and-licences/energy-taxes/electricity-tax/index.aspx)). The PSO levy is per month, without VAT: €2.01 from 1 October 2025, €1.46 from 1 December 2025 (CRU2025164) and €0.51 from 1 October 2026. Capacity, imperfections and market operator charges are the supplier's, inside its rates.
- **Dynamic base rates:** with a dynamic contract, a kWh costs the market price with VAT plus the base unit rate of its band, which works as the plan. Electric Ireland's, the biggest supplier's, from 19 May 2026, with VAT: day 0.19810, night 0.08520, peak 0.22550 (without VAT 0.18170, 0.07820, 0.20690) ([Electric Ireland](https://www.electricireland.ie/residential/electricity-and-gas/dynamic-price-plans)). Its page gives no hours; taken in the standard smart bands, its cheap rate starts at 23:00:

  ```yaml
  # Prices with VAT: https://www.electricireland.ie/residential/electricity-and-gas/dynamic-price-plans
  name: Electric Ireland dynamic base rates
  currency: EUR
  calendar:
    jan-dec:
      mon-sun: night 08:00 day 17:00 peak 19:00 day 23:00 night
  rates:
    night: 0.08520
    day: 0.19810
    peak: 0.22550
  ```

  Others, with VAT: Energia day 0.21970, night 0.12510, peak 0.22920 from 2 June 2026 ([Energia](https://www.energia.ie/about-energia/our-tariffs)); Bord Gáis one flat rate, 0.16730 until 8 October 2026, then 0.19020 ([Bord Gáis](https://www.bordgaisenergy.ie/home/our-tariffs)); Yuno day 0.24790, night 0.11280, peak 0.27460, from a comparison site.
- **Plans for electric cars:** the usual one is a fixed plan with a cheap night window, which the car's own schedule reaches, rather than a dynamic contract. Electric Ireland's Home Electric+ Night Boost from 1 July 2026, with VAT and its 5.5% direct debit and online billing discount: day 0.37600 (08:00 - 23:00), night 0.18540 (23:00 - 08:00) and boost 0.10880 (02:00 - 04:00) ([Electric Ireland](https://www.electricireland.ie/residential/electricity-and-gas/ev-night-boost)). Its unit rates had stayed the same through the winter and rose 9.5% on 1 July 2026; worked out from that, they were about 0.34338, 0.16932 and 0.09936 before. Bord Gáis's Urban EV Smart from 9 October 2026, before discounts: EV rate 0.11520, day 0.45220, night 0.34110, peak on weekdays 0.63080.
- **Dynamic contracts against Night Boost:** worked out from the year's market prices, the two cheapest half-hours of each night cost 0.19944 per kWh on Electric Ireland's dynamic plan, 0.22658 charging from 23:00 and 0.41772 at once from 18:00, against 0.10880 at Night Boost's boost rate.
- **The network charges:** homes don't pay them on their own: suppliers pay ESB Networks (DUoS) and EirGrid (TUoS) and put them in their unit rates, and in a dynamic contract's base unit rate. The 2025/26 tariffs cover 1 October 2025 to 30 September 2026, the whole year. DUoS for an urban smart home (DG1, time of use), without VAT: day 6.206 c, night 0.802 c, peak 6.827 c per kWh ([CRU2025118, Appendix B](https://api.cru.ie/documents/30141/Electricity_Distribution_Network_Allowed_Revenues_and_DUoS_Tariffs_Information_Paper.PDF)). TUoS (DTS-D2 at low voltage, with distribution losses), without VAT: day 4.36288 c, night 3.30569 c, which per MWh is a transfer charge of 4.4581 and 4.3927, system services of 29.0912 and 28.6642, and network capacity of 10.0795 by day only, 08:00 - 23:00 ([EirGrid's Statement of Charges 2025/2026, Appendix 1](https://www.eirgrid.ie/sites/default/files/publications/2025-2026-Statement-of-Charges.pdf)). Together, with VAT, for reference only, as they're inside the supplier's rates:

  ```yaml
  # Prices with VAT: https://api.cru.ie/documents/30141/Electricity_Distribution_Network_Allowed_Revenues_and_DUoS_Tariffs_Information_Paper.PDF
  name: DUoS and TUoS, urban smart, 2025/26
  currency: EUR
  calendar:
    jan-dec:
      mon-sun: night 08:00 day 17:00 peak 19:00 day 23:00 night
  rates:
    night: 0.04477
    day: 0.11520
    peak: 0.12197
  ```

  From 1 October 2026 DUoS is day 7.229 c, night 0.934 c, peak 7.952 c (ESB Networks' Statement of Charges, revision 23).
- **Market prices:** SEMOpx's, without an account, in euros per MWh. Its API keeps only the last 365 days (on 11 October 2026 the oldest is delivery day 13 October 2025): [the list](https://reports.semopx.com/api/v1/documents/static-reports?DPuG_ID=EA-001&ExcludeDelayedPublication=0&ResourceName=MarketResult_SEM-DA_PWR-MRC-D%2B1&Date=%3E2026-09-01%3C2026-10-03&sort_by=Date&order_by=ASC&page_size=100), then each day by its `_id`, like [the one for delivery on 2 September 2026](https://reports.semopx.com/api/v1/documents/6a96b3c89620d92e0e7531bb), which the board's add_semopx() (scheduler/market.h) reads. Older days are in SEMOpx's yearly archives, [2025's](https://www.semopx.com/sites/semopx/files/2026-01/DAM%20IDM%20MarketResults%202025.zip) (34 MB, all of 2025's auctions) and [2026's](https://www.semopx.com/sites/semopx/files/2026-09/DAM%20IDM%20MarketResults%202026.zip) (31 MB, auctions to 31 August 2026), with a CSV a day, like `MarketResult_SEM-DA_PWR-MRC-D+1_20250930100000_20250930105930.csv` for delivery on 1 October 2025: after the line `Market;ROI-DA` come `Index prices;30;EUR`, the half-hours' starts in UTC and their prices, separated by semicolons, with decimal commas. Archive and API agree, and together hold all 17,568 half-hours from 1 October 2025 to 1 October 2026. The day-ahead market went from hours to half-hours with delivery on 1 October 2025, and a delivery day runs from 23:00 to 23:00 Irish time.

## Changes ahead

- **Any time:** the suppliers' unit rates. Night Boost's rose 9.5% on 1 July 2026, and Bord Gáis's changed in October 2026.
- **1 October each year:** the network tariffs, DUoS and TUoS, for October to September, which suppliers put in their unit rates. The PSO levy changed on 1 October 2025 and 1 October 2026, and on 1 December 2025 in between.
- **31 December 2030:** the reduced 9% VAT rate ends, as Budget 2026 set it.

## Open questions

- **How many homes have a dynamic contract:** nothing is published.
- **Which plan electric car owners are on:** nothing is published either.
- **SSE Airtricity's delay:** its site still says its dynamic plan isn't available yet; whether that has consequences isn't known.
- **The hours of Electric Ireland's dynamic bands:** its page gives none. They're taken as the standard smart bands, with the peak every day, while Bord Gáis's own peak is on weekdays only.
- **Yuno's base rates:** from a comparison site only.
- **Night Boost before July 2026:** its rates are worked out from the 9.5% rise.
- **Bord Gáis's EV rate:** its hours aren't known.
- **The cap:** Energia ties it to the strike price floor, which may change weekly; the others say €0.50, and no half-hour reached it.

## Notes

- **No plans here:** the supplier's prices include the network charges (plans/README.md), so this folder has none, and the board downloads SEMOpx's prices for Ireland. Night Boost is a custom plan in test/savings_example.cpp, and the two YAML blocks under Findings, in the plans' format, are for reference.
- **Ember's hourly file:** it has Ireland's prices a day late and only each hour's first half: don't use it.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Electric Ireland's Night Boost](https://www.electricireland.ie/residential/electricity-and-gas/ev-night-boost) | the example's plan: its rates, hours and discount | any time |
| [Electric Ireland's dynamic price plans](https://www.electricireland.ie/residential/electricity-and-gas/dynamic-price-plans) | its dynamic base rates | any time |
| [Energia's tariffs](https://www.energia.ie/about-energia/our-tariffs) | its dynamic base rates | any time |
| [Bord Gáis Energy's tariffs](https://www.bordgaisenergy.ie/home/our-tariffs) | its dynamic base rate | any time |
| [SSE Airtricity on its dynamic plan](https://www.sseairtricity.com/ie/home/help-centre/dynamic-tariffs/when-will-the-dynamic-tariff-plan-be-available) | whether it offers one yet | any time |
| [CRU2025118](https://api.cru.ie/documents/30141/Electricity_Distribution_Network_Allowed_Revenues_and_DUoS_Tariffs_Information_Paper.PDF) | DUoS for 2025/26 | each 1 October |
| [EirGrid's Statement of Charges 2025/2026](https://www.eirgrid.ie/sites/default/files/publications/2025-2026-Statement-of-Charges.pdf) | TUoS for 2025/26 | each 1 October |
| [Budget 2026's tax policy changes](https://assets.gov.ie/static/documents/15569ee3/Budget_2026_Tax_Policy_Changes_Book.pdf) | the 9% VAT rate, to 31 December 2030 | each budget, in October |
| [Revenue on Electricity Tax](https://www.revenue.ie/en/companies-and-charities/excise-and-licences/energy-taxes/electricity-tax/index.aspx) | that homes don't pay it | with the law |
| [SEMOpx's API](https://reports.semopx.com/api/v1/documents/static-reports?DPuG_ID=EA-001&ExcludeDelayedPublication=0&ResourceName=MarketResult_SEM-DA_PWR-MRC-D%2B1&Date=%3E2026-09-01%3C2026-10-03&sort_by=Date&order_by=ASC&page_size=100) | the market prices, a document a day, for the last 365 days | daily |
| SEMOpx's yearly archives, [2025](https://www.semopx.com/sites/semopx/files/2026-01/DAM%20IDM%20MarketResults%202025.zip) and [2026](https://www.semopx.com/sites/semopx/files/2026-09/DAM%20IDM%20MarketResults%202026.zip) | older days, a CSV a day | each year |
