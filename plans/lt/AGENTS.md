# plans/lt/

Lithuania's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On an exchange-price plan, the board's charging cost 535.54 EUR over the year from 1 October 2025: 46% less than charging at once (1,000.77 EUR), 32% less than the car's own schedule from 22:00 (785.65 EUR) and 8% less than its best start, 03:45 (581.78 EUR). Confidence in the README is Medium: only a few percent of homes pay each quarter-hour's price, most a fixed one.

## Decisions

- **Contract:** an exchange-price plan billing Nord Pool's LT price for each quarter-hour of the home's own use, like Ignitis Išmanus 15: the price the board downloads.
- **Plan:** `lt/eso-standartinis-4-zones`. The car's own schedule starts at 22:00, when its night fee does.
- **Billing:** each quarter-hour at its own price.
- **Margin:** 0.025 EUR per kWh with VAT, about the middle of the three suppliers' offers.
- **The same every hour:** -0.00026 EUR per kWh with VAT, the public service obligations and the household extra component.
- **Left out:** the suppliers' monthly fees and ESO's monthly fees, as they don't depend on when the car charges.
- **Market prices:** Elering's, as Nord Pool serves only its last two months without an account.

## Findings

- **Who pays the market price:** about 200,895 household contracts were dynamic or linked to the exchange at the end of 2025, 2.2% fewer than a year before ([VERT's 2025 report to the European Commission](https://www.ceer.eu/wp-content/uploads/2026/09/NERC-Annual-Report-2025-EN.pdf)), 11.6% of the 1,727,209 household supply points of December 2025 ([VERT](https://www.vert.lt/elektra/SiteAssets/2026-02/Tiekimo%20rinka_apzvalga_sausis_2026.pdf)). That count includes monthly plans, which price a month's average whatever the hour of use. Ignitis said on 8 April 2025 that about 14.5% of its customers were on its monthly plan, about 3% on its hourly plan, now Išmanus 15, and more than 80% on fixed prices ([Ignitis](https://ignitis.lt/naujienos/ignitis-pataria-kaip-nesukti-galvos-del-elektros)). Enefit said on 19 August 2025 that about eight in ten of its customers were on fixed prices ([15min](https://www.15min.lt/verslas/naujiena/energetika/birzos-planas-ar-fiksuota-kaina-ekspertas-pasake-kuris-pasirinkimas-siandien-gali-buti-naudingesnis-664-2508162)), and Elektrum on 6 November 2025 that 7% of its customers, mostly businesses, were on exchange plans ([Elektrum](https://www.elektrum.lt/lt/namams/naujienos/naujienos/fiksuota-ar-birzos-kaina-ka-verta-zinoti-pries-pasirenkant)). Worked out from these, with the dates mixed: Ignitis' 3% is about 27,000 homes, and VERT's count less Ignitis' monthly plan (about 129,000) leaves at most about 72,000, so 1.5% to 4% of household supply points pay each quarter-hour's or hour's price.
- **The public supplier:** in March 2026 Ignitis supplied 889,017 of the 1,731,071 household supply points as an independent supplier and 441,016 (25.5%) as the public supplier, whose price VERT caps every half year, and ESO 39,767 (2.3%) as the guaranteed supplier, at the month's average price plus 15% ([VERT's report for the month](https://www.vert.lt/elektra/SiteAssets/2026-04/Tiekimo%20rinka_apzvalga_kovas_2026.pdf), [VERT on the cap](https://vert.lt/Puslapiai/naujienos/2025-metai/2025-05-21/nustatyta-UAB-Ignitis-visuomenines-elektros-kainos-virsutine-riba-2025-m-II-pusmeciui.aspx)).
- **Meters and billing:** billing each quarter-hour of a home's own use takes a smart meter. Ignitis sells Išmanus 15 only to homes with one, weights each 15-minute price by the home's use in it, and gives other homes its monthly plan; where ESO sends no quarter-hour data for a month, the monthly average applies too ([Ignitis](https://ignitis.lt/lt/duk/nepriklausomas-elektros-tiekimas)). Elektrum bills a home without one at the previous month's Nord Pool average, weighted by ESO's 15-minute consumption coefficients ([Elektrum](https://www.elektrum.lt/lt/namams/duk/elektros-energija-privatiems-vartotojams/elektros-energijos-sutartis-privatiems-klientams/kaip-apskaiciuojama-kintamos-elektros-tarifo-kaina)). ESO had installed 1.3 million smart meters by 19 December 2025, starting with customers using more than 1,000 kWh a year: about 66% of its customers, covering more than 90% of the country's consumption ([Ministry of Energy](https://enmin.lrv.lt/lt/naujienos/eso-baigia-pirmaji-ismaniuju-skaitikliu-diegimo-etapa-13-mln-skaitikliu-padengia-daugiau-nei-90-proc-salies-suvartojimo-rIZr/)).
- **VAT:** 21% on everything.
- **Per kWh, the same every hour:** -0.00026 EUR with VAT, unchanged from January to September 2026:
  - the public service obligations (VIAP), -0.00044 EUR without VAT, -0.00053 with it ([VERT O3E-1591](https://www.e-tar.lt/portal/lt/legalAct/1d881776b65c11f092fda1fd0c194cc5)), billed on top of ESO's fee;
  - the household extra component of the distribution service, 0.00022 EUR without VAT, 0.00027 with it (VERT O3E-1650, as [ESO](https://www.eso.lt/namams/elektra/tarifu-planai-kainos-atsiskaitymas/tarifu-planai-ir-kainos-2026-metais/4801) cites it);
  - no excise: households are exempt (Law on Excise Duties, art. 48(1)(3)).
- **Margins:** Elektrum Pažangus 0.02456, Enefit Laisvas 0.024 and Ignitis Išmanus 15 0.02981 EUR per kWh with VAT, each with 2 EUR a month ([VERT's comparison](https://skaiciuokle.vert.lt/), updated 7 October 2026).
- **The plans' fees:** ESO's fee includes Litgrid's transmission. VERT approves ESO's prices for each year, from 1 January, and 2026's held from January to September (ESO's monthly price history).
- **Market prices:** Elering's dashboard republishes Nord Pool's Baltic prices, a month a file, field `data.lt`. It matched Nord Pool's LT prices on all 4,992 quarter-hours from 10 August to 30 September 2026.

## Changes ahead

- **1 January each year:** ESO's new prices, which VERT approves in December; the public supplier's capped price changes every half year.
- **Smart meters:** ESO replaces the rest of the old meters as they expire, about 97% by 2030 and the last in 2034 (VERT gives 2037); the research didn't record where that's from.

## Open questions

- **How many homes pay each quarter-hour's price:** no source counts them. Ignitis' 3% dates from before its hourly plan became Išmanus 15 on 1 October 2025; its next figures, or VERT's next report, would settle it.
- **The count's change:** VERT's 2025 count adds "exchange-linked plan contracts" to the "dynamic price contracts" of its 2024 report ([2024 report](https://www.ceer.eu/wp-content/uploads/2025/10/NERC-Annual-Report-2024-EN.pdf)), so its 2.2% fall may not compare like with like.
- **Enefit's billing:** its page for Laisvas says in English that the price changes every 15 minutes, in Lithuanian every hour, and recommends a smart meter ([Enefit](https://www.enefit.lt/lt/privatiems/elektra/kintama-birzos-kaina)). Its terms for homes, behind a bot check, would say how it bills a home with or without one.
- **Smart meters in homes:** no share for 2026. ESO's 66% counts all its customers, VERT gave 53% of households in early 2025 ([2024 report](https://www.ceer.eu/wp-content/uploads/2025/10/NERC-Annual-Report-2024-EN.pdf)) and Elektrum more than 60% in November 2025 ([Elektrum](https://www.elektrum.lt/lt/namams/naujienos/naujienos/fiksuota-ar-birzos-kaina-ka-verta-zinoti-pries-pasirenkant)).
- **The public supplier:** the research found it open to homes using under 1,000 kWh a year and to vulnerable consumers until 2030, without a source recorded.
- **Margins earlier in the year:** the ones above are October 2026's comparison; earlier in 2026 they may have differed.

## Notes

- **To fix:** ESO's 2026 price rules (point 3.2) list the holidays with weekend zones without Easter Monday or 2 November, which the four-zone plans have as holidays.
- **Dynamic in name:** the Rules on Electricity Supply and Use, point 47(1) in a 2024 consolidated text, bar calling a contract dynamic where the home has no smart meter and pays a monthly weighted average. VERT's counts still mix the two, so say "each quarter-hour's price" rather than "dynamic".

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [ESO's plans and prices for 2026](https://www.eso.lt/namams/elektra/tarifu-planai-kainos-atsiskaitymas/tarifu-planai-ir-kainos-2026-metais/4801) | the plans' fees and hours, the household extra component | each 1 January |
| [VERT's comparison of offers](https://skaiciuokle.vert.lt/) | the suppliers' margins and monthly fees | any time |
| [VERT's monthly retail market review](https://www.vert.lt/elektra/SiteAssets/2026-04/Tiekimo%20rinka_apzvalga_kovas_2026.pdf) | supply points by supplier, the public and guaranteed suppliers' included | each month |
| [VERT's annual report to the European Commission](https://www.ceer.eu/wp-content/uploads/2026/09/NERC-Annual-Report-2025-EN.pdf) | the count of dynamic and exchange-linked contracts | each year, around September |
| [VERT O3E-1591](https://www.e-tar.lt/portal/lt/legalAct/1d881776b65c11f092fda1fd0c194cc5) | the public service obligations | each year |
| [Elering's dashboard](https://dashboard.elering.ee/api/nps/price?start=2026-01-01T00:00:00.000Z&end=2026-02-01T00:00:00.000Z) | the market prices, a month a request | daily |
