# Danish plans

The household grid tariff, Nettarif C, of Denmark's eight biggest grid companies, with prices with VAT, in kroner. Name yours under `tariff:` in your settings file, with your market area and Denmark's VAT, which make kroner the currency:

```yaml
market:
  area: DK2
  vat: 0.25
tariff:
  plan: dk/radius-nettarif-c
```

| Grid company | `market: area` | Plan |
| --- | --- | --- |
| Radius Elnet | `DK2` | `dk/radius-nettarif-c` |
| N1 | `DK1` | `dk/n1-nettarif-c` |
| Cerius | `DK2` | `dk/cerius-nettarif-c` |
| Konstant | `DK1` | `dk/konstant-nettarif-c` |
| Vores Elnet | `DK1` | `dk/vores-elnet-nettarif-c` |
| TREFOR El-net | `DK1` | `dk/trefor-nettarif-c` |
| Dinel | `DK1` | `dk/dinel-nettarif-c` |
| Nord Energi Net | `DK1` | `dk/nord-energi-nettarif-c` |

Your bill names your grid company (netselskab), or [find it by your address](https://elnet.dk/nettilslutning/find-netselskab).

Every plan has the same hours, every day, weekends and public holidays included:

| Hours | Rate |
| --- | --- |
| 00:00 - 06:00 | Low |
| 06:00 - 17:00 and 21:00 - 24:00 | High |
| 17:00 - 21:00 | Peak |

High and peak cost less from April to September than from October to March. The hours follow the clock, summer time included. Konstant's prices are after the discount (rabat) it gives on the tariff.

The plans leave out the yearly subscription and the charges that are the same every hour: Energinet's grid and system tariffs and the electricity tax (elafgift). What homes with solar panels pay or get for their own power is left out too.

Most other grid companies have the same hours. Name one of these plans and give your company's prices with VAT under `rates:`, for `low`, `summer-high`, `summer-peak`, `winter-high` and `winter-peak`, as [Your own changes](../README.md#your-own-changes) says. Where the hours differ, write your own tariff, as [Your own tariff](../README.md#your-own-tariff) says.

Each grid company sets its own prices, within a revenue cap the Danish Utility Regulator (Forsyningstilsynet) sets. Most change them on 1 January, some also during the year.

[Your own changes](../README.md#your-own-changes) says how to change the plans' prices for yourself.
