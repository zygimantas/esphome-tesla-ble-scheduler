# plans/si/

Slovenia's research, checked in October 2026, for this folder's plan and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a dynamic contract billing BSP SouthPool's price by the hour, as Bisol's does, the board's charging cost 592.67 EUR over the year from 1 October 2025: 28% less than charging at once (822.07 EUR), 17% less than the car's own schedule from 22:00 (712.95 EUR) and 0% less than its best start, 03:00 (593.29 EUR). Confidence in the README is Low: 0.3% of homes pay the market price.

## Decisions

- **Contract:** a dynamic contract billing BSP SouthPool's day-ahead price, like Bisol's, the biggest supplier of dynamic homes.
- **Plan:** `si/omreznina`. The car's own schedule starts at 22:00, when the night block does.
- **Billing:** each hour at the average of its four quarter-hours (SIPXhourly), as Bisol, ECE, GEN-I and SunContract bill.
- **Margin:** 0.01586 EUR per kWh with VAT, Bisol's.
- **The same every hour:** 0.003 EUR per kWh with VAT: the excise, the energy efficiency contribution and the market operator's contribution.
- **Left out:** the OVE and SPTE contribution and the network charge's power part, both per kW (#82), and the monthly fees, which don't depend on when the car charges; the flatter network prices of October 2025 to February 2026, as 2026's are used all year.
- **Market prices:** SMARD's series 260.

## Findings

- **Who pays the market price:** few homes. The Energy Agency (AGEN-RS) counted 2,259 households on dynamic contracts at four suppliers in 2025, 0.26% of the 883,278 households and 0.24% of their energy, up from 385 in 2024 ([report for 2025](https://www.agen-rs.si/-/porocilo-o-stanju-na-podrocju-energetike-v-sloveniji-v-letu-2025), 1 July 2026, pages 29 and 159 to 163). Bisol Energija supplied 71% of that energy, ECE 21%, SunContract 8% and GEN-I 1% (rounded). Households mostly buy at flat supplier price lists (Energy price, below), and only 628 had fixed-price contracts.
- **The offers:** in September 2026 Bisol, ECE, Elektro energija, Energija plus, GEN-I and SunContract offered dynamic contracts ([ZPS's table](https://www.zps.si/aktualne-vsebine/trzni-pregledi/pregled-cen-elektricne-energije-in-zemeljskega-plina-152), checked 2 September 2026), and Petrol since 1 July 2026. They bill BSP SouthPool's day-ahead price plus a fixed markup. SunContract bills hours at or below 0 as 0; the rest, by ZPS's check, pass negative prices on.
- **Meters and billing:** the offers need a meter that reads every 15 minutes, as over 90% of Slovenian meters do. Bisol also takes homes without one and spreads their use over the national load curve, so shifting gains nothing there. Bisol, ECE, GEN-I and SunContract bill the hourly index SIPXhourly, which BSP defines as the average of the hour's 15-minute prices ([BSP's definition](https://web.archive.org/web/20251206130833/https://www.bsp-southpool.com/sipx.html)), times the hour's use ([ECE's terms](https://www.ece.si/app/uploads/2026/07/SPLOSNI-POGOJI-ECE-FLEKS_1.9.2026.pdf) spell it out), so the board's choice of quarter-hours within an hour changes nothing on their bills. Only Petrol bills each quarter-hour at its own price ([terms](https://www.petrol.si/binaries/content/assets/www/2026/dokumenti/ee/pogodbeni-pogoji-za-dobavo-elektricne-energije-po-dinamicnih-cenah-1-01-2026.pdf)). Worked out with the example's method, with each quarter-hour billed at its own price, the board's charging would have cost 30.0% less than charging at once, 21.8% less than the car's own schedule from 22:00 and 4.3% less than its best start.
- **VAT:** 22%, all year ([FURS](https://www.fu.gov.si/en/taxes_and_other_duties/areas_of_work/value_added_tax_vat/) gives the general rate, and every 2026 price list here shows it), on everything below, the excise included: the ministry's Q1 2026 price adds up that way. The temporary 9.5% of September 2022 to May 2023 is over.
- **Per kWh, the same every hour:** 0.00246 EUR without VAT, 0.00300 with it (0.0030012 before rounding), unchanged from October 2025 to September 2026. A [2026 price guide](https://www.varcevanje-energije.si/aktualne-novice/cena-elektrike-2020-drasticno-navzdol/) lists all three, with VAT as 0.0018666, 0.000976 and 0.0001586:
  - the excise (trošarina), 0.00153 EUR without VAT, 0.00187 with it: 1.53 EUR per MWh for homes since 1 August 2022 ([Uradni list 99/2022](https://www.uradni-list.si/_pdf/2022/Ur/u2022099.pdf), article 1 point 7). The six amendments from November 2025 to June 2026 change only fuels, and the [consolidated text](https://www.racunovodstvo.net/zakonodaja/uredba-o-dolocitvi-zneska-trosarine-za-energente-in-elektricno-energijo/1-clen) valid from 16 June 2026 still has 1.53. The ministry's [Q1 2026 prices](https://www.energetika-portal.si/nc/novica/n/cene-elektricne-energije-v-prvem-cetrtletju-2026/) show it too;
  - the energy efficiency contribution (URE), 0.00080 EUR without VAT, 0.00098 with it, the same since 2017 ([energy portal](https://www.energetika-portal.si/podrocja/energetika/prihranki-energije/prispevek-za-energetsko-ucinkovitost/));
  - the market operator's contribution (Borzen), 0.00013 EUR without VAT, 0.00016 with it ([Borzen](https://borzen.si/Portals/0/Trg%20z%20elektriko/Dokumenti/Financiranje%20GJS.pdf)).
- **Per kW and per month, left out:**
  - the OVE and SPTE contribution, per kW of billing power: 0.77562 EUR per kW a month without VAT (0.94626 with it) from 1 July 2025, half of it from 1 November 2025 to 28 February 2026, the full rate again from 1 March 2026 ([energy portal](https://www.energetika-portal.si/podrocja/energetika/prispevek-za-obnovljive-vire/)). ZSROVE-1 ([Uradni list 112/2025](https://www.uradni-list.si/glasilo-uradni-list-rs/vsebina/2025-01-3977)) makes it a charge per kWh, but only from a new decree, due by the end of 2026. The estimate behind the law was 0.00845 EUR per kWh without VAT ([Žurnal24](https://www.zurnal24.si/pod-streho/varcna-hisa/nov-obracun-prispevka-na-racunu-za-elektriko-kdo-bo-placal-vec-in-kdo-manj-448027), 23 October 2025);
  - the network charge's power part, which does depend on when the car charges: a block's agreed power is set from its five highest quarter-hour peaks in a past year ([act of 29 September 2025](https://www.agen-rs.si/documents/10926/554036/Sprejeti-akt_20251006/893bdb24-a92b-4596-a044-df93eb81b186), article 12), and in 2026 a kW costs 3.82301 EUR a month in block 1, 1.09230 in block 2, 0.28902 in block 3, 0.02436 in block 4 and 0.00245 in block 5 (user group 0, without VAT). An 11 kW car charging in an evening block would add far more than one at night;
  - the suppliers' monthly fees, 1.21 to 3.62 EUR with VAT (Margins, below).
- **Margins:** Bisol's, the example's, is 0.01586 EUR per kWh with VAT (0.01300 without), near the mean of the seven offers, 0.01285 without VAT. ZPS's tables of 3 March, 2 June and 2 September 2026 have the same markups. AGEN-RS says markups are fixed, 6.5 to 14 EUR per MWh, or 6 to 10% of the price. Markups per kWh without and with VAT, and monthly fees:
  - Bisol Dinamični 0.01300 and 0.01586, 1.63 EUR (1.99 with VAT) ([Bisol](https://www.bisol-energija.si/dinamicni-gospodinjstva), 11 October 2026);
  - ECE Fleks 0.01400 and 0.01708, 1.21 EUR with VAT;
  - Elektro energija Dinamična oskrba 0.01199 and 0.01463, 3.62 EUR with VAT, 0.99 less with an e-bill;
  - GEN-I Dinamični 0.01199 and 0.01463, from 2 October 2024, 2.97 EUR (3.62 with VAT), 1.97 with an e-bill ([GEN-I](https://www.gen-i.si/dom/elektricna-energija/ceniki-in-akcije/gen-i-dinamicni-gospodinjski-odjemalci/));
  - SunContract Dinamika 0.01200 and 0.01464, 2.43 EUR with VAT;
  - Energija plus Dinamične cene 0.01500 and 0.01830, no fee given;
  - Petrol 0.01195 and 0.01458 from 1 July 2026, 0.01295 and 0.01580 in its terms from 1 January 2026, 1.98 EUR (2.42 with VAT) ([price list](https://www.petrol.si/binaries/content/assets/www/2026/dokumenti/ee/dinamicni-cenik_go-mpo-odjem_1_7_2026.pdf)).
- **Energy price:** no cap or regulated energy price: the government's last cap (VT 0.08400, MT 0.07000 and ET 0.07700 EUR per kWh without VAT) ended on 28 February 2025 ([ministry](https://www.energetika-portal.si/podrocja/energetika/ukrepi-za-omilitev-draginje-na-podrocju-energetike/elektricna-energija/)). A household on a supplier's price list pays a flat price: in September 2026 ECE's and Energija plus's were VT 0.12479, MT 0.10285 and ET 0.11378 EUR per kWh without VAT (0.15224, 0.12548 and 0.13881 with it), as in ZPS's table. VT is workdays from 06:00 to 22:00, MT the rest, weekends and holidays included (annex 2 of the act of 29 September 2025, in its text for 2027). The average energy price in Q1 2026 was 0.1175 EUR per kWh without VAT.
- **Prices with fixed hours:** no special tariff for electric cars turned up. The one supplier price with fixed hours is GEN-I Aktivni since 6 February 2026: without VAT 0.17990 EUR from 18:00 to 21:00, 0.11790 from 21:00 to 09:00 and 16:00 to 18:00, and from 09:00 to 16:00 0.03490 (March to October) or 0.08990 (winter) ([GEN-I](https://www.gen-i.si/dom/elektricna-energija/ceniki-in-akcije/aktivni-cenik-elektrike-za-dom/)).
- **State support:** none on the energy price. From 10 April to 10 October 2026 suppliers needed the government's consent to raise retail prices for homes ([Uradni list 227/2026](https://pisrs.si/api/uradni-list/objava/u20260227.pdf)), which caps nothing. The temporary cuts were per kW: the OVE and SPTE contribution at half for November 2025 to February 2026, and block 1's power price at 50% for November and December 2025 and January and February 2026.
- **The plan:** `si/omreznina`, the energy part of the network charge for user group 0, homes on a low-voltage feeder: one national price list, the same in every distribution area. Its prices match the AGEN-RS act of 15 December 2025 ([Uradni list 105/2025](https://www.uradni-list.si/glasilo-uradni-list-rs/vsebina/2025-01-3592/akt-o-dolocitvi-tarifnih-postavk-za-omreznine-elektrooperaterjev)), valid from 1 March to 31 December 2026: transmission and distribution together, without VAT, 0.02217, 0.01998, 0.01717, 0.01805 and 0.01299 EUR per kWh for blocks 1 to 5, and with 22% VAT the 0.02705, 0.02438, 0.02095, 0.02202 and 0.01585 in the plan. Its hours match [AGEN-RS's table for 2026](https://www.uro.si/prenova-omre%C5%BEnine/%C4%8Dasovni-bloki/2026). The blocks treat Saturdays, Sundays and work-free holidays alike.
- **The night:** the night block starts at 22:00 on every kind of day: block 3 on workdays from November to February, 4 on workdays from March to October and on weekends and holidays from November to February, 5 on weekends and holidays from March to October. On workdays from March to October block 3 (06:00 - 07:00, 14:00 - 16:00 and 20:00 - 22:00) costs 0.00107 EUR less per kWh than the night's block 4, but the night has almost no fee per kW.
- **The plan's earlier prices:** from 1 January 2025 to 28 February 2026 the energy part was flatter: 0.01998, 0.01833, 0.01809, 0.01855 and 0.01873 EUR per kWh without VAT, 0.02438, 0.02236, 0.02207, 0.02263 and 0.02285 with it ([Uradni list 97/2024](https://www.uradni-list.si/_pdf/2024/Ur/u2024097.pdf) and [uro.si](https://www.uro.si/prenova-omre%C5%BEnine/tarifne-postavke)). Worked out with the example's method, 2026's prices for October 2025 to February 2026 understate the board's cost for those 151 nights by about 1%: 197.91 EUR, against 199.72 with the old prices.
- **Market prices:** SMARD's series 260, a week a file from Monday 00:00 local time, in euros per MWh, like [the week of 29 December 2025](https://www.smard.de/app/chart_data/260/DE/260_DE_quarterhour_1766962800000.json): BSP's SIPXquarterly, the day-ahead price of each quarter-hour, which Petrol bills and the other suppliers average by the hour. All 35,040 quarter-hours from 1 October 2025 to 30 September 2026 are there, none missing. Its 2025 average, 104.64 EUR per MWh, and its 08:00 - 20:00 average, 98.94, are the figures AGEN-RS's report gives for BSP's day-ahead prices in 2025. Nord Pool has no area for Slovenia, and BSP's own data is paid after 60 days.

## Changes ahead

- **By the end of 2026:** the decree under ZSROVE-1 that makes the OVE and SPTE contribution a charge per kWh, about 0.00845 EUR without VAT by the estimate behind the law.
- **1 January 2027:** the network charge's blocks change their hours, with new prices: the plan needs both.
- **VAT, date unknown:** under ZIURS, passed 11 May 2026, the government may set 9.5% again on electricity by decree for up to nine months ([IUS-INFO](https://www.iusinfo.si/medijsko-sredisce/kolumne/ziurs-katere-novosti-prinasa-novi-interventni-zakon-323082)), but the law waits on a referendum and wasn't in force on 11 October 2026.

## Open questions

- **SIPXhourly's weights:** BSP shows its formula as a picture; a plain average is assumed.
- **The markups' sources:** ZPS's table is the only source for ECE's, Elektro energija's, Energija plus's and SunContract's markups, and for SunContract's zero floor.
- **The markups before March 2026:** ZPS's tables of earlier months weren't reachable, while GEN-I's and SunContract's lists date from 2024.
- **Billing by the hour:** whether Elektro energija and Energija plus bill by the hour, as GEN-I does.
- **E 3:** whether it has a dynamic offer: none was found.
- **Summer time:** whether the network blocks' hours move with it: the acts give clock hours only, as the plan does.
- **The offers' caps:** GEN-I's and Elektro energija's, 220 EUR per MWh, and Bisol Dinamični+'s, 197, which the example doesn't model and which would lower evening costs: 4.65% of the year's quarter-hours were above 220.
- **Dynamic homes in 2026:** not counted yet; AGEN-RS's next report would give it.

## Notes

- **Easter Monday:** the plan has 2027's date, so 6 April 2026 isn't in it: one night of the example's year.
- **Prices as pictures:** the page in the plan's first line shows its prices as pictures; the same numbers are in Uradni list 105/2025.
- **AGEN-RS's table for 2026:** its one-column picture labels 14:00 - 16:00 on a lower-season weekend or holiday as 5, against 4 in its other two tables and in the plan.
- **7 August:** the law of 2 October 2026 that adds it as a holiday makes it one that isn't work-free ([Uradni list 1567/2026](https://pisrs.si/api/uradni-list/objava/u20261567.pdf)), so the plan's dates stand.
- **Energy-Charts:** its API (bzn=SI) serves the same numbers as SMARD, but it republishes SMARD's, so it checks nothing.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [AGEN-RS's act of 15 December 2025, Uradni list 105/2025](https://www.uradni-list.si/glasilo-uradni-list-rs/vsebina/2025-01-3592/akt-o-dolocitvi-tarifnih-postavk-za-omreznine-elektrooperaterjev) | the plan's prices, without VAT | 1 January 2027 |
| [AGEN-RS's time blocks for 2026](https://www.uro.si/prenova-omre%C5%BEnine/%C4%8Dasovni-bloki/2026) | the plan's hours | 1 January 2027 |
| [ZPS's market review](https://www.zps.si/aktualne-vsebine/trzni-pregledi/pregled-cen-elektricne-energije-in-zemeljskega-plina-152) | the dynamic offers' markups, the price lists | every three months (March, June and September 2026) |
| [Bisol's dynamic offer](https://www.bisol-energija.si/dinamicni-gospodinjstva) | the example's margin and monthly fee | any time |
| [AGEN-RS's report for the year](https://www.agen-rs.si/-/porocilo-o-stanju-na-podrocju-energetike-v-sloveniji-v-letu-2025) | dynamic households and their suppliers, the year's market prices | each year, around July |
| [The ministry's prices for the quarter](https://www.energetika-portal.si/nc/novica/n/cene-elektricne-energije-v-prvem-cetrtletju-2026/) | the average energy price, the excise | each quarter |
| [Energy portal on the OVE and SPTE contribution](https://www.energetika-portal.si/podrocja/energetika/prispevek-za-obnovljive-vire/) | the contribution per kW, and per kWh from the new decree | by the end of 2026 |
| [SMARD's series 260](https://www.smard.de/app/chart_data/260/DE/260_DE_quarterhour_1766962800000.json) | the market prices, a week a file | daily |
