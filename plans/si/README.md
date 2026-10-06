# Slovenia's network charge

The network charge (omrežnina) that households in Slovenia pay, for user group 0 with 15-minute metering, with prices with VAT. It's the same with every distribution company. The board's page chooses it for you under **Grid plan**.

Each quarter-hour falls in one of five time blocks, from block 1, the dearest, to block 5. The block depends on the season, higher from November to February and lower from March to October, and on the day: a workday, or a Saturday, Sunday or work-free public holiday.

| Hours | Higher season, workday | Higher season, other day | Lower season, workday | Lower season, other day |
| --- | --- | --- | --- | --- |
| 07:00 - 14:00 and 16:00 - 20:00 | 1 | 2 | 2 | 3 |
| 06:00 - 07:00, 14:00 - 16:00 and 20:00 - 22:00 | 2 | 3 | 3 | 4 |
| 22:00 - 06:00 | 3 | 4 | 4 | 5 |

The hours follow the clock, summer time included. In 2026, block 4 costs a little more per kWh than block 3.

The plan counts the charge per kWh. Most of the network charge is a monthly charge per kW for each block, which the board doesn't count yet ([#82](https://github.com/zygimantas/esphome-tesla-ble-scheduler/issues/82)). The contributions and the excise per kWh are left out too, as they're the same every hour.

The blocks' hours change on 1 January 2027, and the plan changes with them, with 2027's prices. [Custom plan](../README.md#custom-plan) says how to change the plan's prices for yourself: download its file from this folder, change it and upload it with **Upload custom plan**.
