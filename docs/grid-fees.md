# Grid fees

The `grid:` part of `config.yaml` adds your grid fees to the Nord Pool prices, so the board compares what you really pay. The example comes with ESO's 2026 Standartinis plan with four zones; [ESO plans](eso.md) has the others ready to paste. For any other grid operator, write your plan the same way:

- `fees`: the fee per kWh, with VAT, for each zone, named by a letter you choose.
- `hours`: the zone of each hour from 00:00 to 23:00, one letter per hour, on a `workday`, a `weekend` day and a public `holiday`. Leave `holiday` out when you list no holidays.
- `holidays`: public holidays as `MM-DD`, or days around Easter as `easter`, `easter+1` or `easter-2`. The `holiday` hours apply on them.
- `clock: winter`: the zone hours stay on winter time all year, as some plans do. Leave it out when they follow the clock.
- `winter` under `hours`: other zone hours for part of the year, between `from` and `to` as `MM-DD` (the range may cross New Year), for the `workday`, `weekend` or `holiday` you give; the others keep the year-round hours.

The VAT on the Nord Pool price itself is `vat` under `nordpool:`, next to `area` (see Settings in the README).

A plan with two zones, cheap at night and all weekend, on winter time all year:

```yaml
charging:
  grid:
    clock: winter
    fees:
      n: 0.07139
      d: 0.12947
    hours:
      weekend: nnnnnnnnnnnnnnnnnnnnnnnn
      workday: nnnnnnnddddddddddddddddn
```

Here `n` runs from 00:00 to 07:00 and from 23:00 on a workday, `d` from 07:00 to 23:00, and the weekend is all `n`.

A plan whose weekdays from 06:00 to 22:00 cost more from November to March, as some Swedish and Finnish plans do:

```yaml
charging:
  grid:
    fees:
      l: 0.03
      h: 0.08
    hours:
      weekend: llllllllllllllllllllllll
      workday: llllllllllllllllllllllll
      winter:
        from: 11-01
        to: 03-31
        workday: llllllhhhhhhhhhhhhhhhhll
```

If something is wrong, the install stops and says what: a letter in `hours` without a fee, a fee no hour uses, or a date that doesn't exist.
