# Contributing

`main` is always deployable: it's what goes on the board. All work lands through short-lived branches and pull requests.

## The code

`device.yaml` is the device. It sets up the `charging` component in `charging/`: `charging.h` plans and decides (plain C++, unit-tested on a computer), `charging_component.h` and `.cpp` connect it to ESPHome (the page's entities, price downloads, phone messages and the Tesla's entities), and `__init__.py` checks the settings and grid fees when you build. `web/` holds the page, and `test/` the unit tests, the mutation test and the simulation.

A user's `config.yaml` loads `device.yaml` and the component from a release on GitHub. To build from this folder instead, point your `config.yaml` at it: `source: .` for the component and `!include device.yaml` for the package. The time zone is the building computer's; when building in a container, add `timezone:` to the clock with `id: sntp_time` under `time:` in your config.

In `device.yaml`, `ble_mac_address` stays all zeros because the board finds the car by its VIN, `tesla_ble_ref` pins the esphome-tesla-ble version, and `reboot_timeout: 0s` stops restarts every 15 minutes without Home Assistant.

## Branching

1. Branch off `main`. Name branches `type/short-description`, e.g. `feat/postpone-suggestion`, `fix/price-retry`, `docs/grid-fees`, using the commit types below.
2. Keep branches short-lived and scoped to one change.
3. Open a pull request early.

## Checks

CI runs these on every pull request and every push to `main` (see `.github/workflows/ci.yml`). Run them before you push; uv runs the tools without installing them (`brew install uv`):

```sh
# Formatters and linters: clang-format, ruff, Biome for the page, codespell and file hygiene.
# `uvx pre-commit install` runs them on each commit.
uvx pre-commit run --all-files
# Unit tests, with CI's flags and ArduinoJson (ARDUINOJSON_VERSION in ci.yml) on the include path
g++ -std=c++17 -Wall -Wextra -Wshadow -Werror -I . -isystem path/to/ArduinoJson/src test/charging_test.cpp -o charging_test && ./charging_test
# Static analysis with the checks in .clang-tidy (on macOS, add -isysroot $(xcrun --show-sdk-path))
uvx clang-tidy test/charging_test.cpp -- -std=c++17 -Wall -Wextra -Wshadow -Werror -I . -isystem path/to/ArduinoJson/src
# The board's logic on your computer, with a simulated Tesla; CHEAP_NOW=1 START_STOPPED=1 tries the start path
esphome run test/simulation.yaml
# The firmware, from your config.yaml pointed at this folder (see The code, above)
esphome compile config.yaml
```

New behavior comes with a unit test in `test/charging_test.cpp`. Try changes on the simulation before flashing a real board.

The unit tests cover every line and branch of `charging/charging.h`, and CI fails when they don't. Mutation testing shows what they'd still miss:

```sh
# Coverage, with clang (on macOS, run llvm-profdata and llvm-cov through xcrun)
clang++ -std=c++17 -fprofile-instr-generate -fcoverage-mapping -I . -I path/to/ArduinoJson/src test/charging_test.cpp -o charging_test
LLVM_PROFILE_FILE=charging_test.profraw ./charging_test
llvm-profdata merge charging_test.profraw -o charging_test.profdata
llvm-cov report charging_test -instr-profile=charging_test.profdata -show-branch-summary charging/charging.h
# Mutation testing, about half an hour
python3 test/mutation_test.py path/to/ArduinoJson/src
```

A surviving mutant is a change to `charging.h` that no test notices: add a test that does, or remove the code if it makes no difference. Some can't be noticed because they change nothing, such as a spare byte in a buffer or a default that's always overwritten.

The C++ follows ESPHome's own style. Comments say why, not what. YAML config files have no comments.

## Pull requests

- The body says what changed and why, in plain prose, and how you checked it.
- Keep your own values out: git ignores `config.yaml` and `secrets.yaml`, and `config.example.yaml` keeps ESO's grid fees unless the change is about them.
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

**Scopes** (optional but preferred) follow the repository layout: `charging` (the component in `charging/`), `web` (the page in `web/`), `board` (`device.yaml` and the example files), `grid` (the grid fees in `config.example.yaml`) and `docs`. Tests take the scope of what they test.

Write the summary in the imperative mood, lower case, with no trailing period:

```
feat(charging): suggest a later ready-by when it's much cheaper
fix(web): keep ready by hidden until prices arrive
chore(grid): update the ESO fees for 2027
```

## Releases

Releases are what users install, numbered vMAJOR.MINOR.PATCH: a new major number when users must edit their settings files, a new minor one for new behavior and a new patch one for fixes. To release, set `version` under `project:` in `device.yaml` and `version` in `config.example.yaml`, and merge that as `chore(board): release v0.2.0`: once CI passes on `main`, it tags the merge and publishes the release with `config.example.yaml` and `secrets.example.yaml`, which users download, and notes listing the merged pull requests. For a new major number, add to the notes what users must change in their settings files. Users get it when they change their `version`.
