# Austria's network charges

The network charges that households in Austria pay per kWh, the network usage and network loss charges (Netznutzungsentgelt and Netzverlustentgelt), with prices with VAT. E-Control sets them for each network area, and every operator in an area charges the same: a smaller one, like a town's own, charges the prices of its area. Your bill names your operator (Netzbetreiber). Name its plan under `tariff:` in your settings file, with Austria's market area and VAT:

```yaml
market:
  area: AT
  vat: 0.2
tariff:
  plan: at/wiener-netze
```

| Operator | Area | Plan |
| --- | --- | --- |
| Energie Klagenfurt | Klagenfurt | `at/energie-klagenfurt` |
| Energienetze Steiermark | Styria | `at/energienetze-steiermark` |
| Innsbrucker Kommunalbetriebe (IKB) | Innsbruck | `at/ikb` |
| KNG-Kärnten Netz | Carinthia | `at/kng-kaernten-netz` |
| LINZ NETZ | Linz | `at/linz-netz` |
| Netz Burgenland | Burgenland | `at/netz-burgenland` |
| Netz Niederösterreich | Lower Austria | `at/netz-niederoesterreich` |
| Netz Oberösterreich | Upper Austria | `at/netz-oberoesterreich` |
| Salzburg Netz | Salzburg | `at/salzburg-netz` |
| Stromnetz Graz | Graz | `at/stromnetz-graz` |
| TINETZ-Tiroler Netze | Tyrol | `at/tinetz` |
| Vorarlberger Energienetze | Vorarlberg | `at/vorarlberger-energienetze` |
| Wiener Netze | Vienna | `at/wiener-netze` |

Linz, Graz, Innsbruck and Klagenfurt are areas of their own, with their own prices.

- **From October to March:** the same price every hour.
- **From April to September:** the same, but from 10:00 to 16:00 every day the network usage charge is 20% lower, the summer price (SNAP).

The summer price needs a smart meter that sends your operator quarter-hour readings (Viertelstundenwerte): allow them in its customer portal if it doesn't have them yet. A dynamic contract needs them anyway. The hours follow the clock, summer time included.

The plans leave out the yearly flat fee, the meter fee, the renewable energy fees (Erneuerbaren-Förderbeitrag and Erneuerbaren-Förderpauschale), the electricity tax and, in Vienna, the Gebrauchsabgabe, as they don't change which hours are cheapest.

- **An interruptible circuit** (unterbrechbar) on its own meter, which the operator may switch off at times it sets, has lower prices. Give them as your own rates, as [Your own changes](../README.md#your-own-changes) says.
- **Day and night prices** (Doppeltarif) ended on 31 March 2026: those meters pay the area's plan now.
- **The Kleinwalsertal** has prices of its own: write your own tariff, as [Your own tariff](../README.md#your-own-tariff) says.

E-Control sets the prices every year, from 1 January, and the plans change with them. For 2027, its draft adds a lower price from 22:00 to 04:00 from October to March (WiNAP), and a monthly fee per kW of the month's highest quarter-hour, which the board doesn't count yet ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)).

[Your own changes](../README.md#your-own-changes) says how to change the plans' prices for yourself.
