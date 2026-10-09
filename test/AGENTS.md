# test/

## Unit tests

- Build and run them from the repository root; the include paths assume it, and the tests read plans/.
- scheduler_test_tesla.h, the simulated Tesla, is also the simulation's car: keep it free of unit-test code.
- CHECK is a macro: a comma outside parentheses, as in `std::vector<std::pair<A, B>>`, splits its argument. Use a type alias.
- A plug-in counts as one (the phone message, a fresh schedule mode) only after a tick with the car unplugged: the first plug state the controller sees is taken as the state after a restart.
- On the first tick there's no schedule yet, so a start from the car on that tick doesn't switch to charging now, unless Stop charging was in force. Tick once before.
- simulate() ticks every 30 s and can't tell a 2-minute limit from a 1:58 one. Test limits with your own ticks just before and at the boundary.

## The simulation

- ESPHome 2026.10's precompiled header, forced into every file (`-include esphome_pch.h`), breaks the host build: logger_host.cpp's `time()` finds `namespace esphome::time` (posix_tz.h, with a time zone), and on a Mac httplib.h's MacTypes.h finds ArduinoJson's `Ptr`. `ESPHOME_PCH_ENABLE=0` leaves it out, in the workflows and CONTRIBUTING.md; the firmware builds with it. Try without it at the next ESPHome update.
- Its program needs a terminal and a time limit: `ESPHOME_PREFDIR=$(mktemp -d) python3 -c 'import pty, sys; pty.spawn(sys.argv[1:])' perl -e 'alarm 45; exec @ARGV' test/.esphome/build/simulation/.pioenvs/simulation/program > log`, then `grep -F '][scheduler' log`. ESPHOME_PREFDIR starts it without what an earlier run saved, like the buttons' mode, Ready by and savings.
- Without CHEAP_NOW=1, the outcome depends on the time of day, as the made-up prices are cheap from 01:00 to 05:00. A first "Wake the car (Reading battery)" is a startup race with the made-up sensors, not a bug, and the start may then wait for the next tick, 30 s later: give the program well over 30 s.

## Mutation testing

- A run takes about 40 minutes on 8 cores and builds scheduler_test.cpp for every mutant: don't edit it while a run is going. Survivor line numbers refer to the headers as they were when the run started. On the owner's Mac a run took 5 hours in October 2026, with XProtect checking every new program and ReportCrash reporting every mutant that crashes.
- About 240 survivors are entries of two tables, AREAS in market.h and TIME_ZONES in settings.h: data, which a test would only repeat.
- Around 280 others are equivalent; don't chase them: spare buffer sizes, the order of declarations, defaults overwritten before use, `a % b` standing in for a time difference `a - b`, swapped std::min and std::max arguments, `<=` for `==` on values that can't be negative or on an enum's first or last value, iterator `<` for `!=`, `>=` or `<` against an end that find never passes, a `continue;` at the end of a loop body or a `break;` after a `return`, sscanf's count against more than it converts, constants with slack in the calendar formulas or within a test's float tolerance, comparisons with `npos` or within a closed set of names (read_tariff()'s sections, the clock's two values), -2 for the -1 that marks a non-digit or `% '0'` for `- '0'` on a digit in two_digits(), battery levels with a fraction, as a Tesla reports whole percents, and input no market or the ECB sends, like a rate under 0.1, OMIE's header twice or a price off the quarter-hours.
- Look closely at any other survivor: one found a real bug, where a button erased the board's memory of its own start.
