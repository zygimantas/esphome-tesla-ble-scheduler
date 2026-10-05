# Portugal's access tariffs

The network access tariffs (tarifas de acesso às redes) that every home on the mainland up to 20.7 kVA pays per kWh, whoever its supplier, with prices with VAT. ERSE sets them, the same with E-REDES and with the small local grid operators. Your bill names your tariff option (opção tarifária) and, for bi-horária and tri-horária, your cycle (ciclo). Name yours under `tariff:` in your settings file:

```yaml
market:
  area: PT
  vat: 0.23
tariff:
  plan: pt/bi-horaria-diario
```

| Your tariff | Plan |
| --- | --- |
| Simples | `pt/simples` |
| Bi-horária, ciclo diário | `pt/bi-horaria-diario` |
| Bi-horária, ciclo semanal | `pt/bi-horaria-semanal` |
| Tri-horária, ciclo diário | `pt/tri-horaria-diario` |
| Tri-horária, ciclo semanal | `pt/tri-horaria-semanal` |

- **Simples:** the same fee every hour.
- **Bi-horária:** cheap off-peak hours (vazio) and dearer ones the rest of the time (fora de vazio). With the daily cycle, off-peak is from 22:00 to 08:00 every day. With the weekly cycle, off-peak is from 00:00 to 07:00 on weekdays, all day on Sunday, and on Saturday all day except 09:30 - 13:00 and 18:30 - 22:00 in winter, 09:00 - 14:00 and 20:00 - 22:00 in summer.
- **Tri-horária:** off-peak at the same hours as bi-horária, and the rest split into peak hours (ponta), the dearest, and shoulder hours (cheias). With the weekly cycle, there are no peak hours at weekends.

| Peak hours | Winter | Summer |
| --- | --- | --- |
| Daily cycle, every day | 09:00 - 10:30 and 18:00 - 20:30 | 10:30 - 13:00 and 19:30 - 21:00 |
| Weekly cycle, Monday to Friday | 09:30 - 12:00 and 18:30 - 21:00 | 09:15 - 12:15 |

Winter hours start with winter time, on the last Sunday of October, and summer hours with summer time, on the last Sunday of March. Public holidays are ordinary days.

Bi-horária and tri-horária move to new hours on a day between 1 July and 31 December 2027, which your grid operator tells you. The plans have today's hours until then.

The plans count the access tariff per kWh. The power term (potência contratada), the electricity tax, the audiovisual contribution, the DGEG fee and the social tariff's discount are left out: they're charged by the day or month, or are the same in every hour, so they don't change which hours are cheapest.

The prices add 23% VAT. Up to 6.9 kVA, a home's first 200 kWh in 30 days pay 6%, but charging the car comes on top of those.

With a price that follows the market (OMIE) by the hour or quarter-hour, choose **Dynamic (spot, exchange)** on the page. With a fixed price, choose Fixed: the plan's periods then make the cheap hours.

The Azores and Madeira have hours of their own and no market prices, and tri-horária above 20.7 kVA has other prices: write your own tariff, as [Tariff](tariff.md) says.

ERSE sets the prices every January. The prices are in the plans in [plans/pt](../plans/pt). [Tariff](tariff.md) says how to change them for yourself.
