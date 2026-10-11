# plans/

- `name:` replaced the name in the first line (`# <name>, prices with VAT: <link>`) in October 2026, in every plan at once, by the owner's decision, rather than in a new folder: boards on 5.3.0 and before turn such a plan away and keep the one they have until they're updated.
- Only the `*.yaml` files are plans: the README.md and AGENTS.md files never reach a board, and the unit tests read only the `.yaml` files, as the board does. Run test/scheduler_test.cpp after any change.
- Each country's AGENTS.md holds what's known about the country, checked in October 2026, for its plans and the README's Example of yearly savings (test/savings_example.cpp): a summary, the example's decisions, the findings with their sources (who pays the market price and how it's billed, VAT, taxes and levies per kWh, margins, the plans' fees, the market prices), the changes ahead, open questions, notes (traps, things to fix, what's not done yet) and a table of the sources to re-check, with when each changes. Start the yearly update (#161), or any question about a country, there rather than researching afresh, and keep it current.
- What goes in a plan, as the owner settled it in October 2026: the operator's own fee per kWh for that plan, with VAT, summing the per-kWh parts it bills together (like Austria's usage and losses), and another network charge only if it too changes with the hour. Taxes, levies, a transmission operator's flat tariff on a line of its own, monthly fees and fees per kW stay out, and the country's README.md says which. Where VAT is split, it's the rate on the extra kWh a car adds: Portugal's 23%, not the 6% on a home's first 200 kWh. Norway's Arva has no VAT, as its whole area is exempt.
- Conventions: 5 decimals (older plans drop a trailing zero); `name:` at most about 40 characters, for the phone's list, as the first key, above `currency:`; the first line, `# Prices with VAT: <link>`, links the operator's prices, or the official document that has them all or that can be read where the operator's page can't (Austria's RIS, Spain's BOE, Romania's ANRE orders, the CWaPE's and Brugel's copies in Belgium); comments on how the prices were converted and on any known change ahead; public holidays only where the operator treats them as weekends, moving ones at their next date with a comment above.
- How the plans of October 2026 were made: one researcher per country (Germany in two), an independent checker that fetched every source again, and a third agent to settle what they disagreed on. Every country had a disagreement, and the checkers found real errors: last year's price list, a wrong VAT rate, a sheet's preliminary version. Do the same for a new country or a yearly update.

## Not done yet

Fees per kW (#82) matter in more countries over time: Norway's capacity step, Sweden's power fees, Luxembourg's supplement, Slovenia's, Austria's from 2027 and Finland's from 2029. Each country's AGENTS.md says what else it left out, where its prices are and its traps.

## Settled with the owner; don't propose again

- No plans for France, whose off-peak hours are set for each address, or the Netherlands, which has no grid fee per kWh (time-of-use network fees are planned from 2029: plans then, one per operator).
- All the plans stay, without trimming the long lists of Germany, Sweden, Finland and Switzerland.
- Czechia's, Hungary's and Switzerland's plans are in their own currency, and the board converts SMARD's euros into it at the ECB's daily rate.
- The plans went in with 2026's prices, to update at the turn of the year (#161).
