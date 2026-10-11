# plans/fi/

Finland's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a spot contract (pörssisähkö), the board's charging cost 359.90 EUR over the year from 1 October 2025: 38% less than charging at once (582.93 EUR), 22% less than the car's own schedule from 22:00 (459.11 EUR) and 2% less than its best start, 03:00 (368.99 EUR). Confidence in the README is High: a third of homes pay the market price.

## Decisions

- **Contract:** a spot contract (pörssisähkö) billing Nord Pool's FI price.
- **Plan:** `fi/caruna-yosiirto`. The car's own schedule starts at 22:00, when its night fee does.
- **Billing:** each hour at the average of its four quarter-hours, as most of Caruna's meters still record hours.
- **Margin:** 0.00550 EUR per kWh with VAT (0.00438 without), the middle of four big national suppliers'.
- **The same every hour:** 0.02918 EUR per kWh with VAT, the electricity tax with the security of supply fee, at its rate from April 2026.
- **Left out:** Caruna's and the supplier's monthly fees, as they don't depend on when the car charges: Yösiirto's basic fee, 38.03 EUR a month with VAT at 3 x 25 A, and the supplier's, about 5 EUR.
- **Market prices:** Elering's dashboard, field `data.fi`.

## Findings

- **Who pays the market price:** at the end of 2025, 34% of households had an open-ended spot contract (pörssisähkö), 42% a fixed-term one, a third of them hybrids partly priced by the hour, and 24% an open-ended fixed price; spot was 33% at the end of 2024 (Energiavirasto's press release of 4 May 2026, [copied on ePressi](https://www.epressi.com/tiedotteet/energia/kulutuksen-ajoitus-vaikuttaa-jo-lahes-joka-toisen-kotitalouden-sahkolaskuun.html)). Homes on a wholly fixed price, about half, gain only from the plan's cheap hours.
- **Meters and billing:** a spot contract bills Nord Pool's FI day-ahead price with VAT, plus the margin. Since 1 October 2025 that's each quarter-hour's price where the meter measures quarter-hours (Helen's, Vattenfall's, Oomi's and Turku Energia's pages, under Margins). An hourly meter's hour is split evenly over its four quarter-hours, which bills the hour's average ([Fortum](https://www.fortum.com/fi/sahkoa/sahkon-hinta/varttisahko), [Oomi](https://oomi.fi/varttisahkon-abc/)). About 40% of Caruna's meters measured quarter-hours in September 2025 ([Caruna](https://caruna.fi/ajankohtaista/sahkomarkkinat-siirtyvat-varttihinnoitteluun-lokakuussa-2025)), so about 60% of its homes had an hourly meter, which they keep until Caruna's new meters (Changes ahead). Older contracts moved to quarter-hours later, billing the hour's average until then: Fortum's made before October 2025 on 1 December 2025, Oomi's Aktiivinen on 1 April 2026.
- **The board on an hourly meter:** it picks quarter-hours, and the example's car charges for only some 50 minutes a night. Worked out roughly on Elering's prices with Yösiirto, apart from the example, against billing each quarter-hour at its own price: billed at each hour's average, the quarter-hours the board picks cost about 19 EUR a year more in market price with VAT (16%), and hours picked by their average about 10 EUR more.
- **VAT:** 25.5% since 1 September 2024, on the market price, the margin, the network fees and the electricity tax alike ([Vero](https://vero.fi/yritykset-ja-yhteisot/verot-ja-maksut/arvonlisaverotus/arvonlisaveroprosentit/)): the same from October to December 2025 and all of 2026, as only the reduced rate changed, from 14% to 13.5%, in January 2026.
- **Per kWh, the same every hour:** only the electricity tax, class I for every home, which the network company bills with its fees ([Caruna](https://caruna.fi/tuotteet-ja-palvelut/kotiin-ja-kiinteistoon/sahkovero)) and the plans leave out (README.md):
  - from 1 April 2026, the energy tax, 0.02240 EUR, and the security of supply fee (huoltovarmuusmaksu), 0.00085: 0.02325 without VAT, 0.02918 with it ([Vero's tax tables](https://vero.fi/yritykset-ja-yhteisot/verot-ja-maksut/valmisteverotus/sahkovero/verotaulukot/); Caruna's page gives the same, 0.02917875);
  - from October 2025 to March 2026, with the fee at 0.00013: 0.02253 without VAT, 0.02828 with it (Caruna's news of 20 March 2026);
  - nothing else: the network bill is a basic fee, the energy fee and the tax ([Caruna](https://caruna.fi/tuotteet-ja-palvelut/kotiin-ja-kiinteistoon/mista-sahkonjakelun-hinta-muodostuu)), and the supplier's a basic fee, the spot price and the margin.
- **Margins:** four big national suppliers' spot contracts and a regional one, checked on 11 October 2026, each in EUR per kWh with VAT and without, and its basic fee a month with VAT:
  - Helen Pörssisähkö: 0.00490, 0.00390 and 4.96 EUR ([Helen](https://www.helen.fi/sahko/porssisahko));
  - Vattenfall Optimi Pörssisähkö: 0.00500, 0.00398 and 5.95 EUR ([Vattenfall](https://www.vattenfall.fi/sahkosopimukset/porssisahko/));
  - Fortum Tarkka: 0.00590, 0.00470 and 5.99 EUR ([Fortum](https://www.fortum.com/fi/sahkoa/sahkoa-kotiin/tarkka-porssisahko));
  - Oomi Aktiivinen: 0.00600, 0.00478 and 4.96 EUR after four months free ([Oomi](https://oomi.fi/sahko/sahkosopimukset/aktiivinen/));
  - Turku Energia Louna Pörssi, the regional one: 0.00420, 0.00335 and 4.95 EUR ([Turku Energia](https://www.turkuenergia.fi/kotitaloudet/sahko/sahkosopimukset-ja-hinnat/louna-porssi)).
- **Energiavirasto's comparison:** [sahkonhinta.fi](https://www.sahkonhinta.fi/) lists the same, through [its API for a postcode in Caruna's area](https://ev-shv-prod-app-wa-consumerapi1.azurewebsites.net/api/productlist/08100). There the prices date from 28 February 2025 (Fortum), 30 September 2025 (Vattenfall), 2 October 2025 (Helen) and 4 September 2026 (Oomi), so October to December 2025 had the same, Oomi's perhaps excepted. Its household spot offers mostly ask 0.00300 to 0.01000, with a median of 0.00500.
- **The plan:** Caruna is Finland's largest network company, with 744,000 customers and about a fifth of the country's distribution, 493,000 of them Caruna Oy's ([Caruna](https://caruna.fi/tietoa-meista/caruna-lyhyesti) and its page on the price, under Per kWh). Yösiirto's cheap fee runs from 22:00 to 07:00 every day, without holiday rules, and Caruna's household products have no power fee. The plan's prices match [Caruna Oy's list](https://caruna.fi/tuotteet-ja-palvelut/kotiin-ja-kiinteistoon/verkkopalveluhinnastot/verkkopalveluhinnasto-caruna-oy), dated 1 September 2024 and still in force on 11 October 2026: night 0.02490 and day 0.04070 EUR per kWh without VAT. halpasahko.com showed the same Yleissiirto on 7 May 2026. Caruna Oy didn't change its prices on 1 January 2026, when Caruna Espoo raised its own by 4.4% on average (Kirkkonummen Sanomat, 12 November 2025), and announces no change, so the whole year from October 2025 had these fees.
- **Why Yösiirto:** no figures say which product EV owners take, so the plan follows from the arithmetic. A car taking 3,320 kWh a year at night saves about 71 EUR on Yösiirto's night fee, 0.03125 EUR per kWh with VAT against Yleissiirto's 0.05258. A house's own use of some 3,500 kWh a year, a third of it at night, makes up the rest of the dearer basic fee, 99.84 EUR a year more at 3 x 25 A (84.96 EUR in a block of flats or a row house). Caruna advises Kausisiirto for direct electric heating and heat pumps: its fee is close to Yösiirto's at night (0.03225), and the same all day from April to October and on Sundays.
- **Market prices:** Elering's dashboard republishes Nord Pool's day-ahead prices for Finland without an account or key, a month a file, as for Lithuania: field `data.fi`, in EUR per MWh without VAT, by the quarter-hour, with Unix timestamps in UTC, like [October 2025's](https://dashboard.elering.ee/api/nps/price?start=2025-10-01T00:00:00.000Z&end=2025-11-01T00:00:00.000Z). October 2025 to October 2026 came without a gap. Its monthly averages with VAT equal the FI ones Helen publishes for its spot contract to 0.0001 EUR per kWh for every month from November 2025 to September 2026, and October 2025's within 0.0001 EUR, as a month in UTC starts three hours into Finland's: so it's the price spot contracts bill. Finland's clock is Lithuania's: UTC+2, with summer time.

## Changes ahead

- **Any time of year:** each network company's prices, which it may change in any month, telling homes at least a month before. Caruna Oy's have held since 1 September 2024.
- **2027 to 2029:** Caruna replaces all its meters, at about 750,000 sites, the hourly ones included (its news of 10 September 2026).
- **From 2029:** energy fees may use only the national time division, with a power fee above 8 kW (Energiavirasto, 2 February 2026).

## Open questions

- **Which product EV owners take:** no statistics were found.
- **Summer time:** Caruna's pages don't say whether its night follows summer time; the plans assume it does.
- **State support:** none was found for 2025 or 2026, nor a price cap or an EV network product, but the search wasn't thorough. The last support was VAT cut to 10% from December 2022 to April 2023 (vero.fi).
- **VAT on a negative price:** not stated anywhere checked. It hardly matters: 236 of the year's 18,980 night quarter-hours were negative, none below -10.01 EUR per MWh.

## Notes

- **Energiavirasto's site:** it was behind a bot check, so the contract shares come from its press release's copy on ePressi.
- **Not done yet:** Finland's smaller companies, and Helen Sähköverkko Aikasiirto's charge per kW, which the plans leave out (#82).

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Caruna Oy's price list](https://caruna.fi/tuotteet-ja-palvelut/kotiin-ja-kiinteistoon/verkkopalveluhinnastot/verkkopalveluhinnasto-caruna-oy) | the example's plan's fees | any time of year |
| Each company's price page, linked in the first line of its plans | the other plans' fees and hours | any time of year |
| [Energiavirasto's comparison](https://www.sahkonhinta.fi/), through [its API](https://ev-shv-prod-app-wa-consumerapi1.azurewebsites.net/api/productlist/08100) | the suppliers' spot offers: margins and basic fees | any time |
| Energiavirasto's press release, [copied on ePressi](https://www.epressi.com/tiedotteet/energia/kulutuksen-ajoitus-vaikuttaa-jo-lahes-joka-toisen-kotitalouden-sahkolaskuun.html) | households' contract types at the end of the year | each year |
| [Vero's electricity tax tables](https://vero.fi/yritykset-ja-yhteisot/verot-ja-maksut/valmisteverotus/sahkovero/verotaulukot/) | the energy tax and the security of supply fee | when the law changes, as on 1 April 2026 |
| [Elering's dashboard](https://dashboard.elering.ee/api/nps/price?start=2025-10-01T00:00:00.000Z&end=2025-11-01T00:00:00.000Z) | the market prices, a month a request | daily |
