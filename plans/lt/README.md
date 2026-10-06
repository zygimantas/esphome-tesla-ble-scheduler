# ESO plans

ESO's household plans for Lithuania, with prices with VAT. Your supplier bills ESO's fee unchanged, and its bill names your plan. Name yours under `tariff:` in your settings file, and leave `market:` as it is:

```yaml
tariff:
  plan: lt/eso-standartinis-4-zones
```

| Plan | One zone | Two zones | Four zones |
| --- | --- | --- | --- |
| Standartinis | `lt/eso-standartinis-1-zone` | `lt/eso-standartinis-2-zones` | `lt/eso-standartinis-4-zones` |
| Efektyvus | `lt/eso-efektyvus-1-zone` | `lt/eso-efektyvus-2-zones` | `lt/eso-efektyvus-4-zones` |
| Namai | `lt/eso-namai-1-zone` | `lt/eso-namai-2-zones` | |

- **One zone:** the same fee every hour.
- **Two zones:** night from 23:00 to 07:00 and all weekend, day the rest, on winter time all year. Public holidays count as workdays.
- **Four zones:** on a workday, night from 22:00 to 05:00, morning from 05:00 to 07:00, day from 07:00 to 17:00 and evening from 17:00 to 22:00. At weekends and on public holidays, night from 22:00 to 07:00 and day the rest. The hours follow the clock, summer time included.

The plans leave out Efektyvus's monthly fee per kW of your permitted power and Namai's monthly fee, and the charges per kWh that are the same every hour, like the public service obligations (VIAP).

VERT, the regulator, approves ESO's prices for each year, from 1 January. [Your own changes](../README.md#your-own-changes) says how to change the plans' prices for yourself.
