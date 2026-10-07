# Contributing

`main` is always deployable: it's what goes on the board. All work lands through short-lived branches and pull requests.

## The code

`device.yaml` is the device. It sets up the `scheduler` component in `scheduler/`: `charger.h` decides, with `calendar.h`, `market.h`, `savings.h`, `schedule.h` and `tariff.h` for dates, market prices from Nord Pool, SMARD and OMIE, the savings, the schedule and the plan, and `settings.h` reads and checks the settings file, which the page writes and POSTs to `/settings`, with a custom plan under `tariff:` (plain C++, unit-tested on a computer). `scheduler_component.h` and `.cpp` connect them to ESPHome (the settings file, the page's entities, price and plan downloads, phone messages and the Tesla's entities), and `__init__.py` builds in the plans. `plans/` holds the grid operators' plans, `web/` the page, and `test/` the unit tests, the mutation test and the simulation.

Releases ship one firmware for every board, which CI builds from `release.yaml`: `device.yaml` with the component of this folder, and nothing of a user's, who sets Wi-Fi with ESPHome Web and enters the settings on the page. For a build of your own, start from `config.example.yaml`, which loads both from a release, or point a `config.yaml` at this folder like `release.yaml` does, and add what you need, like your Wi-Fi, an API key and OTA from your computer.

In `device.yaml`, `ble_mac_address` stays all zeros because the board finds the car by its VIN, `tesla_ble_ref` pins the esphome-tesla-ble version, `reboot_timeout: 0s` stops restarts every 15 minutes without Home Assistant, and `scan_parameters: continuous: true` with the two `!remove` lines under `wifi:` undo the package's single-core workaround, which stopped scanning for the car whenever Wi-Fi was down. `- id: !remove homeassistant_time` drops the package's Home Assistant clock, which would otherwise set the time zone from the build computer or from Home Assistant. `tesla_vin` stays empty and the clock on UTC until the board has its settings: it reads them as it starts, and while it has no car, as in the setup, it takes the page's at once, the time zone with the prices and the VIN with the car. `improv_serial:` takes Wi-Fi from ESPHome Web over USB, and for 15 minutes after the board starts (`provisioning:`) the package's hotspot opens when it can't join Wi-Fi, and the first client to connect, like Home Assistant, can set the API's key, as `encryption:` has none. `ota: !remove` drops the package's OTA from a computer, which would need a key built in.

## Branching

1. Branch off `main`. Name branches `type/short-description`, e.g. `feat/postpone-suggestion`, `fix/price-retry`, `docs/plans`, using the commit types below.
2. Keep branches short-lived and scoped to one change.
3. Open a pull request early.

## Checks

CI runs these on every pull request and every push to `main` (see `.github/workflows/ci.yml`). Run them before you push; uv runs the tools without installing them (`brew install uv`):

```sh
# ArduinoJson, as CI gets it (ARDUINOJSON_VERSION in ci.yml)
mkdir ArduinoJson && curl -sSfL https://github.com/bblanchon/ArduinoJson/archive/refs/tags/v7.4.3.tar.gz | tar xz --strip-components=1 -C ArduinoJson
# Formatters and linters: clang-format, ruff, Biome for the page, actionlint for the workflows, codespell and file
# hygiene. `uvx pre-commit install` runs them on each commit.
uvx pre-commit run --all-files
# Unit tests, with CI's flags
g++ -std=c++17 -Wall -Wextra -Wshadow -Werror -I . -isystem ArduinoJson/src test/scheduler_test.cpp -o scheduler_test && ./scheduler_test
# Static analysis with CI's clang-tidy (CLANG_TIDY_VERSION in ci.yml) and the checks in .clang-tidy; on macOS, add
# -isysroot $(xcrun --show-sdk-path)
uvx "clang-tidy==22.1.8" test/scheduler_test.cpp -- -std=c++17 -Wall -Wextra -Wshadow -Werror -I . -isystem ArduinoJson/src
# The board's logic on your computer, with a simulated Tesla; CHEAP_NOW=1 START_STOPPED=1 tries the start path
esphome run test/simulation.yaml
# The firmware, from your config.yaml pointed at this folder (see The code, above)
esphome compile config.yaml
```

New behavior comes with a unit test in `test/scheduler_test.cpp`. Try changes on the simulation before flashing a real board.

The unit tests cover every line and branch of `charger.h`, `settings.h` and the headers they include, and CI fails when they don't. Mutation testing shows what they'd still miss:

```sh
# Coverage, with clang (on macOS, run llvm-profdata and llvm-cov through xcrun)
clang++ -std=c++17 -fprofile-instr-generate -fcoverage-mapping -I . -isystem ArduinoJson/src test/scheduler_test.cpp -o scheduler_test
LLVM_PROFILE_FILE=scheduler_test.profraw ./scheduler_test
llvm-profdata merge scheduler_test.profraw -o scheduler_test.profdata
llvm-cov report scheduler_test -instr-profile=scheduler_test.profdata -show-branch-summary scheduler/*.h
# Mutation testing, about 40 minutes
python3 test/mutation_test.py ArduinoJson/src
```

A surviving mutant is a change to one of the headers that no test notices: add a test that does, or remove the code if it makes no difference. Some can't be noticed because they change nothing, such as a spare byte in a buffer or a default that's always overwritten.

The C++ follows ESPHome's own style. Comments say why, not what. `device.yaml`, `release.yaml` and `config.example.yaml` have no `#` comments; plans have theirs.

## Pull requests

- The body says what changed and why, in plain prose, and how you checked it.
- Keep your own values out: git ignores `config.yaml`, `secrets.yaml` and `settings.yaml`.
- Pull requests merge by squash only, and the head branch is deleted on merge. Only the squash commit reaches `main`.

## Commit and PR-title format

The PR title becomes the commit subject on `main`, so it follows [Conventional Commits](https://www.conventionalcommits.org/): `type(scope): summary`. Local commits on a branch can be informal.

**Types**

| Type       | Use for                                |
| ---------- | -------------------------------------- |
| `feat`     | New user-facing behavior               |
| `fix`      | Bug fix                                |
| `refactor` | Behavior-preserving restructuring      |
| `perf`     | Performance improvement                |
| `docs`     | Documentation only                     |
| `test`     | Tests only                             |
| `build`    | Build, dependencies, packaging         |
| `ci`       | CI workflows and repository automation |
| `chore`    | Maintenance with no product impact     |
| `revert`   | Reverting a previous change            |

**Scopes** (optional but preferred) follow the repository layout: `scheduler` (the component in `scheduler/`), `web` (the page in `web/`), `board` (`device.yaml`, `release.yaml` and `config.example.yaml`), `plans` (the plans in `plans/`) and `docs`. Tests take the scope of what they test.

A workflow labels the pull request from the type: `fix` is bug, `feat` is enhancement, `docs` is documentation, and the rest maintenance. A `!` before the colon, as in `feat(board)!: ...`, marks a change users must act on, and adds breaking. The release notes list breaking changes first, group the rest by those labels and leave maintenance out.

Write the summary in the imperative mood, lower case, with no trailing period:

```
feat(scheduler): suggest a later ready-by when it's much cheaper
fix(web): keep ready by hidden until prices arrive
chore(plans): update ESO's plans for 2027
```

## Plans

A plan is a grid operator's prices and hours, in `plans/`, in a folder for its country, asked for with the Grid plan issue form or added by anyone: `plans/lt/eso-standartinis-4-zones.yaml`. Its name says the operator, the plan and, where the plan comes in several, the number of zones or rates.

- **The format** is the one [Custom plan](plans/README.md#custom-plan) describes, with the plan's `currency`. Boards read the file as plain text: two-space indents, comments on lines of their own, no quotes, and a line break at the end.
- **The prices** are the operator's fees per kWh, with VAT, in its currency. Monthly fees, fees per kW and the charges that are the same every hour, like taxes, stay out, and the folder's `README.md` says which.
- **The folder's `README.md`** is the country's page for people, which GitHub shows under the folder's files: which plans there are and for whom, their hours, what's left out, and who sets the prices and when. A new plan adds its row there, and a new country's folder its row in the table of [Countries and plans](plans/README.md#countries). Boards download only the `.yaml` files.
- **`name`** names the plan for the page's list, short enough for a phone, like `name: ESO Standartinis, four zones`.
- **Comments at the top** link the operator's prices, like `# Prices with VAT: https://www.eso.lt`, and say who maintains it: `# Maintained by @your-github-name`. Where the operator prints its prices otherwise, like in cents or without VAT, they also say how they were converted.
- **The maintainer updates it every January,** and whenever prices change: the prices, and the dates of holidays that move, like Easter Monday. Merge the change on the day the prices start. Boards download their plan from `main` every day, so merging publishes it, without a release.
- **A new plan reaches users with the next release,** as the page lists only the plans built into the board's release. Until then, they can upload it as a custom plan.
- **CI checks every plan** in the unit tests: as the board reads it, and as the settings name it.

## Releases

Releases are what users install, numbered vMAJOR.MINOR.PATCH: a new major number when users must act, like entering their settings again, a new minor one for new behavior and a new patch one for fixes. To release, run the Release workflow in the repository's Actions tab, choosing major, minor or patch. It pushes a commit like `chore(board): release v4.2.0` to `main`, which sets the version in `device.yaml` and `config.example.yaml`, with a deploy key that `main`'s ruleset lets skip pull requests. CI on `main` then sees a version without a release, tags that commit and publishes the release with notes listing the merged pull requests, the firmware it built for ESPHome Web, `esphome-tesla-ble-scheduler.bin`, which users download, and the update that boards offer on their page: `esphome-tesla-ble-scheduler.ota.bin` and `manifest.json`. Add to the notes anything users must do, such as settings to change for a new major number.

Every board checks `manifest.json` of the latest release once each time its page opens, which presses `release.yaml`'s **Check for update**, and never by itself (`update_interval: never`, as with an interval ESPHome also checks at every start), and the page then offers the new version at the top, under **Update available**: it installs only when the user presses **Update**. Try a release on a board of your own first. Boards take only firmware signed with the key of the `SIGNING_KEY` secret, an RSA key of 3072 bits, which CI signs `release.yaml` with: without that key, no board can take an update except over USB. Only builds of `main` get that secret: CI signs pull requests with a throwaway key. `release.yaml` also gives HTTP requests buffers of 2048 bytes, as GitHub redirects a release's downloads to an address of about 900 characters. `release.yaml` also gives the Tesla package's OTA from a computer a key that nobody keeps (`-s ota_key`), as ESPHome adds that OTA back once `release.yaml` lists its own.

To put a build of this folder on a board running a release, sign it with the same key: copy the key into `signing_key.pem` here, which git ignores, and build `release.yaml` with your `config.yaml`'s additions, or add its `signed_ota_verification:` to your `config.yaml`. Leave its `update:` out of a test build, or its page may offer the release as an update.
