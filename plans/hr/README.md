# HEP ODS plans

The network fees of HEP ODS, Croatia's only distribution operator, for homes, with prices with VAT. Your bill names your tariff model (tarifni model). Name yours under `tariff:` in your settings file, with Croatia's market area and VAT:

```yaml
market:
  area: HR
  vat: 0.13
tariff:
  plan: hr/hep-ods-bijeli
```

| Tariff model | Plan |
| --- | --- |
| Plavi | `hr/hep-ods-plavi` |
| Bijeli | `hr/hep-ods-bijeli` |
| Crveni | `hr/hep-ods-crveni` |

- **Plavi:** the same fee every hour.
- **Bijeli:** the lower rate (NT) from 21:00 to 07:00 every day, weekends and public holidays too, and the higher rate (VT) the rest.
- **Crveni:** for homes connected with more than 22 kW, with Bijeli's hours. Its fee per kW of the month's highest power in the higher rate's hours isn't counted ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)): charging in those hours can raise it.

The hours are on winter time all year, as the meters don't change their clocks, so in summer they come an hour later on the clock: the lower rate from 22:00 to 08:00.

Each fee is HEP ODS's distribution fee and HOPS's transmission fee together. The plans leave out the monthly metering point fee and the renewable energy and cogeneration fee, which is the same every hour.

Crni isn't here: it's for storage heaters and water heaters on a meter of their own, which are switched on by remote control at times set for them. Where no plan fits, write your own tariff, as [Your own tariff](../README.md#your-own-tariff) says.

Most homes pay their supplier a fixed price, without the grid fees. With one price for every hour, choose Fixed on the page and give it as the supplier's part. With a day (VT) and a night (NT) price, leave `market:` out and write your whole prices instead, the supplier's and the grid fee together, with VAT, as the plan's rates:

```yaml
tariff:
  plan: hr/hep-ods-bijeli
  rates:
    day: 0.18
    night: 0.09
```

HERA, the energy regulator, sets the fees, usually from 1 January. Under a government decree, the operators won't ask for new ones before April 2027. [Your own changes](../README.md#your-own-changes) says how to change the plans' prices for yourself.
