# Bulgarian plans

The network fees that Bulgaria's three big distribution operators charge households, with prices with VAT, in euros. Name yours under `tariff:` in your settings file, with Bulgaria's market area and VAT:

```yaml
market:
  area: BG
  vat: 0.2
tariff:
  plan: bg/erm-zapad-households
```

| Operator | Where | Plan |
| --- | --- | --- |
| Elektrorazpredelitelni Mrezhi Zapad (ERM Zapad) | The west, with Sofia | `bg/erm-zapad-households` |
| Elektrorazpredelenie Yug (ER Yug) | The south-east, with Plovdiv and Burgas | `bg/er-yug-households` |
| Elektrorazpredelenie Sever (ERP Sever) | The north-east, with Varna and Ruse | `bg/erp-sever-households` |

Your operator is the one for where you live. Each plan is one fee for every hour: the operator's access fee and its low voltage transmission fee for households, which don't change with the hour. With a price that follows the exchange by the hour, choose **Dynamic (spot, exchange)** on the page.

The plans leave out what is the same every hour and isn't the operator's own: the access and transmission fees of ESO, the transmission operator, 0.98 cents per kWh with VAT everywhere, and the obligations to society fee, which is 0 now.

## Day and night meters

With the regulated price and a day and night meter, the night is cheaper for the electricity, not for the network fees. The hours are the same in the whole country: the night is from 22:00 to 06:00 from November to March, and from 23:00 to 07:00 from April to October. Leave `market:` out and write your own tariff with the whole price of each rate, network fees and VAT included, from your supplier's price list, as [Tariff](tariff.md) says. These are the prices in the west from July 2026:

```yaml
tariff:
  calendar:
    apr-oct:
      mon-sun: night 07:00 day 23:00 night
    nov-mar:
      mon-sun: night 06:00 day 22:00 night
  rates:
    day: 0.15425
    night: 0.09192
```

In Golden Sands, whose own small operator has no plan here, write your own tariff too.

The Energy and Water Regulatory Commission (EWRC) sets the network fees every year from 1 July, the same whoever your supplier is. The prices are in the plans in [plans/bg](../plans/bg). [Tariff](tariff.md) says how to change them for yourself.
