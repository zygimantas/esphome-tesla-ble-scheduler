# Romanian plans

The distribution tariffs of Romania's four distribution operators for homes on low voltage, with prices with VAT, in lei. Name yours under `tariff:` in your settings file, with Romania's market area and VAT, which make lei the currency:

```yaml
market:
  area: RO
  vat: 0.21
tariff:
  plan: ro/delgaz-grid-jt
```

Find your operator by your county:

| Operator | Counties | Plan |
| --- | --- | --- |
| Distribuție Energie Electrică Romania (DEER) | Alba, Bihor, Bistrița-Năsăud, Brașov, Brăila, Buzău, Cluj, Covasna, Dâmbovița, Galați, Harghita, Maramureș, Mureș, Prahova, Satu Mare, Sălaj, Sibiu and Vrancea | `ro/deer-jt` |
| Delgaz Grid | Bacău, Botoșani, Iași, Neamț, Suceava and Vaslui | `ro/delgaz-grid-jt` |
| Distribuție Energie Oltenia | Argeș, Dolj, Gorj, Mehedinți, Olt, Teleorman and Vâlcea | `ro/distributie-oltenia-jt` |
| Rețele Electrice România | Bucharest, Arad, Caraș-Severin, Călărași, Constanța, Giurgiu, Hunedoara, Ialomița, Ilfov, Timiș and Tulcea | `ro/retele-electrice-jt` |

Each operator has one tariff for low voltage (JT), which nearly every home is on. It's the distribution tariff on your bill, the operator's tariffs for high, medium and low voltage added up, and it's the same in every hour of every day. It doesn't change which hours are cheapest, the market price does, but with it the costs the board shows are whole.

With a contract that follows the market by the hour, choose **Dynamic (spot, exchange)** on the page. With one fixed price for every hour, choose **Fixed**: every hour then costs the same, so the board charges at once. With your supplier's own day and night prices, or on medium voltage or a small local network, write your own plan, as [Your own plan](../README.md#your-own-plan) says.

The plans leave out the charges that are the same in every hour and aren't the distribution operator's: Transelectrica's transmission and system services tariffs, the green certificates, the cogeneration contribution, the contribution for contracts for difference and the excise.

ANRE, the energy regulator, sets each operator's tariffs, usually from 1 January. These are 2026's. [Your own changes](../README.md#your-own-changes) says how to change the plans' prices for yourself.
