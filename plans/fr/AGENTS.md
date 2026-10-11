# plans/fr/

France's research, checked in October 2026, for this folder's README.md and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a dynamic offer billing EPEX's French day-ahead price, as Sobry's and Frank Énergie's do, the board's charging cost 560.93 EUR over the year from 1 October 2025: 26% less than charging at once (756.91 EUR), 23% less than the car's own schedule from 22:00 (731.64 EUR) and 1% less than its best start, 04:00 (564.95 EUR). Confidence in the README is Low: two small suppliers bill the market price, and off-peak hours differ by address.

## Decisions

- **Contract:** a dynamic offer billing EPEX's French day-ahead price, as Sobry's and Frank Énergie's do.
- **Plan:** TURPE's network fee with off-peak hours (CU4), 22:00 - 06:00, as a custom plan: Enedis sets the off-peak hours for each address, and the owner settled that there are no plans for France. The car's own schedule starts at 22:00, when the off-peak fee does.
- **Billing:** each hour at the average of its four quarter-hours, as Frank Énergie bills.
- **Margin:** 0.02340 EUR per kWh with VAT, the mean of the two offers.
- **The same every hour:** the accise, 0.03702 EUR per kWh with VAT, 2026's.
- **Left out:** TURPE's fixed parts, the CTA and the supplier's subscription, as they don't depend on when the car charges.
- **Market prices:** SMARD's series 254, as Nord Pool, the board's own source for France, serves only recent days without an account.

## Findings

- **Who pays the market price:** few homes. The CRE names two small suppliers that bill the market price itself, EPEX SPOT's French day-ahead price: Sobry, a brand launched in 2025, by quarter-hour, and Frank Énergie, which announced its French launch on 10 February 2026, by hour ([CRE's report to the European Commission, 31 July 2026](https://www.cre.fr/fileadmin/Documents/Rapports_et_etudes/2026/Rapport_annuel_DG_ENER_2026_CRE.pdf), pages 83 and 84; [CRE consultation 2026-02](https://www.cre.fr/fileadmin/Documents/Consultations_publiques/2026/260122_2026-02_Offres_tarification_dynamique.pdf)). No count of customers is published. The CRE calls the demand very limited and counts dynamic contracts among the 7% of homes' market offers, about 1 million, that are neither Base nor HP/HC, with offers like super off-peak or solar hours ([CRE report 2026-06](https://www.cre.fr/fileadmin/Documents/Rapports_et_etudes/2026/Rapport_Coherence_des_offres_2026.pdf)).
- **The law:** Article L. 332-7 of the Code de l'énergie (March 2021) makes every supplier with more than 200,000 sites offer a dynamic contract to homes with a Linky meter. The CRE's deliberation 2021-135 defines it: an energy price at least half indexed on the day-ahead or intraday market, following it at least by the hour, with each month's bill capped at twice the regulated Base tariff's. Since 1 January 2023 a transitional rule (deliberation 2022-215, extended to 30 June 2027 by deliberation 2026-128) lets the big suppliers offer simpler peak-day offers instead, and they did: EDF's Zen option Flex, TotalEnergies' Heures Eco+, ENGIE's éco défis+ rebates.
- **Meters:** any home with a Linky meter can sign up for a dynamic contract, which is most of them (94% of EDF's regulated-tariff homes at the end of 2024, CRE deliberation 2025-155); Frank only on Enedis's network.
- **How homes buy:** on 30 June 2026, 19.20 million of 35.04 million homes had the regulated tariff (TRVE) and 15.84 million a market offer ([CRE observatory](https://www.cre.fr/fileadmin/Documents/Rapports_et_etudes/2026/Observatoire_T2_2026.pdf)), of which a third follow the TRVE, a third are fixed and a third change on the supplier's own terms. 14.5 million homes (40%) have peak and off-peak prices (HP/HC), most others Base, one price all day.
- **The regulated tariff:** EDF's from 1 August 2026, per kWh with VAT: Base 0.20010; HP 0.21420, HC 0.15890; Tempo, with 300 blue, 43 white and 22 red days that RTE announces the day before and off-peak 22:00 - 06:00 for everyone, 0.13560 and 0.16540 on blue days, 0.15360 and 0.19210 on white, 0.16150 and 0.72950 on red, off-peak first ([EDF](https://particulier.edf.fr/content/dam/2-Actifs/Documents/Offres/Grille_prix_Tarif_Bleu.pdf)). With HP/HC or Tempo every off-peak hour costs the same, so the car's own schedule from the start of off-peak already gets the night's lowest price, and the board can only match it.
- **VAT:** 20% on everything a kWh costs: the market price, the supplier's part, TURPE and the accise, all year from October 2025 to September 2026 ([CRE](https://www.cre.fr/electricite/marche-de-detail-de-lelectricite/presentation.html); EDF's tariff sheet). The subscription and the CTA went from 5.5% to 20% on 1 August 2025, before the year studied.
- **Per kWh, the same every hour:** only the accise sur l'électricité, formerly TICFE and CSPE, at the households' rate ("ménages et assimilés"), the surcharge for the islands (ZNI) included. The example takes 2026's rate, in force the longest in 2026 and in the year studied: 0.03702 EUR with VAT. Worked out from the rates below, October to December 2025 paid 0.00104 less.
  - 29.98 EUR per MWh from 1 August 2025 to 31 January 2026: 0.02998 EUR per kWh, 0.03598 with VAT ([impots.gouv.fr](https://www.impots.gouv.fr/actualite/consommation-denergie-tarifs-normaux-des-accises-en-2025); [BOFiP BOI-RES-EAT-000240](https://bofip.impots.gouv.fr/bofip/14903-PGP.html) for January 2026);
  - 30.85 EUR per MWh from 1 February to 31 July 2026: 0.03085, 0.03702 with VAT (the same BOFiP note; the CRE's report of 31 July 2026, page 81);
  - 30.62 EUR per MWh from 1 August 2026: 0.03062, 0.03674 with VAT (EDF's tariff sheet; Sobry's grid).
- **Fixed parts, left out:** TURPE's management, metering and power parts, the CTA on them (15% since 1 February 2026, 21.93% before), and the supplier's subscription.
- **Margins:** 0.02340 EUR per kWh with VAT is the mean of the two offers that bill the market price:
  - Frank Énergie: (EPEX's hourly day-ahead price + 0.01900 + TURPE + accise) times 1.2, so 0.02280 with VAT, which it calls procurement costs (capacity, guarantees of origin), and 6.00 EUR a month with VAT. [Price sheet](https://cdn.sanity.io/files/8navd656/production-fr/7186cef7994ce2ba46c482cdd338ad25cb095dee.pdf) of 14 July 2026, for 1 August 2026, from [frankenergie.fr](https://www.frankenergie.fr/fr/tarification-dynamique).
  - Sobry SoFlex, prices from 1 October 2026, with VAT: margin 0.00960, a premium for its cap 0.00240, energy savings certificates (CEE) 0.00840 and capacity 0.00360, so 0.02400, and 19.84 EUR a month for 6 kVA in CU4, TURPE's fixed part included. The spot price, TURPE, the accise, the CEE and capacity are capped together at 0.40 EUR per kWh. [Price grid](https://storage.googleapis.com/sobry-legals/grille-tarifaire-sobry.pdf), from [sobry.co](https://www.sobry.co).
- **The grid fee:** TURPE, set by the CRE and the same with every supplier. TURPE 7 HTA-BT started on 1 August 2025, not 1 February 2025 (deliberation 2025-78 of 13 March 2025), and changes every 1 August: +3.04% on 1 August 2026 ([deliberation 2026-105](https://www.cre.fr/fileadmin/Documents/Deliberations/2026/260521_2026-105_Evolution_TURPE_7_HTA-BT.pdf)). Since 1 August 2024 every home with a Linky meter has a four-period option, CU4 or MU4, which the supplier chooses (for the regulated tariff, the cheaper for each home); Sobry bills CU4, Frank lists both. Peak (HP) and off-peak (HC) prices differ in the high season, November to March, and the low season, April to October. Every day is alike, weekends and holidays included. Per kWh, in the order HPH, HCH, HPB, HCB:
  - CU4 from 1 August 2025 to 31 July 2026: 0.07490, 0.03970, 0.01660, 0.01160; with VAT 0.08988, 0.04764, 0.01992, 0.01392 (Enedis's brochure of 1 August 2025, [archived](https://web.archive.org/web/20251118170841/https://www.enedis.fr/media/4717/download));
  - CU4 from 1 August 2026: 0.07720, 0.04090, 0.01710, 0.01200; with VAT 0.09264, 0.04908, 0.02052, 0.01440 ([Enedis](https://www.enedis.fr/media/4717/download); the same in Frank's and Sobry's sheets);
  - MU4: 0.07000, 0.03730, 0.01610, 0.01110, with VAT 0.08400, 0.04476, 0.01932, 0.01332; from 1 August 2026 0.07210, 0.03840, 0.01660, 0.01140, with VAT 0.08652, 0.04608, 0.01992, 0.01368.
- **Off-peak hours:** Enedis sets the 8 off-peak hours for each address. In February 2025, of 14.5 million HP/HC homes, 8.8 million had them all at night, 5.7 million split between night and day, and 1.4 million had 23:00 - 07:00 ([CRE](https://www.cre.fr/fileadmin/Documents/Communiques_de_presse/2025/250206_Annexe_CP_TURPE_7_HPHC.pdf)). Common night windows are 22:00 - 06:00, 22:30 - 06:30 and 23:00 - 07:00. A reform moves 11 million homes to at least 5 hours in a row between 23:00 and 07:00 and the rest between 11:00 and 17:00: 1.7 million homes with split hours from November 2025 to June 2026, then 9.3 million from December 2026 to October 2027, with other hours in summer than in winter ([Enedis](https://www.enedis.fr/magazine/modification-des-heures-creuses-comprendre-ce-qui-change-partir-du-1er-novembre-2025)). So most homes kept their old hours through September 2026.
- **The example's custom plan:** 2026's values, the grid of 1 August 2025, in force from January to July 2026 and for 10 of the 12 months studied. With this folder's README.md's example window, 22:00 - 06:00, the cheap fee starts at 22:00:

  ```yaml
  # Prices with VAT: https://web.archive.org/web/20251118170841/https://www.enedis.fr/media/4717/download
  name: TURPE CU4, 22:00 - 06:00
  currency: EUR
  calendar:
    nov-mar:
      mon-sun: hch 06:00 hph 22:00 hch
    apr-oct:
      mon-sun: hcb 06:00 hpb 22:00 hcb
  rates:
    hph: 0.08988
    hch: 0.04764
    hpb: 0.01992
    hcb: 0.01392
  ```

- **Market prices:** SMARD (the Bundesnetzagentur) has France's day-ahead prices as series 254, by quarter-hour, in euros per MWh, a file a week from Monday 00:00 German time, as the board reads SMARD's other areas (smard_url() and add_smard() in scheduler/market.h): its [index](https://www.smard.de/app/chart_data/254/DE/index_quarterhour.json), and the week from Monday 29 September 2025, https://www.smard.de/app/chart_data/254/DE/254_DE_quarterhour_1759096800000.json. It's EPEX SPOT's French day-ahead price, the one Sobry and Frank bill (Frank by the hour). Checked on 11 October 2026: all 35,072 quarter-hours from 1 October 2025 00:00 to 1 October 2026 08:00 are there, and 15 September 2026's 96 equal Nord Pool's FR prices, the board's own source for France.

## Changes ahead

- **1 August each year:** TURPE's new prices, by the CRE's decision in May, as +3.04% on 1 August 2026.
- **November 2026:** the new capacity mechanism's cost falls on suppliers by what their customers use on days of tension; Sobry passes it on as a flat 0.00360 EUR per kWh.
- **December 2026 to October 2027:** Enedis moves the off-peak hours of 9.3 million homes, with other hours in summer than in winter.
- **30 June 2027:** the transitional rule that lets the big suppliers offer peak-day offers instead of dynamic contracts ends.

## Open questions

- **How many homes pay the market price:** no source counts the homes on dynamic contracts. That they're few rests on the CRE's words and on two small suppliers.
- **Margins earlier in the year:** the margin rests on two offers at their prices of August and October 2026; their earlier prices aren't known.
- **Off-peak windows:** no source gives a national breakdown. 22:00 - 06:00 is common and this folder's README.md's example; 23:00 - 07:00 is Paris's usual window (Selectra) and the reform's night core. Worked out over the year, the market price averaged 109 EUR per MWh at 22:00, 99 at 23:00 and 67 at 04:00, so a 23:00 start makes the car's own schedule cheaper.
- **CU4 or MU4:** it depends on the home. MU4 costs about 2 EUR a year more per kVA before VAT and less per kWh.
- **The accise from 1 August 2026:** 30.62 EUR per MWh is on EDF's and Sobry's sheets and in the AFP's report, not on impots.gouv.fr yet. Deloitte quotes the finance law's 30.35, which EDF says was then raised by inflation.

## Notes

- **What a plan can hold:** nothing else changes a dynamic contract's price by the hour. Tempo and the peak-day offers (about 20 days notified the day before) can't go in a plan. Weekend and super off-peak offers, fixed by the hour, can, as a whole price.
- **Enedis's brochure:** its link now serves 2026's grid, so 2025's comes from an archived copy, which matches the 2026 grid divided by 1.0304. CRE deliberation 2025-78 itself wasn't read, so the 2025 grid rests on that copy.
- **Energy-Charts:** its API republishes SMARD's prices, so it's no second source.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Enedis's TURPE brochure](https://www.enedis.fr/media/4717/download) | TURPE's prices per kWh, CU4 and MU4 | each 1 August |
| [CRE deliberation 2026-105](https://www.cre.fr/fileadmin/Documents/Deliberations/2026/260521_2026-105_Evolution_TURPE_7_HTA-BT.pdf) | TURPE's yearly change | each May, for 1 August |
| [impots.gouv.fr](https://www.impots.gouv.fr/actualite/consommation-denergie-tarifs-normaux-des-accises-en-2025) and [BOFiP BOI-RES-EAT-000240](https://bofip.impots.gouv.fr/bofip/14903-PGP.html) | the accise | by law, on 1 February and 1 August in 2026 |
| [Frank Énergie's dynamic price](https://www.frankenergie.fr/fr/tarification-dynamique) | its margin and monthly fee | any time |
| [Sobry's price grid](https://storage.googleapis.com/sobry-legals/grille-tarifaire-sobry.pdf) | its margin, CEE, capacity and monthly fee | any time |
| [CRE's report to the European Commission](https://www.cre.fr/fileadmin/Documents/Rapports_et_etudes/2026/Rapport_annuel_DG_ENER_2026_CRE.pdf) | the suppliers that bill the market price | each year |
| [CRE's observatory](https://www.cre.fr/fileadmin/Documents/Rapports_et_etudes/2026/Observatoire_T2_2026.pdf) | homes on the regulated tariff and on market offers | each quarter |
| [SMARD's index for France](https://www.smard.de/app/chart_data/254/DE/index_quarterhour.json) | the market prices, a week a file | daily |
