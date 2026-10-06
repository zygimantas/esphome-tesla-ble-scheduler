# plans/

- `name:` replaced the name in the first line (`# <name>, prices with VAT: <link>`) in October 2026, in every plan at once, by the owner's decision, rather than in a new folder: boards on 5.3.0 and before turn such a plan away and keep the one they have until they're updated.
- Only the `*.yaml` files are plans: README.md and this file never reach a board, and the unit tests read only the `.yaml` files, as the board does. Run test/scheduler_test.cpp after any change.
- What goes in a plan, as the owner settled it in October 2026: the operator's own fee per kWh for that plan, with VAT, summing the per-kWh parts it bills together (like Austria's usage and losses), and another network charge only if it too changes with the hour. Taxes, levies, a transmission operator's flat tariff on a line of its own, monthly fees and fees per kW stay out, and the country's README.md says which. Where VAT is split, it's the rate on the extra kWh a car adds: Portugal's 23%, not the 6% on a home's first 200 kWh. Norway's Arva has no VAT, as its whole area is exempt.
- Conventions: 5 decimals (older plans drop a trailing zero); `name:` at most about 40 characters, for the phone's list, as the first key, above `currency:`; the first line, `# Prices with VAT: <link>`, links the operator's prices, or the official document that has them all or that can be read where the operator's page can't (Austria's RIS, Spain's BOE, Romania's ANRE orders, the CWaPE's and Brugel's copies in Belgium); comments on how the prices were converted and on any known change ahead; public holidays only where the operator treats them as weekends, moving ones at their next date with a comment above.
- Portugal's summer and winter periods switch at the clock change, which a month calendar can't hold: the last week of October and of March are dated exceptions, to redo every year (in 2027 summer time ends on Sunday 31 October).
- How the plans of October 2026 were made: one researcher per country (Germany in two), an independent checker that fetched every source again, and a third agent to settle what they disagreed on. Every country had a disagreement, and the checkers found real errors: last year's price list, a wrong VAT rate, a sheet's preliminary version. Do the same for a new country or a yearly update.

## Where the prices are, and the traps

- Austria: every area's prices are in E-Control's SNE-V (BGBl. II Nr. 305/2025, in RIS). From 1 January 2027 the SNE-G-V brings new prices, a winter price from 22:00 to 04:00 from October to March (WiNAP), and a monthly fee per kW.
- Belgium: Fluvius's tariff list for each area; ORES's and RESA's grids on the CWaPE site (RESA's own page renders its prices with scripts); Sibelga's in Brugel's decisions (Sibelga's site refuses automated requests). Brussels' fees change structure from 2028.
- Bulgaria: EWRC's decision, which changes every 1 July.
- Croatia: HERA's decisions in Narodne novine; no change before April 2027.
- Czechia: ERÚ's price decision at the end of November, for 1 January.
- Denmark: Energi Data Service's DatahubPricelist (by ChargeOwner and the C tariff's code, like DT_C_01), in kroner without VAT. Konstant's discount (rabat) is a record of its own (C_FBTNTR_R), which can change from one month to the next. Next year's summer prices aren't out in the autumn, so the plans keep the last summer's until then.
- Finland: each company's price page, changed at any time of year. From 2029 energy fees may use only the national time division, with a power fee above 8 kW (Energiavirasto, 2 February 2026).
- France: TURPE changes every 1 August, by the CRE's decision in May.
- Germany: each operator's Preisblatt, preliminary for the next year by 15 October and final for 1 January; Modul 3's hours can change by quarter. Avacon's final 2026 sheet is the file whose name ends in "(1).pdf". Schleswig-Holstein Netz's site blocks automated reads; its 2026 sheet is at https://www.sh-netz.com/content/dam/revu-global/sh-netz/Documents/Schleswig-Holstein-Netz/Netzentgelte/Strom/Allgemeine-Netzentgelte/2026/finale_netzentgelte_strom_ab_20260101.pdf for whoever adds it by hand.
- Hungary: MEKH's decree, yearly. A2's cheap hours are in universal service's energy price, not in the network fee, so no plan can hold them.
- Italy: ARERA's TD tariff every 1 January; UC3 and UC6 can change every quarter.
- Latvia: on 1 January 2027 Pamata-1 and Pamata-2 become one Pamata, and a new plan, Jaudīgais, starts (Latvijas Vēstnesis 2026/145).
- Luxembourg: the operators publish next year's prices by 15 October, and the ILR approves them in December. 2026's include a state contribution that was for 2026 only.
- Norway: the companies' own pages, as they change prices at any time at 14 days' notice (Glitre on 1 October 2026). NVE's open data (nettleietariffer.dataplattform.nve.no) gives each company's share of homes and its prices by date, to check against.
- Portugal: ERSE's access tariffs every January. Each home moves to ERSE's new hours on its own day between 1 July and 31 December 2027.
- Romania: ANRE's orders, with a table for each operator. Rețele Electrice's own page shows DEER's, the first in Order 78/2025: its own is on the last page.
- Sweden: each company's page; there's no central source. The government dropped the requirement for power fees in 2026, and Ellevio and Mälarenergi went back to prices by fuse size; Ei's proposal for a new model is due by 12 April 2027.
- Switzerland: ElCom's comparison (strompreis.elcom.admin.ch) and the operators' sheets, published by the end of August for 1 January; 2027's prices are in each plan's comments. Some operators set network prices a day ahead (EKZ 400D, CKW, Primeo NetzDynamisch, Groupe E Vario), which a plan can't hold.

## Not done yet

Left out in October 2026, each for a reason the country's README.md or the research gives: Schleswig-Holstein Netz and Germany's smaller operators, and its reduced fees for steerable devices from before 2024; Czechia's D27d and the other rates whose low-tariff hours ripple control sets per place; Hungary's A2; Latvia's Jaudīgais, until 2027; Norway's Linja and the smaller companies; Sweden's and Finland's smaller companies; Austria's interruptible circuits (unterbrechbar); Belgium's AIEG, AIESH and REW; Bulgaria's Zlatni Pyasatsi; Portugal's tri-horária above 20.7 kVA; the share of the market price that Öresundskraft (5.57% of the month's average SE4 price) and Kraftringen (5% of each hour's) add per kWh. Fees per kW (#82) matter in more countries over time: Norway's capacity step, Sweden's power fees, Luxembourg's supplement, Slovenia's, Austria's from 2027 and Finland's from 2029.

## Settled with the owner; don't propose again

- No plans for France, whose off-peak hours are set for each address, or the Netherlands, which has no grid fee per kWh (time-of-use network fees are planned from 2029: plans then, one per operator).
- All the plans stay, without trimming the long lists of Germany, Sweden, Finland and Switzerland.
- Czechia's, Hungary's and Switzerland's plans are in their own currency, and the board converts SMARD's euros into it at the ECB's daily rate.
- The plans went in with 2026's prices, to update at the turn of the year (#161).
