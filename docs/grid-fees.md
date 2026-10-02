# Grid fees

The `grid:` part of `config.yaml` adds your grid fees to the market prices, so the board compares what you really pay. The example comes with ESO's 2026 Standartinis plan with four zones; [ESO plans](eso.md) has the others ready to paste. For any other grid operator, write your plan the same way:

- `prices`: the fee per kWh, with VAT, for each zone, named by a letter you choose, in your `currency` (your market area's own unless you set it).
- `hours`: the zones of a day: one letter for the whole day, or a letter for each hour from 00:00 (24 letters), half-hour (48) or quarter-hour (96). Name the days `mon` to `sun`, or ranges like `mon-fri` and `sat-sun`, so that each day of the week is named once. `holiday` gives public holidays their own zones; without it, they take Sunday's.
- `holidays`: public holidays as `MM-DD`, or days around Easter as `easter`, `easter+1` or `easter-2`.
- `clock: winter`: the plan's clock stays on winter time all year, as some plans do: the zone hours, and the days and dates that pick them. Leave it out when the plan follows the clock.
- `seasons`: other zones for parts of the year. Each is named by its first and last date, like `11-01 to 03-31` (a season may cross New Year), and gives the zones of the days that change, as `hours` does; the other days keep the year-round zones. Seasons can't overlap. For other prices in a season, give its days zones of their own.

`market:` says where the market prices come from: the `area`, and the `vat` on the price itself (see Settings in the README).

## Examples

A plan with two zones, cheap at night and all weekend, on winter time all year:

```yaml
charging:
  grid:
    clock: winter
    hours:
      mon-fri: nnnnnnnddddddddddddddddn
      sat-sun: n
    prices:
      n: 0.07139
      d: 0.12947
```

Here `n` runs from 00:00 to 07:00 and from 23:00 on weekdays, `d` from 07:00 to 23:00, and the weekend is all `n`.

A fee that is dearer from November to March, Monday to Saturday from 07:00 to 22:00, like Finland's seasonal grid fee:

```yaml
charging:
  grid:
    hours:
      mon-sun: l
    prices:
      l: 0.03
      h: 0.08
    seasons:
      11-01 to 03-31:
        mon-sat: lllllllhhhhhhhhhhhhhhhll
```

## A fixed price

Leave `market:` out: nothing is downloaded, and each zone's price is your whole price per kWh, the supplier's rate and the grid fee together, with VAT. One price for every hour:

```yaml
  grid:
    hours:
      mon-sun: x
    prices:
      x: 0.24
```

All hours then cost the same, so the board charges at once. With day and night prices, give each its zone, and the board charges in the cheap one. A night rate that starts on the half-hour, like Octopus Go's from 00:30 to 05:30 in the UK, takes 48 letters, with your rates:

```yaml
charging:
  currency: GBP
  grid:
    hours:
      mon-sun: nccccccccccnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
    prices:
      c: 0.085
      n: 0.245
```

If something is wrong, the install stops and says what: a letter without a price, a price no hour uses, a day named twice or not at all, seasons that overlap, or a date that doesn't exist.
