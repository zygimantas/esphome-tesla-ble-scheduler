# Norwegian plans

The network fees for households (nettleie) of Norway's biggest network companies, which serve more than three homes in four, with prices with VAT, in kroner. Your network company is the one that owns the grid where you live, and your bill names it. Name its plan under `tariff:` in your settings file, with your price area and VAT:

```yaml
market:
  area: NO1
  vat: 0.25
tariff:
  plan: no/elvia-households
```

| Network company | Mostly in | Plan |
| --- | --- | --- |
| Elvia | Oslo, Akershus, Innlandet, Østfold | `no/elvia-households` |
| Glitre Nett | Agder, Buskerud, Akershus, Østfold | `no/glitre-nett-households` |
| BKK | Vestland | `no/bkk-households` |
| Lede | Vestfold, Telemark | `no/lede-households` |
| Tensio TS | Southern Trøndelag | `no/tensio-ts-households` |
| Tensio TN | Northern Trøndelag | `no/tensio-tn-households` |
| Lnett | Rogaland | `no/lnett-households` |
| Arva | Troms, Nordland | `no/arva-households` |
| Fagne | Rogaland, Vestland | `no/fagne-households` |

- **Elvia, BKK and Lnett:** day from 06:00 to 22:00 on workdays, night the rest, and all day at weekends and on public holidays.
- **Fagne:** the same, but public holidays count as workdays, as its price list names only weekends.
- **Glitre Nett, Tensio and Arva:** day from 06:00 to 22:00 and night from 22:00 to 06:00, every day.
- **Lede:** the same fee every hour.

The public holidays are 1 January, Maundy Thursday, Good Friday, Easter Monday, 1 May, Ascension Day, 17 May, Whit Monday and 25 and 26 December. The hours follow the clock, summer time included.

Homes in Nordland, Troms and Finnmark pay no VAT on electricity: set `vat: 0`. Arva's plan has no VAT in it, as all its homes are there.

The plans count the energy fee (energiledd) per kWh. They leave out the capacity step (kapasitetsledd), a monthly fee set by your three highest hours, which the board doesn't count yet ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)), and the charges that are the same every hour: the electricity tax (elavgift) and the Enova fee, which some companies print inside their energy fee. The electricity subsidy (strømstøtte), which pays back 90% of the hourly market price above 77 øre/kWh without VAT, isn't counted either.

With Norgespris, your power costs 50 øre/kWh with VAT whatever the market does, or 40 øre where there's no VAT, until the end of 2026. Choose **Fixed** on the page, with that price and your supplier's margin together as the supplier's part: the plan's night hours then make the cheap hours.

For another network company, write your own tariff with its day and night fees, as [Tariff](../../docs/tariff.md) says.

Each network company sets its own fees, within the income the regulator, RME, allows it. They change every January and often in the middle of the year too, at 14 days' notice. [Tariff](../../docs/tariff.md) says how to change the plans' prices for yourself.
