# Finnish plans

The household products of Finland's eight biggest network companies, with prices with VAT. Together they serve over half of Finland's homes. Your network company's bill names your product. Name it under `tariff:` in your settings file, with Finland's market area and VAT:

```yaml
market:
  area: FI
  vat: 0.255
tariff:
  plan: fi/caruna-yosiirto
```

| Network company | One rate | Day and night | Winter weekdays |
| --- | --- | --- | --- |
| Caruna | `fi/caruna-yleissiirto` | `fi/caruna-yosiirto` | `fi/caruna-kausisiirto` |
| Caruna Espoo | `fi/caruna-espoo-yleissiirto` | `fi/caruna-espoo-yosiirto` | `fi/caruna-espoo-kausisiirto` |
| Elenia | `fi/elenia-yleissiirto` | `fi/elenia-yosiirto` | `fi/elenia-vuodenaikasiirto` |
| Helen Sähköverkko | `fi/helen-yleissiirto` | `fi/helen-aikasiirto` | |
| Tampereen Energia Sähköverkko | `fi/tampereen-energia-yleissiirto` | `fi/tampereen-energia-aikasiirto` | `fi/tampereen-energia-kausisiirto` |
| Vantaan Energia Sähköverkot | `fi/vantaan-energia-yleissiirto`, `fi/vantaan-energia-kerrostalosiirto` | `fi/vantaan-energia-aikasiirto` | `fi/vantaan-energia-kausisiirto` |
| Oulun Energia Sähköverkko | `fi/oulun-energia-yleissahko` | `fi/oulun-energia-aikasahko` | `fi/oulun-energia-kausisahko` |
| Savon Voima Verkko | `fi/savon-voima-yleissahko` | | `fi/savon-voima-aikasahko` |

Caruna Espoo serves Espoo, Kauniainen, Kirkkonummi and the centre of Joensuu, and Caruna its other areas.

- **One rate** (Yleissiirto, Yleissähkö and Vantaan Energia's Kerrostalosiirto, for homes up to 3 x 25 A in buildings with ten meters or more): the same fee every hour.
- **Day and night** (Yösiirto, Aikasiirto and Oulun Energia's Aikasähkö): day from 07:00 to 22:00 and night from 22:00 to 07:00, every day.
- **Winter weekdays** (Kausisiirto, Vuodenaikasiirto, Kausisähkö and Savon Voima's Aikasähkö): dearer from 07:00 to 22:00, Monday to Saturday, from November to March, and cheaper the rest: at night, on Sundays and from April to October. Public holidays from Monday to Saturday count as weekdays, except with Oulun Energia, where they're cheaper all day.

The hours follow the clock, summer time included.

The plans leave out the monthly fees, and the electricity tax with its security of supply fee, which is the same every hour. Helen Sähköverkko's Aikasiirto also has a monthly fee per kW of the month's third highest hour, with night hours at 80%, which the board doesn't count yet ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)): a lower charging current keeps it down.

Not here: the products for temporary connections and for big connections with a fee per kW, and Savon Voima's Yösähkö and Kausisähkö, which ended on 1 November 2025. With another of Finland's network companies, write your own tariff, as [Tariff](../../docs/tariff.md) says: most use the same hours.

Each network company sets its own prices, which the Energy Authority (Energiavirasto) oversees, and may change them in any month, telling homes at least a month before.

[Tariff](../../docs/tariff.md) says how to change the plans' prices for yourself.
