# Dutch grid fees

There's no grid plan for the Netherlands, and none is needed: no Dutch grid fee changes with the hour. Every grid operator (Liander, Enexis, Stedin, Coteq, RENDO and Westland Infra) charges a home a fixed amount a year, set by the size of its connection, like 3 x 25 A, however much it uses and whenever. Your supplier bills it every month. The market price alone makes the cheap hours, so leave `tariff:` out of your settings file:

```yaml
market:
  area: NL
  vat: 0.21
```

On the page, choose **Dynamic (spot, exchange)** with a contract whose price follows the market by the hour.

These are left out of every price the board shows, as they're the same in every hour:

- the grid operator's fees for the connection, the transport and the meter;
- the energy tax (energiebelasting) per kWh, and its yearly tax reduction.

With one fixed price, all hours cost the same, so the board charges at once. With a normal and a cheaper off-peak (dal) price, leave `market:` out and write your own tariff with both, each your whole price per kWh with VAT, as [Tariff](tariff.md#a-fixed-price) says. Off-peak is usually from 23:00 to 07:00 on workdays, in Noord-Brabant and Limburg from 21:00, and all day at weekends and on public holidays. Your contract says if yours differ. With your own prices:

```yaml
tariff:
  calendar:
    jan-dec:
      mon-fri: low 07:00 normal 23:00 low
      sat-sun: low
  exceptions:
    12-25: low
    12-26: low
  rates:
    normal: 0.27
    low: 0.25
```

In Noord-Brabant and Limburg, write 21:00 for 23:00. Add the other public holidays under `exceptions` the same way: 1 January, 27 April (King's Day), and this year's Easter Monday, Ascension Day and Whit Monday.

The ACM, the Dutch regulator, sets each grid operator's fees every year, from 1 January. The operators have proposed fees per kWh that change with the hour, cheaper at night and in the middle of the day and dearest in the evening, from 2029 at the earliest. The ACM expects to decide at the end of 2026. Plans for them come here once their prices are set.

[Tariff](tariff.md) says how to write your own tariff.
