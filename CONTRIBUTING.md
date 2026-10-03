# Contributing

`main` is always deployable: it's what goes on the board. All work lands through short-lived branches and pull requests.

## The code

`device.yaml` is the device. It sets up the `scheduler` component in `scheduler/`: `charger.h` decides, with `calendar.h`, `market.h`, `savings.h`, `schedule.h` and `tariff.h` for dates, Nord Pool prices, the savings, the schedule and the tariff (plain C++, unit-tested on a computer), `scheduler_component.h` and `.cpp` connect it to ESPHome (the page's entities, price and plan downloads, phone messages and the Tesla's entities), and `__init__.py` and `tariff.py` check the settings and the plan when you build. `plans/` holds the grid operators' plans, `web/` the page, and `test/` the unit tests, the mutation test and the simulation.

A user's `config.yaml` loads `device.yaml` and the component from a release on GitHub. To build from this folder instead, point your `config.yaml` at it: `source: .` for the component and `!include device.yaml` for the package.

In `device.yaml`, `ble_mac_address` stays all zeros because the board finds the car by its VIN, `tesla_ble_ref` pins the esphome-tesla-ble version, `reboot_timeout: 0s` stops restarts every 15 minutes without Home Assistant, and `scan_parameters: continuous: true` with the two `!remove` lines under `wifi:` undo the package's single-core workaround, which stopped scanning for the car whenever Wi-Fi was down.

## Branching

1. Branch off `main`. Name branches `type/short-description`, e.g. `feat/postpone-suggestion`, `fix/price-retry`, `docs/tariff`, using the commit types below.
2. Keep branches short-lived and scoped to one change.
3. Open a pull request early.

## Checks

CI runs these on every pull request and every push to `main` (see `.github/workflows/ci.yml`). Run them before you push; uv runs the tools without installing them (`brew install uv`):

```sh
# Formatters and linters: clang-format, ruff, Biome for the page, codespell and file hygiene.
# `uvx pre-commit install` runs them on each commit.
uvx pre-commit run --all-files
# Unit tests, with CI's flags and ArduinoJson (ARDUINOJSON_VERSION in ci.yml) on the include path
g++ -std=c++17 -Wall -Wextra -Wshadow -Werror -I . -isystem path/to/ArduinoJson/src test/scheduler_test.cpp -o scheduler_test && ./scheduler_test
# Static analysis with CI's clang-tidy (CLANG_TIDY_VERSION in ci.yml) and the checks in .clang-tidy; on macOS, add
# -isysroot $(xcrun --show-sdk-path)
uvx "clang-tidy==22.1.8" test/scheduler_test.cpp -- -std=c++17 -Wall -Wextra -Wshadow -Werror -I . -isystem path/to/ArduinoJson/src
# The board's logic on your computer, with a simulated Tesla; CHEAP_NOW=1 START_STOPPED=1 tries the start path
esphome run test/simulation.yaml
# The firmware, from your config.yaml pointed at this folder (see The code, above)
esphome compile config.yaml
```

New behavior comes with a unit test in `test/scheduler_test.cpp`. Try changes on the simulation before flashing a real board.

The unit tests cover every line and branch of those six headers, and CI fails when they don't. Mutation testing shows what they'd still miss:

```sh
# Coverage, with clang (on macOS, run llvm-profdata and llvm-cov through xcrun)
clang++ -std=c++17 -fprofile-instr-generate -fcoverage-mapping -I . -I path/to/ArduinoJson/src test/scheduler_test.cpp -o scheduler_test
LLVM_PROFILE_FILE=scheduler_test.profraw ./scheduler_test
llvm-profdata merge scheduler_test.profraw -o scheduler_test.profdata
llvm-cov report scheduler_test -instr-profile=scheduler_test.profdata -show-branch-summary scheduler/*.h
# Mutation testing, about 40 minutes
python3 test/mutation_test.py path/to/ArduinoJson/src
```

A surviving mutant is a change to one of the headers that no test notices: add a test that does, or remove the code if it makes no difference. Some can't be noticed because they change nothing, such as a spare byte in a buffer or a default that's always overwritten.

The C++ follows ESPHome's own style. Comments say why, not what. YAML config files have no comments; plans have theirs.

## Pull requests

- The body says what changed and why, in plain prose, and how you checked it.
- Keep your own values out: git ignores `config.yaml` and `secrets.yaml`, and `config.example.yaml` keeps ESO's plan unless the change is about it.
- Pull requests merge by squash only, and the head branch is deleted on merge. Only the squash commit reaches `main`.

## Commit and PR-title format

The PR title becomes the commit subject on `main`, so it follows [Conventional Commits](https://www.conventionalcommits.org/): `type(scope): summary`. Local commits on a branch can be informal.

**Types**

| Type       | Use for                            |
| ---------- | ---------------------------------- |
| `feat`     | New user-facing behavior           |
| `fix`      | Bug fix                            |
| `refactor` | Behavior-preserving restructuring  |
| `perf`     | Performance improvement            |
| `docs`     | Documentation only                 |
| `test`     | Tests only                         |
| `build`    | Build, dependencies, packaging     |
| `chore`    | Maintenance with no product impact |
| `revert`   | Reverting a previous change        |

**Scopes** (optional but preferred) follow the repository layout: `scheduler` (the component in `scheduler/`), `web` (the page in `web/`), `board` (`device.yaml` and the example files), `plans` (the plans in `plans/`) and `docs`. Tests take the scope of what they test.

A workflow labels the pull request from the type: `fix` is bug, `feat` is enhancement, `docs` is documentation, and the rest maintenance. A `!` before the colon, as in `feat(board)!: ...`, marks a change users must act on, and adds breaking. The release notes list breaking changes first, group the rest by those labels and leave maintenance out.

Write the summary in the imperative mood, lower case, with no trailing period:

```
feat(scheduler): suggest a later ready-by when it's much cheaper
fix(web): keep ready by hidden until prices arrive
chore(plans): update ESO's plans for 2027
```

## Plans

A plan is a grid operator's prices and hours, in `plans/`, in a folder for its country: `plans/lt/eso-standartinis-4-zones.yaml`. Its name says the operator, the plan and, where the plan comes in several, the number of zones or rates.

- **The format** is that of `tariff:` in [Tariff](docs/tariff.md), with the plan's `currency` too. Boards read the file as plain text: two-space indents, comments on lines of their own, no quotes, and a line break at the end.
- **A comment at the top** says what the plan is, with a link to the operator's prices, and who maintains it: `# Maintained by @your-github-name`.
- **The maintainer updates it every January,** and whenever prices change: the prices, and the dates of holidays that move, like Easter Monday. Merge the change on the day the prices start. Boards download their plan from `main` every day, so merging publishes it, without a release.
- **CI checks every plan** in the unit tests: as the board reads it, and by the install's own rules.

## Releases

Releases are what users install, numbered vMAJOR.MINOR.PATCH: a new major number when users must edit their settings files, a new minor one for new behavior and a new patch one for fixes. To release, set `version` under `project:` in `device.yaml` and `version` in `config.example.yaml`, and merge that as `chore(board): release v0.2.0`: once CI passes on `main`, it tags the merge and publishes the release with `config.example.yaml` and `secrets.example.yaml`, which users download, and notes listing the merged pull requests. Add to the notes anything users must do, such as settings to change for a new major number. Users get it when they change their `version`.
