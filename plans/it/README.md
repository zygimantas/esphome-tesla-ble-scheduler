# Italy's TD plan

The charge per kWh of the TD tariff, which every household in Italy pays for the grid, with prices with VAT. ARERA, the regulator, sets it for the whole country, so every grid operator bills the same. Name it under `tariff:` in your settings file:

```yaml
market:
  area: IT-NORTH
  vat: 0.10
tariff:
  plan: it/td
```

| Plan | Operators |
| --- | --- |
| `it/td` | All of them: e-distribuzione, Unareti, Areti, Ireti and the rest |

The fee is the same in every hour of every day, for residents and non-residents alike. It's the part of your bill's transport and meter costs (spesa per il trasporto e la gestione del contatore) that comes per kWh: the TD tariff's own charge and the UC3 and UC6 components.

The F1, F2 and F3 hours on your bill are your supplier's, not the grid's. With a price that follows the market by the hour, choose **Dynamic (spot, exchange)** on the page. With one fixed price for every hour, anywhere in Italy, choose Fixed and give your supplier's price per kWh, without the grid fees, as **Supplier's part**. With a fixed price for F1 and another for F2 and F3, write your own plan, as [A fixed price](../README.md#a-fixed-price) says, with your whole price in each: F1 is 08:00 - 19:00 from Monday to Friday, except national holidays.

The plan leaves out the fixed fee, the fee per kW of your contracted power and the charges that are the same every hour: the system charges (oneri di sistema, ASOS and ARIM) and the excise duty. The board doesn't count your contracted power, often 3 kW: set the car's charging current low enough for it, in the car or the Tesla app, and `tesla_charging_kw` to match.

ARERA changes the TD tariff every 1 January, and may change UC3 and UC6 every quarter. [Your own changes](../README.md#your-own-changes) says how to change the plan's prices for yourself.
