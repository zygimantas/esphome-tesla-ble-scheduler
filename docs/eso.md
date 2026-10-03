# ESO price lists

ESO's household plans for Lithuania, as price lists with prices with VAT. Name yours under `grid:` in `config.yaml`, and leave `market:` as it is:

```yaml
  grid:
    pricelist: lt/eso-standartinis-4-zones
```

| Plan | One zone | Two zones | Four zones |
| --- | --- | --- | --- |
| Standartinis | `lt/eso-standartinis-1-zone` | `lt/eso-standartinis-2-zones` | `lt/eso-standartinis-4-zones` |
| Efektyvus | `lt/eso-efektyvus-1-zone` | `lt/eso-efektyvus-2-zones` | `lt/eso-efektyvus-4-zones` |
| Namai | `lt/eso-namai-1-zone` | `lt/eso-namai-2-zones` | |

- **One zone:** the same fee every hour.
- **Two zones:** night from 23:00 to 07:00 and all weekend, day the rest, on winter time all year. Public holidays count as workdays.
- **Four zones:** on a workday, night from 22:00 to 05:00, morning from 05:00 to 07:00, day from 07:00 to 17:00 and evening from 17:00 to 22:00. At weekends and on public holidays, night from 22:00 to 07:00 and day the rest. The hours follow the clock, summer time included.

The prices are in the lists in [pricelists/lt](../pricelists/lt). [Grid fees](grid-fees.md) says how to change them for yourself.
