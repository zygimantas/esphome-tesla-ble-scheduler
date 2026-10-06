# Swedish plans

The household plans of Sweden's eight biggest network companies, which together have nearly two thirds of the country's connections, with prices with VAT, in kronor. Your network bill names your company, your subscription and your main fuse. Choose your plan under **Grid plan** on the board's page.

| Network company | Plan | File |
| --- | --- | --- |
| Ellevio | Houses, apartments and single phase 25 A to 35 A | `se/ellevio` |
| Ellevio | Single phase up to 20 A | `se/ellevio-single-phase-20a` |
| Vattenfall Eldistribution | Enkeltariff | `se/vattenfall-enkeltariff` |
| Vattenfall Eldistribution | Tidstariff | `se/vattenfall-tidstariff` |
| Göteborg Energi | House | `se/goteborg-energi-house` |
| Göteborg Energi | Apartment | `se/goteborg-energi-apartment` |
| Göteborg Energi | Tidsindelad | `se/goteborg-energi-tidsindelad` |
| Mälarenergi | Houses and apartments | `se/malarenergi` |
| Öresundskraft | Enkeltariff, and apartments | `se/oresundskraft-enkeltariff` |
| Öresundskraft | Tidstariff | `se/oresundskraft-tidstariff` |
| Kraftringen | Houses and apartments | `se/kraftringen` |
| Tekniska verken, Linköping | Standard | `se/tekniska-verken-linkoping-standard` |
| Tekniska verken, Linköping | Alternativ | `se/tekniska-verken-linkoping-alternativ` |
| Tekniska verken, Linköping | Apartment | `se/tekniska-verken-linkoping-apartment` |

E.ON has its own prices in each of its three areas: Syd (Skåne, Blekinge, Halland, Småland, eastern Götaland and Örebro), Stockholm (Danderyd, Enköping, Åkersberga and southern Uppland) and Nord (Kramfors, Hammarstrand, Medelpad and Ådalen).

| E.ON | 20 A to 63 A, and 16 A over 8,000 kWh a year | 16 A up to 8,000 kWh a year | Apartment 16 A | Apartment 20 A or 25 A |
| --- | --- | --- | --- | --- |
| Syd | `se/eon-syd-16-63a` | `se/eon-syd-16a-8000-kwh` | `se/eon-syd-apartment-16a` | `se/eon-syd-apartment-20-25a` |
| Stockholm | `se/eon-stockholm-16-63a` | `se/eon-stockholm-16a-8000-kwh` | `se/eon-stockholm-apartment-16a` | `se/eon-stockholm-apartment-20-25a` |
| Nord | `se/eon-nord-16-63a` | `se/eon-nord-16a-8000-kwh` | `se/eon-nord-apartment-16a` | `se/eon-nord-apartment-20-25a` |

An apartment here is a home in a building where at least three share one connection.

- **Tidstariff,** with Vattenfall Eldistribution and Öresundskraft: dearer on weekdays from 06:00 to 22:00 from November to March, cheaper the rest of the time. Vattenfall counts 1 and 6 January, Good Friday, Easter Monday and 24, 25, 26 and 31 December as weekend days. Öresundskraft names no holidays and takes no new customers on it.
- **Tekniska verken Alternativ:** night from 23:00 to 06:00 and day the rest, every day of the year.
- **All the others:** the same fee every hour.

The hours follow the clock, summer time included.

The plans leave out the monthly fees and what's the same every hour: the energy tax (45 öre per kWh with VAT in 2026, 12 öre less in some northern municipalities) and the authority fees. Göteborg Energi's House and Tidsindelad and Tekniska verken's Standard and Alternativ also charge for your highest power in kW, Tidsindelad only on weekdays from 07:00 to 20:00 from November to March. The board doesn't count that yet ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)): [Grid fees](../README.md#grid-fees) says what to do until it does. Öresundskraft adds 5.57% of the month's average market price in SE4 to its fee per kWh, and Kraftringen 5% of each hour's: the plans leave them out, as neither changes which hours are cheapest.

If your company or subscription isn't here, like a subscription over 63 A, write a custom plan, as [Custom plan](../README.md#custom-plan) says.

Each network company sets its own prices, within a limit set by the Energy Markets Inspectorate (Ei). Most change them on 1 January, some also during the year. E.ON's three areas move to one price list by 1 January 2028.

[Custom plan](../README.md#custom-plan) says how to change the plans' prices for yourself: download a plan's file from this folder, change it and upload it with **Upload custom plan**.
