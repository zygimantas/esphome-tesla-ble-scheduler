# Grid fees

The `grid:` part of `config.yaml` adds your grid fees to the market prices, so the board compares what you really pay. The example comes with ESO's 2026 Standartinis plan with four zones; [ESO plans](eso.md) has the others ready to paste. For any other grid operator, write your plan the same way:

- `prices`: the fee per kWh, with VAT, for each zone, named by a letter you choose, in your `currency` (your market area's own unless you set it).
- `hours`: the zone of each hour from 00:00 to 23:00, one letter per hour, on a `workday`, a `weekend` day and a public `holiday`. Leave `holiday` out when you list no holidays.
- `holidays`: public holidays as `MM-DD`, or days around Easter as `easter`, `easter+1` or `easter-2`. The `holiday` hours apply on them.
- `clock: winter`: the plan's clock stays on winter time all year, as some plans do: the zone hours, and the days and dates that pick them. Leave it out when the plan follows the clock.
- `winter` under `hours`: other zone hours for part of the year, between `from` and `to` as `MM-DD` (the range may cross New Year), for the `workday`, `weekend` or `holiday` you give; the others keep the year-round hours.

`market:` says where the market prices come from: the `area`, and the `vat` on the price itself (see Settings in the README).

## A fixed price

Leave `market:` out: nothing is downloaded, and each zone's price is your whole price per kWh, the supplier's rate and the grid fee together, with VAT. One price for every hour:

```yaml
  grid:
    hours:
      weekend: xxxxxxxxxxxxxxxxxxxxxxxx
      workday: xxxxxxxxxxxxxxxxxxxxxxxx
    prices:
      x: 0.24
```

All hours then cost the same, so the board charges at once. With day and night prices, give each its zone, and the board charges in the cheap one.

A plan with two zones, cheap at night and all weekend, on winter time all year:

```yaml
charging:
  grid:
    clock: winter
    hours:
      weekend: nnnnnnnnnnnnnnnnnnnnnnnn
      workday: nnnnnnnddddddddddddddddn
    prices:
      n: 0.07139
      d: 0.12947
```

Here `n` runs from 00:00 to 07:00 and from 23:00 on a workday, `d` from 07:00 to 23:00, and the weekend is all `n`.

A plan whose weekdays from 06:00 to 22:00 cost more from November to March, as some Swedish and Finnish plans do:

```yaml
charging:
  grid:
    hours:
      weekend: llllllllllllllllllllllll
      workday: llllllllllllllllllllllll
      winter:
        from: 11-01
        to: 03-31
        workday: llllllhhhhhhhhhhhhhhhhll
    prices:
      l: 0.03
      h: 0.08
```

If something is wrong, the install stops and says what: a letter in `hours` without a price, a price no hour uses, or a date that doesn't exist.
