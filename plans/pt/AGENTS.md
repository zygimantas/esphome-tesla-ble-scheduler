# plans/pt/

Portugal's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a dynamic offer billing OMIE's Portuguese price by the quarter-hour, the board's charging cost 568.07 EUR over the year from 1 October 2025: 37% less than charging at once (908.72 EUR), 16% less than the car's own schedule from 22:00 (680.15 EUR) and 3% less than its best start, 03:00 (583.48 EUR). Confidence in the README is Low: under 1% of homes pay the market price.

## Decisions

- **Contract:** a dynamic offer billing OMIE's Portuguese price by the quarter-hour, as Galp's, Repsol's and others' do.
- **Plan:** `pt/bi-horaria-diario`. The car's own schedule starts at 22:00, when its off-peak does.
- **Billing:** each quarter-hour at its own price.
- **Margin:** 0.03444 EUR per kWh with VAT, about the middle of five quarter-hour offers, REN's regulation charges included.
- **The same every hour:** 0.00377 EUR per kWh with VAT: the electricity tax and the social tariff's financing.
- **Factor:** 1.15 on the market price, for the grid losses the offers bill.
- **VAT:** 23%, the rate on the extra kWh a car adds, not the 6% on a home's first 200 kWh: where VAT is split, the plans and the example take the car's rate.
- **Left out:** the power term, the audiovisual contribution and the DGEG fee, as they don't depend on when the car charges.
- **Market prices:** OMIE's files, Portugal's column. Lisbon is an hour behind CET.

## Findings

- **Who pays the market price:** in ACER and CEER's 2026 retail report ([Figures 18 and 15](https://www.acer.europa.eu/sites/default/files/documents/Publications/ACER-CEER-2026-Retail-Monitoring.pdf), 2025 data from ERSE), 92% of Portugal's household contracts have a single fixed price, 8% are time of use or indexed, and dynamic ones are under 1%, though 99% of homes have a smart meter. Suppliers with over 200,000 customers must offer dynamic prices to homes with a smart meter, and those with over 50,000 an indexed price ([ERSE's Regulamento n.º 827/2023, article 15](https://www.erse.pt/media/cw5fdm3i/regulamento-827-2023_consolidado.pdf)). G9 moved all its customers to its dynamic offer in July 2026 (DECO PROteste, 24 July 2026).
- **The offers:** these bill each quarter-hour of the meter's readings (E-REDES's load diagram): Galp Plano Dinâmico, Repsol Tarifa Leve Sem Mais, Coopérnico Base 2.0, G9 Smart Dynamic SPOT 8! and MEO Energia Tarifa Dinâmica, whose sheets are under Margins, and, as tiagofelicia.pt's simulator lists them, EDP Eletricidade Indexada Horária, Alfa Energia ALFA POWER INDEX BTN and EZU Indexada, with Iberdrola and Plenitude in Simples only. Galp's Plano Flexível spreads the month's use by ERSE's standard profile, and the simulator has offers from EDP, Endesa, Goldenergy, Ibelectra, Luzboa and LUZiGÁS on monthly averages: they don't reward the cheap quarter-hours.
- **The price they bill:** OMIE's day-ahead price for Portugal (PMSp, "OMIE PT"), by the quarter-hour in 2026 (Repsol's sheet of 1 September 2026, Galp's and MEO's pages; Coopérnico still says each hour), times ERSE's loss factor for low voltage. MEO sets a negative energy price to 0 and G9 a negative OMIE price. Prices at or below 0 came in 2.1% of the quarter-hours from 18:00 to 07:00.
- **Grid losses:** ERSE's loss factor for low voltage, which E-REDES sets for each quarter-hour, averaged 1.13 from 00:00 to 07:00, 1.17 from 18:00 to 19:00 and from 22:00 to 23:00, and 1.16 over all hours from October 2025 to September 2026 ([tiagofelicia.pt's data](https://dados.tiagofelicia.pt/data/simuladores/simulador-tarifarios-eletricidade/csv/OMIE_PERDAS_CICLOS_2026.csv), and its _2025.csv). ERSE's 2026 averages are 1.1409 off-peak and 1.1689 the rest of the day (Diretiva n.º 1/2026, article 49). Worked out from these: with one factor, 1.15, the market price's multiplier with VAT is 1.15 x 1.23 = 1.4145.
- **VAT:** 23% on the kWh a car adds, from October 2025 to September 2026. 6% applies only to a home's first 200 kWh in 30 days (300 for large families), and only up to 6.9 kVA (Lei n.º 38/2024, de 7 de agosto, since 1 January 2025: [ERSE](https://www.erse.pt/media/tcsfm4n2/ersexplica_iva-fatura_2025.pdf)). Charging at 11 kW needs more than 6.9 kVA, so such a home pays 23% on every kWh. VAT applies to the electricity tax and the social tariff's financing too.
- **Per kWh, the same every hour:** 0.00377 EUR with VAT, the same all year:
  - the electricity tax (IEC), 0.00100 EUR without VAT, 0.00123 with it (Coopérnico's sheet under Margins, [Goldenergy](https://goldenergy.pt/glossario/imposto-especial-sobre-consumo-eletricidade-iec/)); homes on the social tariff don't pay it;
  - the social tariff's financing, 0.00207 EUR (2.0666 EUR per MWh) without VAT, 0.00254 with it, which REN bills suppliers on the energy they sell in 2026 ([ERSE's Diretiva n.º 12-A/2025](https://files.diariodarepublica.pt/2s/2025/12/250000005/0012900145.pdf)). Repsol and Coopérnico bill it per kWh; Galp and MEO may have it in their price.
- **Other charges:** the power term, in EUR a day, with the network's; the audiovisual contribution, 2.85 EUR a month with 6% VAT; and the DGEG fee, 0.07 EUR a month. No price cap or state support that changes the price by the hour was found. The social tariff, 33.8% off in 2026 and without the IEC, needs at most 6.9 kVA.
- **Margins:** 0.03444 EUR with VAT (0.02800 without) is about the middle of five quarter-hour offers. About 0.017 of it (0.021 with VAT) is REN's regulation charges (encargos de regulação), which every offer passes on in one way or another; the rest is the supplier's own. Worked out from each offer's formula, its addition per kWh at night, without and with VAT, at a loss factor of 1.13, REN's charges at 0.015 and OMIE's night average, 0.0894, for the shares of the market price:
  - Galp Plano Dinâmico, Ci 0.0191 times losses: 0.02158, 0.02655. Ci covers system services, imbalances, guarantees of origin and the margin, and can change each January, April, July and October ([Galp](https://www.galp.com/pt/casa/planos-eletricidade-e-gas/flexivel), read 11 October 2026).
  - Repsol Tarifa Leve Sem Mais, QTarifa 0.02109 plus 3% of the market price after losses (FA 1.03): 0.02412, 0.02967 ([sheet v20260901](https://www.repsol.pt/content/dam/repsol-portugal/eletricidade-e-g%C3%A1s/tarifas-particulares/repsol-tarifas-eletricidade-leve-sem-mais.pdf)). QTarifa has been 0.02109 since June 2025, 0.01479 before.
  - Coopérnico Base 2.0, (k 0.009 + CR + CS) times losses, CR being REN's charges as a three-month average (0.01963 in July 2026) and CS 0.001: 0.02825, 0.03475 ([July 2026](https://prd.coopernico.org/storage/files/tarifario.pdf)). Members buy three shares, 60 EUR once.
  - G9 Smart Dynamic SPOT 8!, K2 0.0185 (REN's charges, an indicative average) + K3 0.008 + 2% of the market price after losses (K1 1.02): 0.02852, 0.03508 ([prices for 13 June to 13 July 2026](https://g9.pt/energia/)).
  - MEO Energia Tarifa Dinâmica, K 0.015 + CG times losses, CG being the system's and MEO's own imbalance costs each quarter-hour, not published, taken as 0.015: 0.03195, 0.03930 ([sheet of 17 September 2026](https://conteudos.meo.pt/meoenergia/Documentos/ficha-informacao-padronizada-meo-energia.pdf)). MEO bills an information service monthly after its campaign.
  - Iberdrola's indexed Simples, not among the five, shows Q 0.0250 and an mFRR band of 0.0019 (iberdrola.pt).
- **REN's regulation charges:** 15.95 EUR per MWh on average over all hours from October 2025 to September 2026, 14.26 from 00:00 to 07:00, 12.64 from October to December 2025 and 17.06 since (tiagofelicia.pt's data).
- **The plan:** `pt/bi-horaria-diario`. Bi-horária makes night charging pay 0.01943 EUR to the network instead of Simples' 0.07466, with VAT, and its daily cycle has off-peak (vazio) from 22:00 to 08:00 every day of the year (Diretiva n.º 1/2026, article 45), where the weekly cycle's starts at 00:00 on weekdays. The plan's prices match 2026's access tariffs, 0.0835 and 0.0158 EUR without VAT ([Diretiva n.º 1/2026](https://files.diariodarepublica.pt/2s/2026/01/004000000/0018600288.pdf), Diário da República of 7 January 2026, page 16, and ERSE's s_tarifas_net.xlsx). They held all year: Diretiva ERSE n.º 4/2026 changed only the regulated energy price from 1 October 2026, and the access tariffs in its s_tarifas_net_outubro-2026.xlsx are the same. Until 31 December 2025 they were 0.0830 and 0.0149, 0.10209 and 0.01833 with VAT ([Diretiva n.º 2/2025](https://www.erse.pt/media/oiwgcxk5/diretiva-2_2025-tarifas-2025.pdf)). Holidays are ordinary days in low voltage: only MAT, AT and MT on the weekly cycle treat them as Sundays (article 45). ERSE expects 362,603 Bi-horária customers on the free market in 2026 against 5,224,196 on Simples ([ERSE](https://www.erse.pt/media/5zclblrg/caracteriza%C3%A7%C3%A3o-procura-se-2026.pdf)).
- **Market prices:** OMIE's daily files, as the board downloads them, without an account or key, each line year;month;day;quarter-hour;Portugal;Spain in EUR per MWh for a CET day, like https://www.omie.es/es/file-download?parents=marginalpdbc&filename=marginalpdbc_20260101.1. Lisbon is an hour behind (standard offset 0), so a night from 18:00 to 07:00 is 19:00 to 08:00 CET, across two files. Portugal's column ("PT"), which the offers bill, differed from Spain's in 10.4% of the quarter-hours from October 2025 to September 2026, though both averaged about 73.3 EUR per MWh. Every day from 1 October 2025 to 1 October 2026 downloads; before 1 October 2025 the files are hourly. tiagofelicia.pt's OMIE prices match OMIE's files, and its loss factors ERSE's averages.

## Changes ahead

- **1 January each year:** ERSE's access tariffs, which the plans hold.
- **Each January, April, July and October:** Galp's Ci can change.
- **Each March and October:** the plans' dated exceptions for the last week of March and of October, to redo every year. In 2027 summer time ends on Sunday 31 October.
- **1 July to 31 December 2027:** each home moves to ERSE's new hours on its own day, which its grid operator tells it. The plans keep today's hours until then.

## Open questions

- **EDP's formula:** tiagofelicia.pt's simulator has OMIE times losses times 1.08, plus 0.0185. edp.pt would confirm it, but it turns automated reads away.
- **REN's charges and the loss factors by quarter-hour:** they come from tiagofelicia.pt's data, built from REN's and E-REDES's, whose own files weren't reached.
- **MEO's imbalance costs (CG):** not published; the margins take 0.015.
- **Billing before 2026:** whether suppliers billed quarter-hours or hourly averages from October to December 2025. Repsol's 2025 sheets used hourly prices.
- **The social tariff's financing in 2025:** per kWh, not found.
- **The IEC's rate:** not confirmed at the Autoridade Tributária; two suppliers give 0.001.
- **Car owners' plans:** no figure says how many take Bi-horária, or with the daily or the weekly cycle.
- **EV offers at a fixed price:** offers with cheap nights at a fixed price weren't checked.

## Notes

- **Summer and winter hours:** Portugal's summer and winter periods switch at the clock change, which a month calendar can't hold: the last week of October and of March are dated exceptions in the plans, to redo every year.
- **OMIE's corrections:** OMIE keeps only a day's latest file: .3 for 30 October 2025 and .2 for 27 November 2025.
- **Not done yet:** tri-horária above 20.7 kVA, which has other prices.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [ERSE's tariffs and prices](https://www.erse.pt/atividade/regulacao/tarifas-e-precos-eletricidade/) | the access tariffs, in s_tarifas_net.xlsx | each 1 January |
| [Diretiva n.º 1/2026](https://files.diariodarepublica.pt/2s/2026/01/004000000/0018600288.pdf) | 2026's access tariffs (page 16), the cycles' hours (article 45) and the loss factors (article 49) | each 1 January |
| [ERSE's Diretiva n.º 12-A/2025](https://files.diariodarepublica.pt/2s/2025/12/250000005/0012900145.pdf) | the social tariff's financing | each year |
| [ACER and CEER's retail report](https://www.acer.europa.eu/sites/default/files/documents/Publications/ACER-CEER-2026-Retail-Monitoring.pdf) | household contracts by kind of price, smart meters | each year |
| [ERSE's demand for 2026](https://www.erse.pt/media/5zclblrg/caracteriza%C3%A7%C3%A3o-procura-se-2026.pdf) | customers by tariff option | each year |
| The offers: [Galp](https://www.galp.com/pt/casa/planos-eletricidade-e-gas/flexivel), [Repsol](https://www.repsol.pt/content/dam/repsol-portugal/eletricidade-e-g%C3%A1s/tarifas-particulares/repsol-tarifas-eletricidade-leve-sem-mais.pdf), [Coopérnico](https://prd.coopernico.org/storage/files/tarifario.pdf), [G9](https://g9.pt/energia/), [MEO](https://conteudos.meo.pt/meoenergia/Documentos/ficha-informacao-padronizada-meo-energia.pdf) | the margins | any time; Galp's Ci each January, April, July and October |
| [tiagofelicia.pt's data](https://dados.tiagofelicia.pt/data/simuladores/simulador-tarifarios-eletricidade/csv/OMIE_PERDAS_CICLOS_2026.csv) | REN's regulation charges and the loss factors by quarter-hour | a file a year |
| [OMIE's daily files](https://www.omie.es/es/file-download?parents=marginalpdbc&filename=marginalpdbc_20260101.1) | the market prices, a day a file | daily |
