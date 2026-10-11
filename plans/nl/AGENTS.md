# plans/nl/

The Netherlands' research, checked in October 2026, for this folder's README.md and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a dynamic contract billing the Dutch day-ahead price, the board's charging cost 816.87 EUR over the year from 1 October 2025: 17% less than charging at once (982.40 EUR), 7% less than the car's own schedule from midnight (876.81 EUR) and 1% less than its best start, 03:00 (827.67 EUR). Confidence in the README is High: 8% of homes pay the market price.

## Decisions

- **Contract:** a dynamic contract billing the Dutch day-ahead price.
- **Plan:** none, as the grid has no fee per kWh. Settled with the owner: no plans here until time-of-use network fees come, planned from 2029, and then one per operator. Without a cheap fee to start at, the car's own schedule starts at midnight.
- **Billing:** each hour at the average of its four quarter-hours, as about half the dynamic contracts bill.
- **Margin:** 0.01900 EUR per kWh with VAT (0.01570 without), the middle of the five biggest offers, weighted by their share of dynamic customers.
- **The same every hour:** the energy tax, 0.11085 EUR per kWh with VAT, 2026's, all year.
- **Left out:** the grid operator's yearly charges, the yearly tax reduction and the supplier's monthly fee, as they don't depend on when the car charges.
- **Market prices:** SMARD's series 256.

## Findings

- **Who pays the market price:** dynamic contracts are common and growing. 8% of households had one in August 2026, 55% a fixed price and 37% a variable one ([ACM, 13 August 2026](https://www.acm.nl/nl/publicaties/energiemonitor-acm-toename-aantal-dynamische-contracten-zet-door)). For electricity that was 573,000 consumers on 1 January 2026, 7% of households, and 651,000 on 1 June 2026, the ACM's figures in Solar Magazine ([January](https://solarmagazine.nl/nieuws-zonne-energie/i43038/573-000-nederlanders-hebben-dynamisch-energiecontract-marktaandeel-anwb-krimpt), [June](https://solarmagazine.nl/nieuws-zonne-energie/i44318/zonneplan-wint-marktaandeel-aantal-dynamische-contracten-blijft-groeien)). The suppliers' shares on 1 June 2026: ANWB Energie 25% to 30%, Zonneplan 20% to 25%, NextEnergy 10% to 15%, and Budget Energie, Frank Energie, Eneco and Tibber 5% to 10% each.
- **Billing:** all bill the Dutch day-ahead price (EPEX's, the same as Nord Pool's), set by the quarter-hour since 1 October 2025, plus the supplier's margin, the energy tax and VAT. [Tibber](https://tibber.com/nl/stroomprijs) and [Zonneplan](https://www.zonneplan.nl/energie/dynamische-energieprijzen/stroomprijs) bill each quarter-hour, and so do Coolblue Energie and Noord Energie (Overstappen.nl, 2025). [Frank Energie](https://www.frankenergie.nl/nl/dynamisch-energiecontract) lets the customer choose. ANWB Energie, NextEnergy and Essent bill each hour at the average of its four quarter-hours: [NextEnergy's](https://www.nextenergy.nl/actuele-energieprijzen) and [Essent's](https://www.essent.nl/energie/energiecontract/dynamisch) pages say so, and ANWB's hourly prices are that average to the cent, as its price API has no quarter-hours (https://api.anwb.nl/energy/energy-services/v2/tarieven/electricity?startDate=2026-09-14T22:00:00.000Z&endDate=2026-09-15T21:59:59.999Z&interval=HOUR). Eneco and Vattenfall price by the hour too. So about half the dynamic customers pay hourly averages. Worked out roughly from the year's market prices, with the cheapest quarter-hours before VAT rather than the board's make_schedule(), the saving on an hourly contract is about a tenth smaller than on one billing each quarter-hour, as the quarter-hours picked cost about 0.7 cents per kWh more at their hour's average.
- **VAT:** 21% on the whole bill, the energy tax and the margin included, in 2025 and 2026 ([Rijksoverheid](https://www.rijksoverheid.nl/vraag-en-antwoord/energie-thuis/opbouw-energierekening)). Frank Energie's price API adds 21% to the market price on all 56 days checked across the year.
- **Per kWh, the same every hour:** only the energy tax (energiebelasting): 0.09161 EUR without VAT and 0.11085 EUR with VAT in 2026, 0.10154 and 0.12286 EUR in 2025. The rate is for the first 10,000 kWh a year (0 to 2,900 and 2,901 to 10,000 kWh pay the same), which a home with the car stays within. The [Belastingdienst's table](https://www.belastingdienst.nl/wps/wcm/connect/bldcontentnl/belastingdienst/zakelijk/overige_belastingen/belastingen_op_milieugrondslag/energiebelasting/energiebelasting) has the rates without VAT. Rijksoverheid gives 11.1 cents with VAT for 2026, [Zonneplan's page](https://www.zonneplan.nl/energie/dynamische-energieprijzen/kwh-prijs) 0.11085 EUR for 2026 and 0.12286 EUR for 2025, and Frank Energie's API bills 0.12286 EUR until 31 December 2025 and 0.11085 EUR from 1 January 2026. The ODE levy has been 0 since 2023, and the reduced rate for charging installations applies in neither year. Nothing else changes with the hour: no holiday rules apply to dynamic prices or the grid.
- **Tax reduction:** the yearly reduction of the energy tax (vermindering energiebelasting) is a fixed amount: 519.80 EUR without VAT and 629 EUR with VAT in 2026, 524.95 EUR without VAT in 2025.
- **Net metering:** the salderingsregeling nets a solar home's yearly feed-in against its use, energy tax included, until it ends on 1 January 2027 ([Rijksoverheid](https://www.rijksoverheid.nl/themas/klimaat-milieu-en-natuur/energie-thuis/salderingsregeling)). It doesn't change which night hours are cheapest.
- **Margins:** the five biggest offers' margins (inkoopvergoeding) per kWh with VAT, read on 11 October 2026, with each one's monthly fee:
  - ANWB Energie: 0.01800 EUR (0.01488 without VAT), 8.50 EUR a month ([prices](https://www.anwb.nl/energie/actuele-tarieven), [monthly fee](https://www.anwb.nl/energie/vaste-leveringskosten-energie)). Its hourly prices are the market price with VAT plus 0.11085 and 0.01800 EUR.
  - Zonneplan: 0.02000 EUR (0.01653), the handling fee in every price on [its page](https://www.zonneplan.nl/energie/dynamische-energieprijzen/stroomprijs), 6.25 EUR a month (Overstappen.nl, 5 October 2026).
  - NextEnergy: 0.02100 EUR (0.01736), 5.99 EUR a month ([NextEnergy](https://www.nextenergy.nl/dynamische-energie)). A customer pays the margin of when they joined.
  - Frank Energie: 0.01815 EUR (0.01500) on all 56 days checked from 1 October 2025 to 30 September 2026 in its price API (https://graphql.frankenergie.nl, field sourcingMarkupPrice), 6.00 EUR a month (Gaslicht, 7 October 2026).
  - Tibber: 0.01800 EUR (0.01488), 6.99 EUR a month ([Tibber](https://tibber.com/nl/stroomprijs)). Its prices shown on 11 October 2026 were the market price with VAT plus 0.11085 and 0.01800 EUR.
  - In October to December 2025 ANWB's margin was about 0.02000 EUR, worked out from its hourly prices, and its monthly fee 7.00 EUR. Frank Energie's was the same as in 2026.
- **The grid:** no fee per kWh, so no plan. Grid operators charge a home a fixed amount a year by the size of its connection, which the supplier bills monthly. For 3x25A in 2026 [Liander](https://www.liander.nl/-/media/files/tarieven/consument/2026/jaarlijkse-netwerkkosten-stroom-2026-v10.pdf) charges 478.04 EUR with VAT a year and [Stedin](https://www.stedin.net/-/media/project/online/files/tarieven/tarieven-2026/tarieven-2026_elektriciteit_kleinverbruik.pdf) 476.76 EUR, in fixed parts only: connection, transport, capacity and meter. Tibber's page agrees: its kWh price has the market price, margin, energy tax and VAT, not the grid charges. Without a plan there's no cheap fee to start the car's own schedule at.
- **Off-peak hours:** the off-peak (dal) hours of fixed and variable contracts with two rates are usually 23:00 - 07:00 on weekdays and all day at weekends and on public holidays, in some regions from 21:00 ([Liander](https://www.liander.nl/tarieven/enkeltarief-of-dubbeltarief)).
- **Market prices:** SMARD's chart data, series 256: the Netherlands' day-ahead prices in euros per MWh, by the quarter-hour from 1 October 2025, a week a file from Monday 00:00 German time, without an account or key. https://www.smard.de/app/chart_data/256/DE/256_DE_quarterhour_1759096800000.json is the week from Monday 29 September 2025, https://www.smard.de/app/chart_data/256/DE/index_quarterhour.json lists the weeks, and market.h's smard_url(256, ...) makes the URL. It has all 35,136 quarter-hours from 1 October 2025 to 1 October 2026. It's the price the contracts bill: it matched Nord Pool's NL prices, which the board downloads, on all 4,800 quarter-hours from 12 August to 30 September 2026, Energy-Charts' (https://api.energy-charts.info/price?bzn=NL&start=2025-10-01&end=2025-10-31) on every quarter-hour of October and December 2025, and the market prices Frank Energie bills on 53 days spread over the year, both clock changes included (5,088 quarter-hours). Tibber's, Zonneplan's and ANWB's prices follow from it too.

## Changes ahead

- **End of 2026:** the ACM planned to decide on grid fees by the time of day for households, to start shortly after large users' in 2028 ([Focus op Energie 2026](https://www.acm.nl/system/files/documents/focus-op-energie-2026_0.pdf)). They're planned from 2029, and plans come then, one per operator.
- **1 January each year:** the energy tax's new rate and yearly reduction (0.12286 EUR with VAT in 2025, 0.11085 EUR in 2026), and the grid operators' new yearly charges. Net metering ends on 1 January 2027.

## Open questions

- **Who bills each hour or quarter-hour:** only the suppliers' pages of October 2026 say, and for Coolblue Energie, Noord Energie and Budget Energie only comparison sites of 2025.
- **The suppliers' shares:** they're the ACM's ranges as Solar Magazine reports them; the ACM's own page gives only the total.
- **ANWB's margin before 2026:** worked out from its prices, rounded to the cent, and not stated anywhere; it changed around 1 January 2026.
- **Monthly fees:** Zonneplan's and Frank Energie's come from comparison sites.
- **Grid fees by the time of day:** whether the grid operators' proposal reached the ACM, and when such fees start, is unconfirmed; the ACM's decision, planned for the end of 2026, would say.

## Notes

- **Margins on comparison sites:** they can differ from the suppliers' own. Overstappen.nl gave Zonneplan's as 0.02200 EUR on 5 October 2026, and NextEnergy's comparison table says 0.02190 EUR, against the 0.02000 and 0.02100 EUR in their prices.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [ACM's energy monitor](https://www.acm.nl/nl/publicaties/energiemonitor-acm-toename-aantal-dynamische-contracten-zet-door) | the shares of households on dynamic, fixed and variable contracts | with each monitor |
| [Solar Magazine on the ACM's count of 1 June 2026](https://solarmagazine.nl/nieuws-zonne-energie/i44318/zonneplan-wint-marktaandeel-aantal-dynamische-contracten-blijft-groeien) | the number of dynamic contracts and the suppliers' shares | with each of the ACM's counts |
| [Belastingdienst's energy tax table](https://www.belastingdienst.nl/wps/wcm/connect/bldcontentnl/belastingdienst/zakelijk/overige_belastingen/belastingen_op_milieugrondslag/energiebelasting/energiebelasting) | the energy tax, without VAT | each 1 January |
| The suppliers' prices: [ANWB Energie](https://www.anwb.nl/energie/actuele-tarieven), [Zonneplan](https://www.zonneplan.nl/energie/dynamische-energieprijzen/stroomprijs), [NextEnergy](https://www.nextenergy.nl/dynamische-energie), [Frank Energie's API](https://graphql.frankenergie.nl) and [Tibber](https://tibber.com/nl/stroomprijs) | the margins and monthly fees | any time |
| [Liander's](https://www.liander.nl/-/media/files/tarieven/consument/2026/jaarlijkse-netwerkkosten-stroom-2026-v10.pdf) and [Stedin's](https://www.stedin.net/-/media/project/online/files/tarieven/tarieven-2026/tarieven-2026_elektriciteit_kleinverbruik.pdf) 2026 charges | the grid operators' yearly charges | each 1 January |
| [ACM's Focus op Energie 2026](https://www.acm.nl/system/files/documents/focus-op-energie-2026_0.pdf) | when grid fees by the time of day come | a decision planned for the end of 2026 |
| [SMARD's weeks for series 256](https://www.smard.de/app/chart_data/256/DE/index_quarterhour.json) | the market prices, a week a file | daily |
