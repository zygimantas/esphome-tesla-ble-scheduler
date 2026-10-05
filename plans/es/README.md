# Spain's 2.0TD

The access tolls and charges (peajes y cargos) of tariff 2.0TD, which every household up to 15 kW pays per kWh, whoever its supplier, with prices with VAT. Name it under `tariff:` in your settings file:

```yaml
market:
  area: ES
  vat: 0.21
tariff:
  plan: es/2-0td
```

| Hours | Monday to Friday | Saturday, Sunday and national holidays |
| --- | --- | --- |
| 10:00 - 14:00 and 18:00 - 22:00 | P1, peak | P3 |
| 08:00 - 10:00, 14:00 - 18:00 and 22:00 - 24:00 | P2, flat | P3 |
| 00:00 - 08:00 | P3, valley | P3 |

The national holidays are the ones with a date of their own that regions can't move: 1 and 6 January, 1 May, 15 August, 12 October, 1 November and 6, 8 and 25 December. The hours follow the clock, summer time included, in the Canary Islands too. Ceuta and Melilla have other hours: write your own tariff for them, as [Your own tariff](../README.md#your-own-tariff) says.

With PVPC, or another price that follows the market by the hour, choose **Dynamic (spot, exchange)** on the page. With your supplier's own price for each period, choose Fixed: the plan's periods then make the cheap hours.

The plan counts the tolls and charges per kWh. The power term, the electricity tax, which adds the same share to every hour, and the meter's rent are left out, as they don't change which hours are cheapest.

The CNMC sets the tolls and the ministry the charges, every January. [Your own changes](../README.md#your-own-changes) says how to change the plans' prices for yourself.
