# German plans

The network charges per kWh (Netzentgelt) of Germany's big grid operators for homes, with prices with VAT. Your electricity bill names your grid operator (Netzbetreiber). Choose its plan under **Grid plan** on the board's page.

| Operator | Standard | Modul 2 | Modul 3 |
| --- | --- | --- | --- |
| Avacon Netz | `de/avacon-standard` | `de/avacon-modul-2` | `de/avacon-modul-3` |
| Bayernwerk Netz | `de/bayernwerk-standard` | `de/bayernwerk-modul-2` | `de/bayernwerk-modul-3` |
| E.DIS Netz | `de/edis-standard` | `de/edis-modul-2` | `de/edis-modul-3` |
| EAM Netz | `de/eam-standard` | `de/eam-modul-2` | `de/eam-modul-3` |
| EWE NETZ | `de/ewe-standard` | `de/ewe-modul-2` | `de/ewe-modul-3` |
| Hamburger Energienetze | `de/hamburger-energienetze-standard` | `de/hamburger-energienetze-modul-2` | `de/hamburger-energienetze-modul-3` |
| MITNETZ STROM | `de/mitnetz-standard` | `de/mitnetz-modul-2` | `de/mitnetz-modul-3` |
| N-ERGIE Netz | `de/n-ergie-standard` | `de/n-ergie-modul-2` | `de/n-ergie-modul-3` |
| Netze BW | `de/netze-bw-standard` | `de/netze-bw-modul-2` | `de/netze-bw-modul-3` |
| RheinNetz | `de/rheinnetz-standard` | `de/rheinnetz-modul-2` | `de/rheinnetz-modul-3` |
| Stromnetz Berlin | `de/stromnetz-berlin-standard` | `de/stromnetz-berlin-modul-2` | `de/stromnetz-berlin-modul-3` |
| SWM Infrastruktur | `de/swm-standard` | `de/swm-modul-2` | `de/swm-modul-3` |
| Westnetz | `de/westnetz-standard` | `de/westnetz-modul-2` | `de/westnetz-modul-3` |

On older bills, Hamburger Energienetze is Stromnetz Hamburg, its name until 2024, and RheinNetz is Rheinische NETZGesellschaft.

- **Standard:** the same fee every hour, which a home pays unless it has Modul 3. A wallbox on Modul 1, a yearly sum off the bill, keeps this one.
- **Modul 2:** for a wallbox on a meter of its own, registered with the grid operator as a steerable device (§ 14a EnWG): the same fee every hour on that meter, 40% of the standard one.
- **Modul 3:** for a home with a smart meter (intelligentes Messsystem) and a registered wallbox on Modul 1: a low, a standard and a high fee by the hour. You ask for it through your supplier.

Modul 3's low and high hours, every day of the week, public holidays too:

| Operator | Months | Low | High |
| --- | --- | --- | --- |
| Avacon Netz | October to March | 23:00 - 05:00 | 16:30 - 21:00 |
| Bayernwerk Netz | April to September | 10:00 - 15:00 | 17:00 - 22:00 |
| E.DIS Netz | October to March | 23:30 - 05:00 | 10:15 - 12:00 and 16:45 - 20:15 |
| EAM Netz | All year | 23:00 - 06:00 and 11:00 - 15:00 | 17:00 - 22:00 |
| EWE NETZ | All year | 23:00 - 05:00 | 16:30 - 20:30 |
| Hamburger Energienetze | All year | 00:30 - 07:00 | 17:15 - 21:00 |
| MITNETZ STROM | October to March | 19:00 - 03:00 | 08:00 - 12:00 and 17:00 - 19:00 |
| N-ERGIE Netz | All year | 23:00 - 06:00 and 12:00 - 14:15 | 18:00 - 21:00 |
| Netze BW | All year | 10:00 - 14:00 | 17:00 - 22:00 |
| RheinNetz | All year | 23:45 - 05:45 | 17:00 - 19:30 |
| Stromnetz Berlin | All year | 22:15 - 06:30 | 17:15 - 20:15 |
| SWM Infrastruktur | October to March | 00:45 - 06:45 | 08:30 - 20:45 |
| Westnetz | All year | 00:00 - 07:00 | 15:00 - 20:00 |

The other hours, and the other months all day, have the standard fee, the same as the standard plan's. The hours follow the clock, summer time included.

The plans count the network charge per kWh. They leave out the yearly base price (Grundpreis), Modul 1's yearly reduction and the meter's fee, and the charges that aren't the grid operator's own: the concession fee (Konzessionsabgabe), the levies (KWKG, § 19 StromNEV and the offshore network levy) and the electricity tax (Stromsteuer). These are the same every hour, except that a supplier's time-variable price may have a lower concession fee in its cheap hours.

Schleswig-Holstein Netz, LEW Verteilnetz, Syna, Netze ODR and the hundreds of smaller operators aren't here yet, nor the reduced fees of wallboxes and heat pumps that had them before 2024, with the operator's own hours. For those, write a custom plan, as [Custom plan](../README.md#custom-plan) says.

Each operator sets its prices, and Modul 3's hours, for a calendar year, under the Federal Network Agency's rules. It publishes next year's by 15 October, as preliminary, and they change on 1 January. [Custom plan](../README.md#custom-plan) says how to change the plans' prices for yourself: download a plan's file from this folder, change it and upload it with **Upload custom plan**.
