# test/

## Unit tests

- Build and run them from the repository root; the include paths assume it, and a test reads every list in pricelists/.
- charging_test_tesla.h, the simulated Tesla, is also the simulation's car: keep it free of unit-test code.
- CHECK is a macro: a comma outside parentheses, as in `std::vector<std::pair<A, B>>`, splits its argument. Use a type alias.
- A plug-in counts as one (the phone message, a fresh plan mode) only after a tick with the car unplugged: the first plug state the controller sees is taken as the state after a restart.
- On the first tick there's no plan yet, so a start from the car on that tick doesn't switch to charging now, unless Stop charging was in force. Tick once before.
- simulate() ticks every 30 s and can't tell a 2-minute limit from a 1:58 one. Test limits with your own ticks just before and at the boundary.

## The simulation

- Its program needs a terminal and a time limit: `ESPHOME_PREFDIR=$(mktemp -d) python3 -c 'import pty, sys; pty.spawn(sys.argv[1:])' perl -e 'alarm 45; exec @ARGV' test/.esphome/build/simulation/.pioenvs/simulation/program > log`, then `grep -F '][charging' log`. ESPHOME_PREFDIR starts it without saved settings.
- Without CHEAP_NOW=1, the outcome depends on the time of day, as the made-up prices are cheap from 01:00 to 05:00. A first "Wake the car (Reading battery)" is a startup race with the made-up sensors, not a bug, and the start may then wait for the next tick, 30 s later: give the program well over 30 s.

## Mutation testing

- A run takes about half an hour on 8 cores and builds charging_test.cpp for every mutant: don't edit it while a run is going. Survivor line numbers refer to the headers as they were when the run started.
- Around 130 survivors are equivalent; don't chase them: spare buffer sizes, the order of declarations, defaults overwritten before use, `a % b` standing in for a time difference `a - b`, swapped std::min and std::max arguments, `<=` for `==` on values that can't be negative or on an enum's last value, iterator `<` for `!=`, a `continue;` at the end of a loop body, constants with slack in the calendar formulas or within a test's float tolerance, comparisons with `npos` or within a closed set of names (read_grid()'s sections, the clock's two values), and -2 for the -1 that marks a non-digit or `% '0'` for `- '0'` on a digit in two_digits().
- Look closely at any other survivor: one found a real bug, where a button erased the board's memory of its own start.
