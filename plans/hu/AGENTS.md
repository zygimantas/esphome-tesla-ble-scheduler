# plans/hu/

Hungary's research, checked in October 2026, for this folder's plan and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On universal service's regulated price, with A2 on the car's own meter, the board's charging cost 137,959.94 HUF (369.76 EUR) over the year from 1 October 2025: 12% less than charging at once (156,520.72 HUF, 419.51 EUR) and the same as the car's own schedule from 23:00, which is also its best start (0% less). Confidence in the README is Medium: homes pay regulated prices, and the car's own meter (A2) pays less at night.

## Decisions

- **Contract:** universal service's regulated price, as no home was billed a dynamic one in the year. The example takes A2 on the car's own meter, as an owner who charges at home would: cheaper from 22:00 winter time on workdays and all weekend, within its own 2523 kWh a year.
- **Area:** ELMŰ's network area, for A2's prices.
- **Plan:** a custom plan with A2's prices. The car's 3,333 kWh a year pass its band, so each rate blends its price with the 70.10400 above the band, by their shares: 49.175 at peak and 41.388 off-peak, HUF per kWh with VAT. The car's own schedule starts at 23:00, when A2's off-peak does on the clock in summer (22:00 winter time).
- **Margin and the same every hour:** none: A2's prices include the network fees and VAT, and homes pay no levies or excise.
- **Left out:** the network's yearly basic fee and the monthly fees, as they don't depend on when the car charges.
- **Market prices:** none, as A2's prices are regulated.
- **Currency:** the plans are in forints, and the board converts SMARD's euros into them at the ECB's daily rate, as the owner settled.

## Findings

- **Who pays the market price:** no home was billed a dynamic price from October 2025 to September 2026. Homes buy from MVM Next, the only universal service supplier, at regulated prices that are the same every hour (Energy price, below). Since 31 July 2026 any trader must offer a flexible price on request for a remotely read meter (VET 61/A), but leaving universal service gives up the regulated price, and no such offer was found.
- **D, the first dynamic price for homes:** MVM Next's D tariff ([page](https://www.mvmnext.hu/aram/pages/aloldal.jsp?id=16455187)) has taken applications since 1 September 2026, and bills from 1 January 2027 at the earliest. It's only for an A1 meter that is read remotely and billed monthly by the quarter-hour, and not for prepaid meters or net-metered solar. Up to 2523 kWh a year, counted at 6.91 kWh a day, the home keeps A1's price. Above that, a quarter-hour costs HUPX's day-ahead price at the MNB's euro rate of the day plus MVM Next's fee, 13.70000 without VAT ([notice of 10 September 2026](https://www.mvmnext.hu/aram/servlet/download?type=file&id=16704), changeable at 60 days' notice), then the network fees and VAT. The month's kWh above the band are billed at the average of those prices weighted by all the month's kWh, so moving a kWh to a cheaper quarter-hour saves only the above-band share of the difference: about 57% for a home that uses its band and adds the car's 3,333 kWh. No figures on take-up were found.
- **D at night:** it wouldn't have paid off. Worked out roughly by quarter-hour from the prices of the year from 1 October 2025, with VAT and the network fees but without the weighting: each night's cheapest quarter-hours from 18:00 to 07:00 averaged about 98 forints a kWh, from 22:00 about 120 and at once about 134, against the fixed 70.10400.
- **VAT:** 27% on all of it: the energy, the network fees and D's fee ([MVM Next](https://www.mvmnext.hu/aram/arak); [20/2022 MEKH rendelet](https://net.jogtar.hu/jogszabaly?docid=a2200020.mek) counts the price above the band with 27%). The same from October to December 2025.
- **Per kWh, the same every hour:** nothing besides the plan's network fees. Homes pay no levies: the pénzeszközök are for non-household consumers only ([VET](https://net.jogtar.hu/jogszabaly?docid=a0700086.tv) 147. § (2)). They pay no excise duty on electricity either: it's due only on sales to non-household consumers ([2016. évi LXVIII. törvény](https://net.jogtar.hu/jogszabaly?docid=a1600068.tv)).
- **Energy price:** households buy only at universal service's prices, set by [4/2011 NFM rendelet](https://net.jogtar.hu/jogszabaly?docid=a1100004.nfm) (2. melléklet, in force since 1 August 2025) and 20/2022 MEKH rendelet, the same from October to December 2025 as in 2026. [MVM Next's price list](https://www.mvmnext.hu/aram/servlet/download?type=file&id=16214) has them by network area, and its [FAQ](https://www.mvmnext.hu/lakossagirezsi/gyakorikerdesek-aram) the band's rules. Per kWh with VAT, the plan's 29.71800 included, in the order MVM Démász / E.ON and OPUS TITÁSZ / ELMŰ / MVM Émász:
  - A1, the same every hour, up to 2523 kWh a year per meter: 36.38550 / 35.29330 / 36.20770 / 35.99180.
  - Above 2523 kWh a year, A1 and A2 alike, every hour: 70.10400 everywhere, 40.38600 of it energy. A car's 3,333 kWh a year mostly fall here, as the band is the average home's use.
  - A2, on the car's own meter: an electric car's owner who proves the car every year may add A2 next to A1 (4/2011 NFM 4. § (5a), since 2016). It gets its own 2523 kWh, as the band counts per tariff and meter (6. § (1)): peak 41.08450 / 43.40860 / 42.45610 / 40.50030, off-peak 31.05150 / 32.76600 / 32.16910 / 30.98800, then 70.10400. Off-peak is 22:00 - 06:00 on workdays (23:00 - 07:00 in summer time, so 22:00 - 06:00 CET all year) and all day on other days.
  - B, controlled: the network operator switches it on 8 hours a day, for a separately metered, hard-wired device that can run in spells (4/2011 NFM 5. § (1)): 22.96160 / 23.52040 / 23.15210 / 22.68220 up to its own 2523 kWh, then 60.93500. No plan can say when.
  - H is for heat pumps and renewable heating only, from 15 October to 15 April at every hour, and B GEO takes no new contracts since 1 September 2021.
- **The plan:** `hu/rendszerhasznalati-dij` fits every household: universal service and D both bill the network fees per kWh on top of the energy, as universal service's discounted price is energy only (4/2011 NFM 3. § (2)), while its price above the band includes them. They're the same every hour, with or without a smart meter: in 2026, 3.39000 transmission and 20.01000 distribution without VAT, 29.71800 with VAT, as the plan has (MEKH's H2995/2025, as [MVM Hálózat posts it](https://mvmhalozat.hu/aram/oldalak/1456), and [E.ON's sheet](https://www.eon.hu/content/dam/eon/eon-hungary/documents/hatarozatok-szabalyzatok-aram/arak-tarifak/Aram_RHD_EON_20260101.pdf)). In 2025 they were 4.84000 and 18.56000 (H4437/2024, [E.ON's sheet](https://www.eon.hu/content/dam/eon/eon-hungary/documents/hatarozatok-szabalyzatok-aram/arak-tarifak/Aram_RHD_EON_20250101.pdf)): the same 29.71800 from October to December 2025. The plan has no cheap hours: the only cheaper night is A2's energy price within its band, from 22:00 CET on workdays. A remotely read meter that isn't a smart meter (KIF III.) pays 22.20000 instead of 20.01000, plus 72,168 forints a year and 13,248 per kW a year without VAT. The network's basic fee is 1,446 forints a year without VAT per connection, 474 for a controlled meter.
- **Market prices:** SMARD's series 262, HUPX's day-ahead prices in euros per MWh, a file a week from Monday 00:00 German time, as the board reads them. All 35,136 quarter-hours from 1 October 2025 to 1 October 2026 are there; before 1 October 2025 each quarter-hour has its hour's price. The board converts them at the ECB's daily rate; the [MNB's](https://www.mnb.hu/arfolyam-tablazat?deviza=rbCurrencySelect&devizaSelected=EUR&datefrom=2025.10.01.&datetill=2026.09.30.&order=1), which D uses, differed by 0.01% on average and by 1.6% at most on a day. The forint went from about 390 a euro in October 2025 to 354 in June 2026 and 365 in September. [MVM Next's D prices](https://www.mvmnext.hu/aram/servlet/download?type=file&id=16727), a PDF of the last 12 months without VAT and with the fee, equal SMARD's at the MNB's rate plus 13.70000, to the rounding, in all 35,040 quarter-hours from September 2025 to August 2026, negative ones included.

## Changes ahead

- **1 January each year:** the network fees, by MEKH's yearly decree. 2027's weren't checked.
- **1 January 2027 at the earliest:** D bills its first homes.
- **Any time:** D's fee, at 60 days' notice.

## Open questions

- **Homes outside universal service:** MEKH's figures on them weren't found, as its site refused connections.
- **D's take-up:** how many homes have applied for it.
- **D's network fees:** whether its remotely read meters pay the smart meter's network fees or KIF III.'s.
- **A2's band:** how its daily band splits between peak and off-peak.
- **A wallbox on B:** whether network operators accept one on a B circuit.

## Notes

- **MEKH's site:** it refused connections, so its resolutions were read in MVM Hálózat's and Jogtár's copies.
- **Not done yet:** a plan for A2. Its cheap hours are in universal service's energy price, not in the network fee, so no plan in this folder can hold them; a custom plan with A2's whole prices can (README.md), as the example's does.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| MEKH's decree on the network fees (H2995/2025 for 2026), as [MVM Hálózat posts it](https://mvmhalozat.hu/aram/oldalak/1456), and [E.ON's sheet](https://www.eon.hu/content/dam/eon/eon-hungary/documents/hatarozatok-szabalyzatok-aram/arak-tarifak/Aram_RHD_EON_20260101.pdf) | the plan's network fees | yearly, from 1 January |
| [4/2011 NFM rendelet](https://net.jogtar.hu/jogszabaly?docid=a1100004.nfm), [20/2022 MEKH rendelet](https://net.jogtar.hu/jogszabaly?docid=a2200020.mek), and [MVM Next's price list](https://www.mvmnext.hu/aram/servlet/download?type=file&id=16214) and [FAQ](https://www.mvmnext.hu/lakossagirezsi/gyakorikerdesek-aram) | universal service's prices by network area, A1, A2, B and above the band, and the band's rules | by decree, last on 1 August 2025 |
| [MVM Next's D tariff](https://www.mvmnext.hu/aram/pages/aloldal.jsp?id=16455187) and [its fee](https://www.mvmnext.hu/aram/servlet/download?type=file&id=16704) | D's terms and fee | at 60 days' notice |
| [VET](https://net.jogtar.hu/jogszabaly?docid=a0700086.tv) 147. § (2) and [2016. évi LXVIII. törvény](https://net.jogtar.hu/jogszabaly?docid=a1600068.tv) | that homes pay no levies or excise duty | any time, by law |
| [SMARD's series 262](https://www.smard.de/app/chart_data/262/DE/262_DE_quarterhour_1759096800000.json), the week from 29 September 2025, and [its list of weeks](https://www.smard.de/app/chart_data/262/DE/index_quarterhour.json) | HUPX's day-ahead prices in euros per MWh, a week a file | daily |
