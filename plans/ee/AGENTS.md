# plans/ee/

Estonia's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On an exchange-price package, the board's charging cost 433.00 EUR over the year from 1 October 2025: 43% less than charging at once (766.27 EUR), 33% less than the car's own schedule from 22:00 (644.65 EUR) and 5% less than its best start, 03:00 (457.89 EUR). Confidence in the README is High: exchange-price packages are common.

## Decisions

- **Contract:** an exchange-price package (börsipakett) billing Nord Pool's EE price: the price the board downloads.
- **Plan:** `ee/elektrilevi-vork-2`. The car's own schedule starts at 22:00, when its night fee does.
- **Billing:** each hour at the average of its four quarter-hours, as most meters still record hours.
- **Margin:** 0.005 EUR per kWh with VAT: Enefit's, the biggest supplier's, and the middle of five big suppliers' offers.
- **The same every hour:** 0.02704 EUR per kWh with VAT: the renewable energy fee, the electricity excise, the security of supply fee and the balancing capacity cost.
- **Left out:** Elektrilevi's and the suppliers' monthly fees, as they don't depend on when the car charges.
- **Market prices:** Elering's dashboard, field `data.ee`, which republishes Nord Pool's prices without an account.

## Findings

- **Who pays the market price:** exchange-price packages are common. Every big supplier sells an open-ended one (börsipakett), which can be left at 14 days' notice. In June 2026, 60% of Alexela's customers had chosen fixed prices, a share that grew during the year ([ERR, 7 August 2026](https://www.err.ee/1610104690/ulevaade-kas-valida-talveks-fikseeritud-elektrihind-voi-borsipakett)). Another 61,191 of 659,290 households (9%) were on universal service in 2025, which costs the month's weighted average ([Konkurentsiamet's report for 2025](https://www.konkurentsiamet.ee/sites/default/files/documents/2026-09/Elektri-%20ja%20gaasituru%20aruanne%202025_viimane.pdf), tables 17 and 31).
- **Meters and billing:** since 1 October 2025 the packages bill Nord Pool's EE day-ahead price for each quarter-hour: Enefit, Elektrum and Alexela say so on their pages, and Alexela's terms for Vali ise define it. Most homes still have hourly meters, though: 110,000 of Elektrilevi's 700,000 meters measured quarter-hours in January 2026 ([ERR, 22 January 2026](https://www.err.ee/1609918682/elektrilevil-votab-uute-arvestite-paigaldamine-arvatust-kauem-aega)). For the rest, Elektrilevi splits each hour's use equally over its four quarter-hours ([Elektrilevi, 6 April 2026](https://elektrilevi.ee/et/blogi/mida-pean-teadma-15-minuti-mootmisest); Elenger's FAQ says the same), so they pay the hour's average of the four prices. The board saves a home with an hourly meter slightly less than one whose meter records quarter-hours.
- **VAT:** 24% on every line of both bills, all year: Estonia's standard rate since 1 July 2025, 22% before, and electricity has no reduced rate ([EMTA](https://emta.ee/ariklient/maksud-ja-tasumine/kaibemaks/kaibemaksumaarad-ja-maksuvaba-kaive)). Elektrilevi's price list and Konkurentsiamet's report for 2025 apply it too. Nothing changed from October 2025 to September 2026.
- **Per kWh, the same every hour:** 0.02181 EUR without VAT, 0.02704 with it, from four charges in 2026, and 0.01302 with VAT from October to December 2025, before the last two began:
  - the renewable energy fee (taastuvenergia tasu), 0.00840 EUR without VAT, 0.01042 with it, on Elektrilevi's bill, the same as in 2025 ([Elektrilevi's price list from 1 June 2026](https://public-docs.elektrilevi.ee/2/7/vorguteenuse_kehtiv_hinnakiri_71ed51b53d.pdf), page 7; [its news of 3 December 2025](https://elektrilevi.ee/en/uudised/ulevaade-2026-aasta-vorguteenuse-arvest-lisandub-varustuskindluse-tasu-taastuvenergia-tasu-ei-muutu));
  - the electricity excise (elektriaktsiis), 0.00210 EUR without VAT, 0.00260 with it, since 1 May 2025, as the rise planned for 1 May 2026 was cancelled (the same price list; [Elektrilevi](https://elektrilevi.ee/et/blogi/millest-koosneb-elektriarve); [KPMG](https://kpmg.com/ee/et/insights/2026/04/infokuller.html));
  - the security of supply fee (varustuskindluse tasu), 0.00758 EUR without VAT, 0.00940 with it, on Elektrilevi's bill from 1 January 2026 ([Elering](https://www.elering.ee/varustuskindluse-tasu); the same price list);
  - the balancing capacity cost (tasakaalustamisvõimsuse kulu), 0.00373 EUR without VAT, 0.00463 with it, on the supplier's bill from 1 January 2026, the same all year ([Elering](https://www.elering.ee/tasakaalustamisvoimsuse-kulu); [ERR, 31 October 2025](https://www.err.ee/1609844289/uus-tasu-elektriarvel-saab-olema-3-73-eurot-megavatt-tunni-kohta); Sunly's page lists it too).
- **Margins:** five big suppliers' open-ended exchange-price packages, checked on 11 October 2026, in EUR per kWh with VAT (without VAT in brackets):
  - Enefit's Muutuv: 0.00500 (0.00403) and 2.05 EUR a month ([Enefit](https://www.enefit.ee/et/era/elekter/elektrileping-ja-paketid)).
  - Alexela's Börsihinnaga elekter: 0.00470 (0.00379) and 2.02 EUR a month ([Alexela](https://www.alexela.ee/et/elekter)).
  - Elektrum's Börsi Klõps: 0.00533 (0.00430) and 1.91 EUR a month, from [elektrihind.ee](https://elektrihind.ee/paketid/), the comparison site Konkurentsiamet certifies.
  - Elenger's (Eesti Gaas) Muutuv: 0.00670 (0.00540) and no monthly fee ([Elenger](https://elenger.ee/kodukliendile/elekter/)).
  - Sunly's: no margin, but 3 to 9 EUR a month by the last year's use, 6 EUR for 6,001 to 12,000 kWh ([Sunly](https://sunly.ee/elekter/era)).
  - Konkurentsiamet's report for 2025 took a margin of 0.00500 EUR without VAT (0.00620 with) for its example bills.
- **The plan:** Elektrilevi runs the grid in 93% of Estonia. For a house with a 25 A main fuse, common where a car charges at 11 kW, its [package page](https://elektrilevi.ee/et/vorguleping/vorgupaketid/eramu) suggests Võrk 2 from 1,400 to 9,300 kWh a year and Võrk 4 above, assuming equal day and night use (6,300 kWh with 16 A). Households used 3,030 kWh on average in 2025 (Konkurentsiamet). Worked out from these, a house that doesn't heat with electricity likely stays under 9,300 kWh with the car's 3,300, and the car's use, all at night, pushes the point where Võrk 4 pays past 10,000 kWh, as Võrk 4 saves less by night than by day. Above that, Võrk 4 (`ee/elektrilevi-vork-4`, with the same hours) is the likely one.
- **The plans' fees:** Võrk 2's night fee, 0.04352 EUR per kWh with VAT against 0.07527 by day, starts at 22:00 on workdays and applies all day at weekends and on public holidays. The plan's prices match 2026's: the price list from 1 June 2026 (Konkurentsiamet's decision 7-3/2026-028 of 2 March 2026) has the same fees per kWh as [the one for 1 August 2025 to 31 May 2026](https://public-docs.elektrilevi.ee/2/7/vorguteenuse_hinnakiri_01_08_2025_kuni_31_05_2026_dd157d3c88.pdf); only the monthly fees rose on 1 June 2026. Elektrilevi's monthly fee goes by the main fuse; homes up to 63 A pay no fee per kW.
- **State support:** no state support, price cap or special tariff for charging at home was found for the year. Enefit's Hinnalaega börsipakett, at the market price between a floor of 0.05990 and a cap of 0.21000 EUR per kWh with VAT for 12 months, is a supplier's product.
- **Market prices:** Elering's dashboard API republishes Nord Pool's day-ahead prices without an account or key, a month a request, in EUR per MWh without VAT. Since 1 October 2025 it gives quarter-hours, Estonia's under `data.ee`, with UTC timestamps in seconds and the slot at `end` included. All twelve months from October 2025 to September 2026 are complete, and a day checked against Nord Pool's own EE prices, which the packages bill, matched exactly. The same monthly files hold the other Baltic areas' and Finland's prices. Tallinn keeps Vilnius's clock (UTC+2, +3 in summer).

## Changes ahead

- **1 January each year:** Elering's renewable energy and security of supply fees, which it sets by 1 December: 2027's by 1 December 2026.
- **Elektrilevi's prices:** a new price list comes when Konkurentsiamet approves one; the last two started on 1 August 2025 and 1 June 2026.
- **1 January 2027:** the balancing capacity cost falls to 0.00297 EUR per kWh without VAT ([ERR, 26 June 2026](https://www.err.ee/1610063971/elering-vahendab-jargmisest-aastast-tasakaalustamistasu)).
- **Quarter-hour meters:** Elektrilevi says all meters measure quarter-hours by 2031, while ERR reported in January 2026 that the government moved the deadline for new meters to 2035.

## Open questions

- **How many homes are on exchange-price packages:** no national share was found; Alexela's is one supplier's.
- **Which package homes are on:** Elektrilevi publishes no split of its customers by package, so the plan rests on its own advice.
- **Margins earlier in the year:** the ones above are October 2026's; earlier in the year they may have differed, though Konkurentsiamet's monthly reviews show the cheapest exchange package at 0.00670 EUR per kWh with VAT on 21 May, 0.00600 on 29 July and 0.00400 on 7 August 2026.

## Notes

- **Good Friday:** the plans carry 2027's Good Friday (26 March), so the example takes Thursday 26 March 2026 as a holiday and Friday 3 April, 2026's Good Friday, as a workday.
- **Elektrum's prices:** shown only after login, so its margin comes from elektrihind.ee.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Elektrilevi's price lists](https://elektrilevi.ee/et/hinnakirjad/vorguteenuste-hinnakiri), [the one from 1 June 2026](https://public-docs.elektrilevi.ee/2/7/vorguteenuse_kehtiv_hinnakiri_71ed51b53d.pdf) | the plans' fees, and on page 7 the renewable energy fee, the excise and the security of supply fee | with each new price list; Elering's fees each 1 January |
| [Elering on the security of supply fee](https://www.elering.ee/varustuskindluse-tasu) | the security of supply fee | each 1 January |
| [Elering on the balancing capacity cost](https://www.elering.ee/tasakaalustamisvoimsuse-kulu) | the balancing capacity cost | next on 1 January 2027 |
| [elektrihind.ee](https://elektrihind.ee/paketid/) | the suppliers' packages, margins and monthly fees | any time |
| [Konkurentsiamet's report for 2025](https://www.konkurentsiamet.ee/sites/default/files/documents/2026-09/Elektri-%20ja%20gaasituru%20aruanne%202025_viimane.pdf) | households, universal service and average use | each year |
| [Elering's dashboard](https://dashboard.elering.ee/api/nps/price?start=2026-01-01T00:00:00.000Z&end=2026-02-01T00:00:00.000Z) | the market prices, a month a request | daily |
