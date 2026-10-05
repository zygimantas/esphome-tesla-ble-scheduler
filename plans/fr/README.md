# France's grid fees

The grid fees (TURPE) that every home in France pays per kWh. The CRE, the energy regulator, sets them, the same with Enedis and with every local grid operator, and your supplier bills them in its price. Your home has one of the fees' options, and your supplier can tell you which.

There are no plans for France: most homes' off-peak hours are set for each address, which a plan can't hold. Finish the setup without one, then write your own tariff with the prices below and upload it under **Change settings**, as [Tariff](../../docs/tariff.md) says.

## Off-peak hours

Most homes have a Linky meter and the option CU4 or MU4. Both have 8 off-peak hours a day (heures creuses), which the grid operator sets for each address, and a dearer high season from November to March. From November 2025 to October 2027, Enedis moves the off-peak hours of about 11 million homes: at least 5 hours in a row between 23:00 and 07:00, and the other 3 may be between 11:00 and 17:00. From December 2026, many get other hours in summer than in winter. Your supplier tells you a month before.

Write your own tariff with your off-peak hours, which your bill gives, with these prices in euros with VAT per kWh, from 1 August 2026 to 31 July 2027:

| Hours | CU4 | MU4 |
| --- | --- | --- |
| Peak, November to March | 0.09264 | 0.08652 |
| Off-peak, November to March | 0.04908 | 0.04608 |
| Peak, April to October | 0.02052 | 0.01992 |
| Off-peak, April to October | 0.01440 | 0.01368 |

With CU4 and off-peak hours from 22:00 to 06:00:

```yaml
market:
  area: FR
  vat: 0.20
tariff:
  calendar:
    nov-mar:
      mon-sun: hch 06:00 hph 22:00 hch
    apr-oct:
      mon-sun: hcb 06:00 hpb 22:00 hcb
  rates:
    hph: 0.09264
    hch: 0.04908
    hpb: 0.02052
    hcb: 0.01440
```

If your off-peak hours change with the season, give each season its own. A meter that isn't a Linky meter and has off-peak hours has the option MUDT: 0.06108 at peak and 0.04332 off-peak, all year.

## One price for every hour

The options LU (long use, rare in homes) and CU (short use, for meters that aren't Linky meters) have one price for every hour: 0.01548 and 0.05988 euros with VAT per kWh. They don't change which hours are cheapest, only the costs the board shows:

```yaml
tariff:
  calendar:
    jan-dec:
      mon-sun: flat
  rates:
    flat: 0.01548
```

## Your supplier's price

With a contract that follows the market by the hour, choose **Dynamic (spot, exchange)** on the page, with your own tariff. Most fixed prices in France already have the grid fees in: write your whole price per kWh as your own tariff, with your off-peak hours if your contract has them, as [Tariff](../../docs/tariff.md#a-fixed-price) says.

## Left out

The prices above count the fee per kWh. The fees' fixed parts, for management, metering and your subscribed power in kVA, the contribution on top of them (CTA), and the excise on electricity, the same in every hour, are left out, as they don't change which hours are cheapest.

The CRE sets the prices and changes them every 1 August. [Tariff](../../docs/tariff.md) says how to write your own tariff.
