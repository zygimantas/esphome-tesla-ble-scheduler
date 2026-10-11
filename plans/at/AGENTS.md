# plans/at/

Austria's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a spot product billing EPEX's AT price, in Vienna, the board's charging cost 906.66 EUR over the year from 1 October 2025: 17% less than charging at once (1,095.23 EUR), 5% less than the car's own schedule from midnight (954.33 EUR) and 0% less than its best start, 03:00 (909.43 EUR). Confidence in the README is Low: about 1% of homes paid the market price in 2025.

## Decisions

- **Contract:** a spot product billing EPEX's AT price, the price the board downloads.
- **Plan:** `at/wiener-netze`. Its fee is the same all night, so the car's own schedule starts at midnight.
- **Billing:** each hour at the average of its four quarter-hours, as most spot products bill.
- **Margin:** 0.018 EUR per kWh with VAT, aWATTar's, about the middle of seven offers.
- **The same every hour:** 0.00807 EUR per kWh with VAT for the electricity tax and the renewables levies, the figure that, times Vienna's 1.07, matches the bill.
- **Factor:** 1.07 on all of it, Vienna's Gebrauchsabgabe.
- **Left out:** the renewables levy's and the network's yearly fees, the meter fee and the suppliers' monthly fees, as they don't depend on when the car charges.
- **Market prices:** SMARD's series 4170.

## Findings

- **Who pays the market price:** households mostly pay one price at every hour. In April 2025 about 90% of household products were fixed-price products, a little over 6% monthly floaters, over 3% time-of-use products and 1% spot products ([E-Control's Tätigkeitsbericht 2026](https://www.e-control.at/documents/1785851/1811582/E-Control-Taetigkeitsbericht-2026-Barrierefrei.pdf/537a1fa1-a945-2ee5-55bc-68ff56da5c36), pages 61 to 63). E-Control counted about 18 spot products at the end of 2025. Since 1 April 2026 every supplier with more than 25,000 metering points must offer one ([ElWG § 22, BGBl. I Nr. 91/2025](https://www.ris.bka.gv.at/eli/bgbl/I/2025/91)).
- **Meters and billing:** a spot product needs a smart meter that sends quarter-hours to the supplier: about 472,000 did at the end of 2024, 119,000 of them in energy communities (E-Control's Smart Meter report 2025, page 26). Nearly all bill "EPEX Spot AT", the day-ahead price of the Austrian bidding zone from the coupled auction (SDAC), the same price Nord Pool and SMARD publish. Most bill it by the hour, as the average of the hour's four quarter-hours: aWATTar, smartENERGY, Energie Steiermark and Wien Energie. Since the market went to quarter-hours on 1 October 2025, VKW and Energie AG bill quarter-hours, and EVN's FAQ says its spot tariff does too. Salzburg AG bills the hourly price of EXAA's own auction instead. So a schedule by quarter-hours saves most of them a little less than it would with quarter-hours billed.
- **VAT:** 20% on everything, the levies included, from October 2025 to September 2026. Electricity has no reduced rate (the USP's list of rates, https://www.usp.gv.at/themen/steuern-finanzen/umsatzsteuer-ueberblick/steuersaetze-und-steuerbefreiungen-der-umsatzsteuer.html, and every price sheet here).
- **Per kWh, the same every hour:** 0.00864 EUR with VAT in 2026, for a household (network level 7 without power metering):
  - the electricity tax (Elektrizitätsabgabe), 0.00100 EUR without VAT, 0.00120 with it, from 1 January to 31 December 2026, for natural persons on a household load profile (H0, HA or HF); other metering points pay 0.00820 (§ 7 Abs. 16 Elektrizitätsabgabegesetz, BGBl. I Nr. 95/2025, https://www.ris.bka.gv.at/eli/bgbl/I/2025/95, and motion 658/A). Until 31 December 2025 it was 0.01500, 0.01800 with VAT;
  - the renewables levy (Erneuerbaren-Förderbeitrag), 0.00583 tied to the usage fee and 0.00037 tied to the loss fee, 0.00620 EUR without VAT, 0.00744 with it ([Erneuerbaren-Förderbeitragsverordnung 2026, BGBl. II Nr. 301/2025](https://www.ris.bka.gv.at/eli/bgbl/II/2025/301)). In 2025 it was 0.00796, 0.00955 with VAT (BGBl. II Nr. 419/2024);
  - Wiener Netze's sheets give the same sum: https://www.wienernetze.at/o/document/wn_ex0106_steuernundabgaben_202603_bf and https://www.wienernetze.at/o/document/ex104_erneuerbaren-forderpauschale-und-forderbeitrag_2026_bf-pdf. In October to December 2025 it was 0.02755, 0.01891 more at every hour;
  - the yearly fees, left out: the renewables levy's 3.796 EUR a year per metering point, the Erneuerbaren-Förderpauschale (19.02 EUR a year, 2025 to 2027), the network's 54.00 EUR a year (48.00 in 2025) and the meter fee.
- **Vienna's Gebrauchsabgabe:** a municipal levy of 7% of the net energy price and the net network price (usage, losses, meter) since 1 March 2026, 6% before. It isn't charged on the electricity tax or the renewables levies, and VAT comes on top ([Wiener Netze's taxes and levies](https://www.wienernetze.at/o/document/wn_ex0106_steuernundabgaben_202603_bf), [Wien Energie's price sheet of 1 October 2026](https://www.wienenergie.at/preisblatt-optimaentspanntplus)). So in Vienna the multiplier is 1.07 on the market price, the margin and the plan's fee; applied to the flat part too, it needs 0.00807 there instead of 0.00864. Wiener Netze's parts of Lower Austria and Burgenland don't pay it.
- **Margins:** seven offers, all checked on 11 October 2026, per kWh without VAT, with their monthly fee without VAT; aWATTar's is about the middle:
  - smartENERGY smartCONTROL: hourly price + 0.01200 (0.01440 with VAT), 2.49 EUR, sheet valid from 1 February 2024 ([smartENERGY](https://www.smartenergy.at/smartcontrol));
  - Energie Steiermark SteirerStrom Spot, from the same company as smartENERGY: hourly + 0.01200, 3.18 EUR, valid from 1 June 2026 ([Energie Steiermark](https://www.e-steiermark.com/privat/produkte/strom/spottarif));
  - VKW Strom Dynamisch, Vorarlberg only: quarter-hours + 0.01200, 3.00 EUR ([VKW](https://www.vkw.at/produkte/strom/strom-dynamisch));
  - Wien Energie OPTIMA Voll Aktiv, for its existing customers only: hourly + 0.01420 + 7% of the market price, about 4.32 EUR ([Wien Energie](https://www.wienenergie.at/privat/produkte/strom/optima-voll-aktiv/));
  - aWATTar HOURLY: hourly + 0.01500 (0.01800 with VAT), 4.79 EUR, sheet valid for 11 October 2026 ([aWATTar](https://www.awattar.at/tariffs/hourly));
  - Salzburg AG Strom FlexSpot: EXAA's hourly price + 3% + 0.01800, 4.17 EUR, capped at 0.76320 with VAT ([Salzburg AG](https://www.salzburg-ag.at/strom/privat/stromprodukte/strom-flexspot.html));
  - Energie AG Ökostrom SPOT: quarter-hours + 0.02500 (0.01667 in the first year with its bonus), 4.50 EUR ([Energie AG](https://www.energieag.at/privat/strom/flexible-tarife/spot));
  - Tibber: its site lists no Austrian offer.
- **The plan:** `at/wiener-netze`. Wiener Netze calls itself Austria's largest network operator, with 1.6 million electricity meters ([Wiener Netze](https://www.wienernetze.at/wir-in-zahlen)), and E-Control counted 1.64 million of Austria's 6.36 million metering points in the Vienna area at the end of 2024, the most of any area (Statistikbroschüre 2025). Its fee is the same at every hour of the night all year: the only lower fee is SNAP, 10:00 - 16:00 from April to September, so no cheap fee starts at night. Its prices, 0.06980 for usage (0.05580 SNAP) and 0.00700 for losses without VAT, make 0.09216 and 0.07536 with VAT, as in the plan. They match the SNE-V 2018 - Novelle 2026 (BGBl. II Nr. 305/2025), the plan's first line, and Wiener Netze's own sheet, valid since 1 January 2026 and still current ([Wiener Netze](https://www.wienernetze.at/o/document/wn_ex0105-netznutzungs-und-netzverlustentgelt_2026_v2_01_bf)). All 13 plans in this folder match the SNE-V. In October to December 2025 the fee was 0.07400 + 0.00700, 0.09720 with VAT, 0.00504 more at every hour (BGBl. II Nr. 370/2024). Worked out from SMARD's prices, on the market alone a 50-minute charge cost least from about 03:30 over the 365 nights: 0.11330 EUR per kWh without VAT, against 0.15420 from 18:00.
- **Electric cars:** Vienna had 45,628 of Austria's 257,717 battery cars at the end of 2025, Lower Austria 52,105 and Upper Austria 51,343 (Statistik Austria).
- **Market prices:** SMARD's series 4170, Austria's day-ahead price in EUR per MWh, a file a week from Monday 00:00 German time, without an account or key, like https://www.smard.de/app/chart_data/4170/DE/4170_DE_quarterhour_1766962800000.json, with the weeks listed in https://www.smard.de/app/chart_data/4170/DE/index_quarterhour.json. It has distinct quarter-hours from 1 October 2025 and every quarter-hour to 1 October 2026 07:00. It matched Nord Pool's AT prices, the board's, on all 96 quarter-hours of 6 October 2026, and its hourly averages matched aWATTar's billed hourly prices on 12 October 2025 ([aWATTar's API](https://api.awattar.at/v1/marketdata?start=1760220000000&end=1760306400000)). So it's the price the EPEX-based contracts bill, by quarter-hour or as the hour's average, but not Salzburg AG's EXAA price.

## Changes ahead

- **1 January each year:** E-Control's network prices for every area, which the plans follow, and the renewables levy's ordinance.
- **1 January 2027:** the electricity tax goes back to 0.01500 EUR without VAT unless extended. E-Control's SNE-G-V, a draft in October 2026, brings new network prices, a winter price 22:00 - 04:00 from October to March (WiNAP), and a monthly fee per kW.

## Open questions

- **Today's share of spot products:** E-Control's 1% is from April 2025, before ElWG § 22 and quarter-hour prices. Its yearly report under ElWG § 22 (6) would give it, but wasn't found.
- **Where the cars charge:** home chargers are probably more often in houses outside Vienna, without the Gebrauchsabgabe. Every plan here is flat at night, so the plan changes the totals, not the saving.
- **A wallbox on a meter of its own:** like an interruptible one, it may not have a household load profile, and then pays 0.00820 electricity tax.
- **Gebrauchsabgaben:** aWATTar's sheet says up to 6% in some municipalities. Vienna's law itself wasn't read; its 7% is from Wiener Netze's and Wien Energie's sheets.
- **Tibber:** whether it still supplies Austrian homes.

## Notes

- **Interruptible circuits:** not done yet. Austria's interruptible circuits (unterbrechbar) have no plans; Vienna's cost 0.04210 + 0.00700 EUR per kWh without VAT, with no yearly fee.
- **Fees per kW:** not done yet. The board doesn't count them (#82), and the SNE-G-V brings a monthly one from 2027.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [E-Control's SNE-V, BGBl. II Nr. 305/2025, in RIS](https://www.ris.bka.gv.at/eli/bgbl/II/2025/305) | every area's network prices, the plans' | each 1 January |
| [Wiener Netze's network prices for 2026](https://www.wienernetze.at/o/document/wn_ex0105-netznutzungs-und-netzverlustentgelt_2026_v2_01_bf) | the example plan's fees, to check against | each 1 January |
| [Wiener Netze's taxes and levies](https://www.wienernetze.at/o/document/wn_ex0106_steuernundabgaben_202603_bf) and [renewables levy](https://www.wienernetze.at/o/document/ex104_erneuerbaren-forderpauschale-und-forderbeitrag_2026_bf-pdf) | the levies per kWh and the Gebrauchsabgabe, to check against | when one changes |
| [Elektrizitätsabgabegesetz, BGBl. I Nr. 95/2025](https://www.ris.bka.gv.at/eli/bgbl/I/2025/95) | the electricity tax | 1 January 2027, unless extended |
| [Erneuerbaren-Förderbeitragsverordnung 2026, BGBl. II Nr. 301/2025](https://www.ris.bka.gv.at/eli/bgbl/II/2025/301) | the renewables levy | each year |
| [aWATTar HOURLY](https://www.awattar.at/tariffs/hourly) and the other offers under Margins | the margins and monthly fees | any time |
| [E-Control's Tätigkeitsbericht](https://www.e-control.at/documents/1785851/1811582/E-Control-Taetigkeitsbericht-2026-Barrierefrei.pdf/537a1fa1-a945-2ee5-55bc-68ff56da5c36) | the shares of household products by kind | each year |
| [SMARD's series 4170](https://www.smard.de/app/chart_data/4170/DE/index_quarterhour.json) | the market prices, a week a file | daily |
