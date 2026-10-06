# Luxembourg's plan

The network fee that homes with a smart meter pay per kWh, with prices with VAT. It's set for the whole country, so Luxembourg's four network operators charge the same, and one plan serves them all, which the board's page chooses for you under **Grid plan**.

| Operator | Plan |
| --- | --- |
| Creos Luxembourg | `lu/client-standard` |
| Sudstroum | `lu/client-standard` |
| Ville de Diekirch | `lu/client-standard` |
| Ville d'Ettelbruck | `lu/client-standard` |

The fee is the same in every hour of every day, so it changes what charging costs, not when it's cheapest.

Each home also has a reference power, from 3 kW up, which is on its bill. In each quarter-hour, the kWh drawn above it pay a supplement on top: 8.26 cents with VAT in 2026. The plan leaves it out, as it depends on your power, not the hour, and the board doesn't count it yet ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). A car charging at 11 kW goes above a reference power of 3 or 7 kW. To stay below yours, set the car's charging current low enough, in the car or the Tesla app, and **Charging power** in the board's setup to match. Your operator sets the reference power each month, at the cheapest for your last 12 months, and you can ask it for another, as when you add a charger.

The plan also leaves out the monthly fees and the charges that are the same every hour: the electricity tax and the contribution to the compensation mechanism.

A home with an old meter, or a smart meter that doesn't send its readings, pays the old fee, 6.07 cents per kWh with VAT in every hour: write a custom plan, as [Custom plan](../README.md#custom-plan) says.

The operators publish the next year's prices by 15 October, and the ILR, the regulator, approves them for 1 January. In 2026 the state pays part of the networks' costs, which lowers the prices. [Custom plan](../README.md#custom-plan) says how to change the plan's prices for yourself: download its file from this folder, change it and upload it with **Upload custom plan**.
