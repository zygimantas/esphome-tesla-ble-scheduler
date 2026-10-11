# plans/pl/

Poland's research, checked in October 2026, for this folder's plans and the README's Example of yearly savings (test/savings_example.cpp). Start an update here, re-checking the sources under References and the dates under Changes ahead, rather than researching afresh.

## Summary

On a dynamic contract billed by the hour, the board's charging cost 2,634.79 PLN (617.67 EUR) over the year from 1 October 2025: 41% less than charging at once (4,500.57 PLN, 1,055.32 EUR), 8% less than the car's own schedule from 23:00 (2,875.76 PLN, 674.04 EUR) and 1% less than its best start, 03:00 (2,667.43 PLN, 625.28 EUR). Confidence in the README is Low: 0.03% of homes have a dynamic contract, and it bills another auction's price.

## Decisions

- **Contract:** a dynamic contract, which bills TGE's Fixing I by the hour. The example has only the European auction's price (Fixing II, which Nord Pool and SMARD publish), so it stands in for it.
- **Plan:** `pl/tauron-g12`, as Tauron Dystrybucja is the largest operator. The car's own schedule starts at 23:00, when G12's night does on the clock in summer (22:00 winter time).
- **Billing:** each hour at the average of its four quarter-hours.
- **Margin:** 0.10357 PLN per kWh with VAT, Tauron's own dynamic offer.
- **The same every hour:** 0.059655 PLN per kWh with VAT: the quality fee, the renewable energy fee, the cogeneration fee and excise.
- **Left out:** the capacity fee and the monthly fees, as they don't depend on when the car charges.
- **Market prices:** SMARD's series 257, in euros, at the ECB's daily rate.

## Findings

- **Who pays the market price:** very few homes. URE counted 4,836 household dynamic contracts at the end of 2025, up from 135, which is 0.03% of households; 73% of them are prosumers with solar panels ([URE's report](https://www.ure.gov.pl/download/9/16268/Raport2025.pdf), 29 May 2026). The only later figure is a guess that Pstryk alone has more than ten thousand customers ([Green-news.pl](https://www.green-news.pl/4781-pstryk-dynamiczne-ceny-axpo-Eon-RWE-axpo), 30 July 2026).
- **Prices set in advance:** other households paid one, the same every hour on G11 and in two zones on G12. From October to December 2025 it was 500 zł/MWh without VAT and excise, by law, when the approved tariffs averaged 572.64 zł/MWh ([URE](https://www.ure.gov.pl/pl/urzad/informacje-ogolne/aktualnosci/12880,Prezes-Urzedu-Regulacji-Energetyki-zatwierdzil-zmiany-taryf-na-sprzedaz-energii-.html), 30 September 2025). Then came the tariffs approved for 2026, 495.16 on average ([URE](https://www.ure.gov.pl/pl/urzad/informacje-ogolne/aktualnosci/13002,Prezes-Urzedu-Regulacji-Energetyki-zatwierdzil-taryfy-na-sprzedaz-i-dystrybucje-.html), 17 December 2025), among them Tauron's G11 at 497.0 and its G12 at 543.0 by day and 413.0 by night ([Tauron's tariff](https://bip.ure.gov.pl/download/3/20340/TauronSprzedaz.pdf)).
- **What dynamic contracts cost:** a dynamic contract gave up that capped price and usually cost more. Its customers paid 630 zł/MWh on average in 2025, fixed fees included, against 442 for Fixing I, and those of only four of ten sellers paid under 500 ([URE's report](https://www.ure.gov.pl/download/9/16268/Raport2025.pdf)). The market alone averaged 666.68 zł/MWh in September 2026 ([Bankier](https://www.bankier.pl/wiadomosc/Srednia-cena-kontraktu-na-dostawy-energii-na-TGE-w-IX-wzrosla-do-666-68-zl-MWh-9206794.html), 2 October 2026). Sellers now cap the monthly average: Pstryk at 0.61 zł/kWh with VAT until 31 December 2026 ([terms](https://pstryk.pl/regulamin-tarcza-pstryk-2026)), and Tauron's Ceny dynamiczne MAX caps energy at 0.68 zł/kWh for 23.49 zł a month, without saying whether with or without VAT ([Tauron](https://www.tauron.pl/dla-domu/prad/prad-z-usluga/ceny-dynamiczne-max)).
- **Fixing I and Fixing II:** Tauron, PGE, Enea, Energa and Pstryk offer dynamic contracts, and they bill TGE's Fixing I ([URE's report](https://www.ure.gov.pl/download/9/16268/Raport2025.pdf)), a local auction in złoty, not the European auction's price (TGE's Fixing II), which Nord Pool publishes and the board downloads. For 10:00 - 10:15 on 12 October 2026, Fixing I was 484.01 zł/MWh and Fixing II 550.78 zł/MWh (125.69 EUR), per [TGE's table](https://tge.pl/energia-elektryczna-rdn).
- **Meters and billing:** a dynamic contract needs a remotely read meter, which 48% of household metering points had, 7.99 million of 16.7 million ([URE's report](https://www.ure.gov.pl/download/9/16268/Raport2025.pdf)), and 41% of households in Tauron Dystrybucja's area ([WysokieNapiecie.pl](https://wysokienapiecie.pl/116612-mamy-juz-prawie-10-mln-inteligentnych-licznikow-energii/), 30 January 2026). The contracts price by the hour, not the quarter-hour: Pstryk, Tauron and Enea list hourly prices ([Pstryk](https://pstryk.pl/cennik), [Tauron](https://www.tauron.pl/dla-domu/prad/prad-z-usluga/ceny-dynamiczne), [Enea](https://www.enea.pl/ceny-dynamiczne)), and Energa applies each hour's Fixing I to its four quarter-hours ([Energa's terms](https://www.energa.pl/dam/jcr:2971d640-ec36-434b-ab84-e310a8b1ea07/Regulamin%20naszej%20oferty%20Oferta%20dynamiczna%20II%20dla%20domu%20obowi%C4%85zuj%C4%85cy%20od%201%20listopada%202025%20roku.pdf)). Yet TGE has quoted Fixing II by the quarter-hour since 1 October 2025 and Fixing I since 11 June 2026 ([TGE, 1 October 2025](https://tge.pl/komunikaty-tge-czytaj?cmn_id=92000&title=SDAC%3A+Zmiana+publikacji+wynik%C3%B3w+na+stronie+TGE); [TGE, 15 May 2026](https://tge.pl/komunikaty-tge-czytaj?cmn_id=92080&title=Publikacja+wynik%C3%B3w+notowa%C5%84+RDN+na+stronie+TGE+%E2%80%93+wdro%C5%BCenie+15+MTU+na+aukcji+I+fixing+RDN)).
- **Cheap and dear hours:** URE's report says the cheapest hours are 11:00 - 16:00 in summer and 01:00 - 06:00 in winter, and the dearest 18:00 - 21:00, so a night charge is cheapest mainly in winter ([URE's report](https://www.ure.gov.pl/download/9/16268/Raport2025.pdf)).
- **VAT:** 23%.
- **Per kWh, the same every hour:** 0.0485 zł without VAT, 0.05966 zł with it:
  - the quality fee, 0.0332 zł from 1 February 2026 (0.0331 in January), set by PSE's tariff ([Tauron's notice](https://www.tauron-dystrybucja.pl/-/media/offer-documents/dystrybucja/aktualna-taryfa/nowa-stawka-oplaty-jakosciowej-od-1-lutego-2026r.ashx)), billed apart from the network rate;
  - the renewable energy fee (OZE), 0.0073 zł ([URE](https://www.ure.gov.pl/download/9/15811/InformacjaPUREwysokosstawkioplatyOZE271025.pdf));
  - the cogeneration fee, 0.0030 zł (Dz.U. 2025 poz. 1664);
  - excise, 0.0050 zł (Excise Act, art. 89(3)), which some suppliers put inside their adder.

  The capacity fee is monthly for households, by yearly use, and the transition fee ended with 2025.
- **Margins:** Tauron's own dynamic offer, 0.0842 zł without VAT and excise, 0.10357 with VAT. The big suppliers' range from 0.080 to 0.0855 zł without VAT, with monthly fees of 0 to 27.06 zł ([URE's offer list](https://maszwybor.ure.gov.pl/), 2 October 2026). Energa's adder, 0.0878 zł/kWh, includes excise, as its terms say ([Energa's terms](https://www.energa.pl/dam/jcr:2971d640-ec36-434b-ab84-e310a8b1ea07/Regulamin%20naszej%20oferty%20Oferta%20dynamiczna%20II%20dla%20domu%20obowi%C4%85zuj%C4%85cy%20od%201%20listopada%202025%20roku.pdf), point 9): 0.0828 without it, inside that range.
- **The plans' fees:** Tauron Dystrybucja is the largest operator. Its G12 night, 22:00 - 06:00 and 13:00 - 15:00, follows winter time, so it starts at 23:00 on the clock in summer. Its prices held from January to September 2026.
- **Market prices:** SMARD's series 257, in euros. It matched Nord Pool's PL prices in euros on all 4,992 quarter-hours from 10 August to 30 September 2026, and at the ECB's daily rate it's within half a percent of Nord Pool's in złoty. Both are Fixing II, not what dynamic contracts bill.

## Changes ahead

- **1 January each year:** the sellers' and operators' new tariffs, which URE usually approves in December, as it did 2026's on 17 December 2025. The quality fee follows PSE's tariff, and last changed on 1 February 2026.
- **31 December 2026:** Pstryk's cap on the monthly average ends, as its terms stand.
- **2027:** PGE's dynamic offer moves to Fixing II ([PGE's offer](https://www.gkpge.pl/dla-domu/oferta/dynamiczna-energia-z-pge)).

## Open questions

- **Tauron's adder:** whether it includes excise is inferred, not stated.
- **Margins earlier in the year:** the suppliers' adders before mid-2026 aren't known.
- **Dynamic contracts in 2026:** how many there were after 31 December 2025, and what their customers paid in 2026, stay unpublished until URE's next report, due by 30 May 2027.
- **PGE's move to Fixing II:** it rests on one sentence on PGE's offer page, which refuses automated reads, so it's known only from search excerpts, which also say the offer is on sale to 30 June 2026. No such change was found at another seller.
- **By the hour or the quarter-hour:** PGE's terms, which would say how it prices, couldn't be read either, and Enea's page still says hourly after TGE began quoting Fixing I by the quarter-hour.

## Notes

- **Fixing I:** until the board can take it, its schedules for Poland follow another price than the bill.
- **Excise in the adder:** some suppliers put excise inside their adder, as Energa does, while the example counts it with what's the same every hour: take it out before comparing margins.
- **Zones from PSE's signals:** Tauron's G14 Dynamic, Energa's G11f and Enea's G12eco and G13active take their price zones from PSE's signals, which a plan with fixed hours can't hold.

## References

| Source | What it gives | Changes |
| --- | --- | --- |
| [Tauron Dystrybucja's tariff for 2026](https://www.tauron-dystrybucja.pl/-/media/offer-documents/dystrybucja/aktualna-taryfa/taryfa-tauron-dystrybucja-2026_ocr.ashx); the other operators' are linked in their plans' first lines | the plans' fees and hours | each 1 January, approved by URE, usually in December |
| [URE's report for 2025](https://www.ure.gov.pl/download/9/16268/Raport2025.pdf) | the count of dynamic contracts and what their customers paid, remotely read meters, the cheapest hours | each year, by 30 May |
| [URE's offer list](https://maszwybor.ure.gov.pl/) | the suppliers' margins and monthly fees | any time |
| The sellers' dynamic offers: [Tauron](https://www.tauron.pl/dla-domu/prad/prad-z-usluga/ceny-dynamiczne), [Pstryk](https://pstryk.pl/cennik), [Enea](https://www.enea.pl/ceny-dynamiczne), [Energa's terms](https://www.energa.pl/dam/jcr:2971d640-ec36-434b-ab84-e310a8b1ea07/Regulamin%20naszej%20oferty%20Oferta%20dynamiczna%20II%20dla%20domu%20obowi%C4%85zuj%C4%85cy%20od%201%20listopada%202025%20roku.pdf), [PGE](https://www.gkpge.pl/dla-domu/oferta/dynamiczna-energia-z-pge), and the caps: [Pstryk's](https://pstryk.pl/regulamin-tarcza-pstryk-2026), [Tauron's](https://www.tauron.pl/dla-domu/prad/prad-z-usluga/ceny-dynamiczne-max) | which price they bill, by the hour or the quarter-hour, and their caps | any time |
| [Tauron's notice on the quality fee](https://www.tauron-dystrybucja.pl/-/media/offer-documents/dystrybucja/aktualna-taryfa/nowa-stawka-oplaty-jakosciowej-od-1-lutego-2026r.ashx) | the quality fee, from PSE's tariff | with PSE's tariff, last on 1 February 2026 |
| [URE on the OZE fee](https://www.ure.gov.pl/download/9/15811/InformacjaPUREwysokosstawkioplatyOZE271025.pdf), and Dz.U. 2025 poz. 1664 | the renewable energy and cogeneration fees | each year |
| [TGE's day-ahead results](https://tge.pl/energia-elektryczna-rdn) | Fixing I and Fixing II, by the quarter-hour | daily |
| [SMARD's series 257](https://www.smard.de/app/chart_data/257/DE/257_DE_quarterhour_1766962800000.json) | the market prices (Fixing II) in euros, a week a file | daily |
