# Hungary's network fees

The network fees per kWh (rendszerhasználati díj) that households in Hungary pay, whoever their supplier, with prices with VAT, in forints. MEKH, the energy regulator, sets the same fees for every network operator:

| Plan | Operators |
| --- | --- |
| `hu/rendszerhasznalati-dij` | E.ON Észak-dunántúli, E.ON Dél-dunántúli, ELMŰ Hálózati, MVM Émász, MVM Démász and OPUS TITÁSZ |

The plan is the transmission fee and the distribution fee per kWh together, the same in every hour, with an ordinary meter or a smart one. A smart meter's distribution fee has three zones, 06:00 - 17:00, 17:00 - 22:00 and 22:00 - 06:00, but in 2026 they cost the same.

Name it under `tariff:` in your settings file, with forints as the currency. The board takes Hungary's market prices from SMARD in euros and converts them into forints at the ECB's daily rate, as the page does when you choose Hungary:

```yaml
currency: HUF
market:
  area: HU
  vat: 0.27
tariff:
  plan: hu/rendszerhasznalati-dij
```

With universal service's price, choose **Fixed (or a monthly average)** on the page and enter your price per kWh with VAT, less the plan's 29.72, as the supplier's part (`fixed_price` in the settings file). Universal service's reduced price ends at 2523 kWh a year; above it, a kWh costs 70.104 forints with the network fees and VAT in, which makes 40.39. All hours then cost the same, so the board charges at once.

Universal service's two-rate tariff A2, which an electric car's owner can have on a meter of its own next to A1, is cheaper on workdays at night, 22:00 - 06:00 (23:00 - 07:00 in summer), and all day on other days, up to 2523 kWh a year; above that, both cost the same. The cheap hours are in the supplier's price, not in the network fees, so no plan has them: write your own plan with your whole A2 prices, `clock: winter` and public holidays under `exceptions`, as [A fixed price](../README.md#a-fixed-price) says.

A controlled circuit (B tariff, vezérelt) on its own meter gets power only when the network operator switches it on, at least 8 hours a day, so no plan can say when. The H tariff is for heat pumps only.

The plan leaves out the yearly fee per connection, a smart meter's fee per kW, which is 0 in 2026, and any levies and taxes per kWh, which are the same every hour.

MEKH sets the network fees every year, from 1 January. [Your own changes](../README.md#your-own-changes) says how to change the plan's prices for yourself.
