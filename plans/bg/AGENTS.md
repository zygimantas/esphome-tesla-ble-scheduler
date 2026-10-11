# plans/bg/

Bulgaria's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On the regulated price, the board's charging cost 306.40 EUR over the year from 1 October 2025: 40% less than charging at once (514.17 EUR), and the same as the car's own schedule from 23:00 (306.40 EUR), which was also its best start. Confidence in the README is High: nearly every home pays a regulated price, cheaper at night.

## Decisions

- **Contract:** the regulated price, as nearly every home pays it: EWRC's price from 1 July 2026, with ERM Zapad's and ESO's network fees and VAT, as a custom plan: night 0.09192 and day 0.15425 EUR per kWh with VAT, the night 22:00 - 06:00 from November to March and 23:00 - 07:00 from April to October.
- **Plan:** that custom plan, whose rates include the fee of `bg/erm-zapad-households`. The car's own schedule starts at 23:00.
- **Margin and the same every hour:** nothing on top: the custom plan's rates are the whole regulated price, ESO's fees and VAT included.
- **Left out:** monthly fees, as they don't depend on when the car charges; regulated households pay none.
- **Market prices:** none, as the regulated price doesn't follow the market.

## Findings

- **Who pays the market price:** hardly any home. Nearly every household buys at a regulated price from its end supplier: Electrohold Sales in the west, EVN Bulgaria Elektrosnabdyavane in the south-east, ENERGO-PRO Sales in the north-east. In 2024 that was 4,757,979 of 4,760,505 households, and 2,326 of the rest had a supplier of last resort ([EWRC's 2025 report to the European Commission](https://www.dker.bg/uploads/2025/EWRC_Report_EC_2025_bg.pdf), section 3.2.2). [ACER](https://aegis.acer.europa.eu/chest/dataitems/1001/view) counts 100% regulated. Parliament put off opening the household market indefinitely in May 2025 ([news.bg](https://news.bg/energy/ofitsialno-liberalizatsiyata-na-pazara-na-tok-za-bita-se-otlaga-za-neopredelen-srok.html)). A household may leave for a trader and come back later.
- **Meters and billing:** traders' exchange-price offers bill IBEX's day-ahead price for Bulgaria, averaged over the month by the site's consumption in each hour or quarter-hour. A household whose meter doesn't record quarter-hours is settled on the network operator's standard load profile (the Electricity Trading Rules, [art. 86(2)](https://www.dker.bg/uploads/normative_docs/prav_targ_el_en_072025.pdf); [ERM_H1 and ERM_H2](https://ermzapad.bg/bg/za-klienta/svoboden-pazar/smyana-na-dostavchik/standartizirani-tovarovi-profili/) in the west), so when the car charges doesn't change its bill. A meter that records them costs 327.37 lev with VAT to fit, or 100.84 lev to reprogram, at ERP Sever (Energy Act art. 120(5); [its price list of 28 March 2025](https://www.erpsever.bg/web/files/richeditor/documents/cenorazpisi/2025-03-28-edn-pricelist-in-force-by-28-03-2025-v1.pdf)).
- **VAT:** 20% on the whole bill, the standard rate (VAT Act art. 66; the reduced 9% of art. 66a doesn't cover electricity), with no other rate from October 2025 to September 2026. [EVN's](https://www.evn.bg/Home/Electricity.aspx) and [ER Yug's](https://www.elyug.bg/Customers/Prices/Bitovi_Klienti.aspx) price pages apply 20%.
- **Energy price:** EWRC now sets a base value for the energy, and the Electricity System Security Fund pays the end suppliers the difference to what the energy cost them, 24.56 EUR/MWh from July 2026, for their household customers only. From 1 July 2026 to 30 June 2027 the energy costs 0.09297 EUR per kWh by day and on single-rate meters and 0.04103 at night, without VAT ([EWRC's decision Ц-13 of 1 July 2026](https://www.dker.bg/uploads/reshenia/2026/res-c-13-2026.pdf)). With the network fees and VAT that's 0.15425 and 0.09192 in ERM Zapad's area, 0.15458 and 0.09226 in ER Yug's and 0.15512 and 0.09280 in ERP Sever's. From 1 July 2025 it was 0.17514 and 0.07546 lev (0.08955 and 0.03858 EUR), 0.14974 and 0.08858 EUR with the fees and VAT in the west. The night is 22:00 - 06:00 from November to March and 23:00 - 07:00 from April to October, every day ([Electrohold](https://electrohold.bg/bg/sales/domakinstva/snabdyavane-po-regulirani-ceni/), [ENERGO-PRO](https://www.energo-pro.bg/bitovi-klienti/ceni-tarifi)).
- **Per kWh, the same every hour:** 0.00983 EUR with VAT from 1 July 2026, which the regulated prices above include:
  - ESO's access to the transmission network, 0.00018 EUR without VAT and 0.00022 with, and its transmission, 0.00801 and 0.00961 (Ц-13, I.1.1 and I.1.4; the same on ER Yug's and EVN's pages). From 1 July 2025 to 30 June 2026 they were 15.22 lev per MWh, 7.78 EUR by EWRC's conversion, 0.00934 with VAT: EWRC's decision Ц-40 of 30 December 2025 kept them, and the network fees, for the first half of 2026;
  - the obligations to society fee, 0, as the Electricity System Security Fund covers it (Ц-13, I.6);
  - no excise on electricity for a household's use (Excise Duties and Tax Warehouses Act art. 34a(2), and 0 on EVN's bills), against 2 lev (1.02 EUR) per MWh for businesses;
  - no charge that's a share of the bill, and no monthly fee for regulated households.
- **Margins:** 0.01882 EUR per kWh with VAT (0.01568 without), worked out as halfway between the middle two (FLEKS at its floor and EHP Borsa +) of the four offers for homes that state an adder on EWRC's comparison platform ([platforma.dker.bg](https://platforma.dker.bg/traders), read 11 October 2026; it shows no dates). All four include balancing, and four more offers for homes give no figure:
  - Ares Energy's AresEnergy Bitovi: IBEX + 8.80 EUR/MWh;
  - EVN's FLEKS: IBEX + 2.71 EUR/MWh for 6 months, then + 7% but at least 12.94 EUR/MWh, with 2.55 EUR a month per meter, for 24 months, for supply from 1 May 2026; [EVN's own page](https://www.evn.bg/Business/Electricity/BusinessElectricityFree/Tarifen-plan-flex-percent.aspx) lists it for businesses;
  - Electrohold Sales' EHP Borsa +: IBEX + 18.41 EUR/MWh;
  - Electrohold Trade's optimal: IBEX + 18.82 EUR/MWh.
- **The plans' fees:** ERM Zapad, the west with Sofia, carried 41% of the electricity distributed in 2024, ER Yug 36% and ERP Sever 22% (EWRC's report). Each plan is its operator's access and low voltage transmission fees for households, the same every hour, so there's no cheap fee to start at; only the regulated energy price has a night rate. All three plans match Ц-13 and the operators' own pages from 1 July 2026: ERM Zapad 0.00347 + 0.02391, ER Yug 0.00419 + 0.02347 and ERP Sever 0.00499 + 0.02312 EUR per kWh without VAT, 0.03286, 0.03319 and 0.03373 with VAT. From 1 July 2025 to 30 June 2026 (decision Ц-25, in EWRC's euros) they were 0.03294, 0.03307 and 0.03460 with VAT.
- **The euro:** households pay in euros since 1 January 2026, at 1.95583 lev to the euro ([European Commission](https://economy-finance.ec.europa.eu/euro/eu-countries-and-euro/bulgaria-and-euro_en)), with prices shown in lev too until 8 August 2026. The plans are in EUR, and 2025's lev prices convert at that rate.
- **Market prices:** IBEX's day-ahead prices for Bulgaria, which exchange-price offers bill. [Energy-Charts' API](https://api.energy-charts.info/price?bzn=BG&start=2025-10-01&end=2025-10-31) has them without an account or key, in EUR/MWh by quarter-hour from 1 October 2025, complete to 30 September 2026; its start and end are Bulgarian dates, the end day included. They equal Nord Pool's quarter-hour for quarter-hour on 10 October 2026, and their averages over the CET days of 11 and 12 October 2026, 175.16 and 235.50 EUR/MWh, are IBEX's base prices ([3e-news](https://3e-news.net/bg/pazar-na-elektricheska-energija/elektroenergijni-borsi)).
- **The market against the regulated night:** worked out from these prices, the cheapest quarter-hours of each night from October 2025 to September 2026 (9.1 kWh at 11 kW between 18:00 and 07:00) averaged 0.09766 EUR per kWh on the market alone, without VAT, and 0.17869 with the margin above, ERM Zapad's and ESO's fees and VAT, against the regulated night's 0.09192. So the regulated night is cheaper even with a meter that records quarter-hours.

## Changes ahead

- **1 July each year:** a new EWRC decision, with the regulated energy prices and the network fees, the operators' and ESO's. Ц-13's prices run to 30 June 2027, and 1 July 2027 starts a new regulatory period.

## Open questions

- **How many homes pay the market price:** no count of households on exchange-price contracts newer than 2024's (EWRC's report to the European Commission).
- **Billing a home's own quarter-hours:** no offer found bills a household its own quarter-hours: they all average by the site's consumption, which for nearly every household is the standard profile.
- **The adders' dates:** EWRC's platform shows none.
- **The night's hours at EVN:** EVN's page doesn't show them; Electrohold's and ENERGO-PRO's agree.
- **2025's energy prices in euros:** 0.08955 and 0.03858 are converted at the fixed rate, as EWRC's decision gives only the network fees in euros.

## Notes

- **Not done yet:** Zlatni Pyasatsi (Golden Sands), whose own small operator has no plan here; this folder's README.md tells its homes to write a custom plan.
- **Energy-Charts:** take a month at a time, as quick repeats get Too Many Requests. Its terms allow private and internal use only.
- **IBEX and Nord Pool:** ibex.bg answers scripts with a JavaScript check, and Nord Pool answers 401 for days before its last two months.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [EWRC's decision Ц-13 of 1 July 2026](https://www.dker.bg/uploads/reshenia/2026/res-c-13-2026.pdf) | the regulated energy prices, the operators' and ESO's network fees, the obligations to society fee | each 1 July |
| [ERM Zapad's](https://ermzapad.bg/bg/za-klienta/ceni-i-nachini-na-plashane/ceni-za-dostp-i-prenos/), [ER Yug's](https://www.elyug.bg/Customers/Prices/Bitovi_Klienti.aspx) and [ERP Sever's](https://www.erpsever.bg/bg/ceni/ceni-za-prenos-i-dostyp) prices | the plans' fees, and ESO's on ER Yug's | each 1 July |
| [Electrohold's](https://electrohold.bg/bg/sales/domakinstva/snabdyavane-po-regulirani-ceni/), [EVN's](https://www.evn.bg/Home/Electricity.aspx) and [ENERGO-PRO's](https://www.energo-pro.bg/bitovi-klienti/ceni-tarifi) prices | the end suppliers' regulated prices, and the night's hours on Electrohold's and ENERGO-PRO's | each 1 July |
| [EWRC's comparison platform](https://platforma.dker.bg/traders) | the traders' offers for homes and their adders, without dates | any time |
| [EWRC's report to the European Commission](https://www.dker.bg/uploads/2025/EWRC_Report_EC_2025_bg.pdf) | households by supplier, the operators' shares of the electricity distributed | each year |
| [Energy-Charts' API](https://api.energy-charts.info/price?bzn=BG&start=2025-10-01&end=2025-10-31) | IBEX's prices for Bulgaria, a month a request | daily |
