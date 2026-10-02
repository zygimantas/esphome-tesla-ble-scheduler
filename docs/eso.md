# ESO plans

ESO's household plans for Lithuania as `grid:` blocks for `config.yaml`, with 2026 prices in EUR/kWh with VAT. Replace the `grid:` block under `charging:` with the one for your plan and leave `prices:` as it is. Check the current prices on [ESO's website](https://www.eso.lt) first: they change at least once a year.

The letters are `n` for night, `m` for morning, `d` for day and `e` for evening. [Grid fees](grid-fees.md) explains the format.

## One zone

The same fee every hour.

Standartinis:

```yaml
  grid:
    fees:
      a: 0.11132
    hours:
      weekend: aaaaaaaaaaaaaaaaaaaaaaaa
      workday: aaaaaaaaaaaaaaaaaaaaaaaa
```

Efektyvus:

```yaml
  grid:
    fees:
      a: 0.08833
    hours:
      weekend: aaaaaaaaaaaaaaaaaaaaaaaa
      workday: aaaaaaaaaaaaaaaaaaaaaaaa
```

Namai:

```yaml
  grid:
    fees:
      a: 0.09559
    hours:
      weekend: aaaaaaaaaaaaaaaaaaaaaaaa
      workday: aaaaaaaaaaaaaaaaaaaaaaaa
```

## Two zones

Night from 23:00 to 07:00 and all weekend, day the rest, on winter time all year.

Standartinis:

```yaml
  grid:
    clock: winter
    fees:
      n: 0.07139
      d: 0.12947
    hours:
      weekend: nnnnnnnnnnnnnnnnnnnnnnnn
      workday: nnnnnnnddddddddddddddddn
```

Efektyvus:

```yaml
  grid:
    clock: winter
    fees:
      n: 0.05687
      d: 0.10164
    hours:
      weekend: nnnnnnnnnnnnnnnnnnnnnnnn
      workday: nnnnnnnddddddddddddddddn
```

Namai:

```yaml
  grid:
    clock: winter
    fees:
      n: 0.06171
      d: 0.11011
    hours:
      weekend: nnnnnnnnnnnnnnnnnnnnnnnn
      workday: nnnnnnnddddddddddddddddn
```

## Four zones

On a workday: night from 22:00 to 05:00, morning from 05:00 to 07:00, day from 07:00 to 17:00, evening from 17:00 to 22:00. At weekends and on public holidays: night from 22:00 to 07:00, day the rest. The hours follow the clock, summer time included. Namai has no four-zone plan.

Standartinis (the plan in `config.example.yaml`):

```yaml
  grid:
    fees:
      n: 0.06292
      m: 0.08349
      d: 0.10406
      e: 0.14641
    holidays:
      - 01-01
      - 02-16
      - 03-11
      - easter
      - easter+1
      - 05-01
      - 06-24
      - 07-06
      - 08-15
      - 11-01
      - 11-02
      - 12-24
      - 12-25
      - 12-26
    hours:
      holiday: nnnnnnndddddddddddddddnn
      weekend: nnnnnnndddddddddddddddnn
      workday: nnnnnmmddddddddddeeeeenn
```

Efektyvus:

```yaml
  grid:
    fees:
      n: 0.05082
      m: 0.06534
      d: 0.08228
      e: 0.11374
    holidays:
      - 01-01
      - 02-16
      - 03-11
      - easter
      - easter+1
      - 05-01
      - 06-24
      - 07-06
      - 08-15
      - 11-01
      - 11-02
      - 12-24
      - 12-25
      - 12-26
    hours:
      holiday: nnnnnnndddddddddddddddnn
      weekend: nnnnnnndddddddddddddddnn
      workday: nnnnnmmddddddddddeeeeenn
```
