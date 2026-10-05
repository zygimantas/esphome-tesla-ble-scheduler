# Belgian plans

The household plans of Belgium's distribution operators, with prices with VAT: Fluvius in Flanders, ORES and RESA in Wallonia, and Sibelga in Brussels. Your bill names your operator, and in Flanders your Fluvius area. Name your plan under `tariff:` in your settings file, with Belgium's market area and VAT:

```yaml
market:
  area: BE
  vat: 0.06
tariff:
  plan: be/ores-impact
```

## Flanders

| Fluvius area | Plan |
| --- | --- |
| Antwerpen | `be/fluvius-antwerpen` |
| Halle-Vilvoorde | `be/fluvius-halle-vilvoorde` |
| Imewo | `be/fluvius-imewo` |
| Kempen | `be/fluvius-kempen` |
| Limburg | `be/fluvius-limburg` |
| Midden-Vlaanderen | `be/fluvius-midden-vlaanderen` |
| West | `be/fluvius-west` |
| Zenne-Dijle | `be/fluvius-zenne-dijle` |

Fluvius charges the same fee per kWh every hour, with a day and night meter too. The plans are for a digital meter. With an analog meter, Fluvius charges more per kWh and a fixed yearly amount instead of a fee on your peaks: write your own tariff, as [Tariff](tariff.md) says.

## Wallonia and Brussels

| Operator | Monohoraire | Bihoraire | Impact |
| --- | --- | --- | --- |
| ORES | `be/ores-monohoraire` | `be/ores-bihoraire` | `be/ores-impact` |
| RESA | `be/resa-monohoraire` | `be/resa-bihoraire` | `be/resa-impact` |
| Sibelga, Brussels | `be/sibelga-monohoraire` | `be/sibelga-bihoraire` | |

Monohoraire is the same fee every hour. In Wallonia, Bihoraire and Impact have these hours every day of the week, public holidays included:

| Hours | Bihoraire | Impact |
| --- | --- | --- |
| 01:00 - 07:00 | off-peak | ECO |
| 07:00 - 11:00 | peak | MEDIUM |
| 11:00 - 17:00 | off-peak | ECO |
| 17:00 - 22:00 | peak | PIC |
| 22:00 - 01:00 | off-peak | MEDIUM |

Impact suits charging a car: ECO costs a fifth of PIC. It needs a smart meter with its communication on: ask your supplier for it.

In Brussels, Bihoraire is off-peak from 22:00 to 07:00 on weekdays, and all day at weekends and on public holidays. Sibelga bills a smart meter's fee as Bihoraire by default.

The hours follow the clock, summer time included.

## Left out

The plans count the fee for using the network per kWh. They leave out the yearly fees, Flanders' capacity tariff on your highest quarter-hour each month, which the board doesn't count yet ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)), and the charges that are the same every hour: the public service obligations, the surcharges (taxes the operator collects), Wallonia's regulatory balances, the transmission fee in Wallonia and Brussels (transport on the bill; in Flanders it's part of Fluvius's fee), and the federal excise and energy contribution.

The small Walloon operators AIEG, AIESH and REW, and exclusive night meters, which heat on a circuit of their own, have no plans here. Write your own tariff, as [Tariff](tariff.md) says.

The regulators approve the prices, which change on 1 January: the Vlaamse Nutsregulator for Fluvius, the CWaPE for ORES and RESA, and Brugel for Sibelga. In Brussels, Brugel brings a fee by your connection's power in 2028 and three periods a day in 2030.

The prices are in the plans in [plans/be](../plans/be). [Tariff](tariff.md) says how to change them for yourself.
