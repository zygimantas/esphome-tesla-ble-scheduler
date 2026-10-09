# Notes for AI agents

What the code, README.md and CONTRIBUTING.md don't say. Each folder has its own AGENTS.md.

## Working with the owner

- Work on a branch with a pull request, as CONTRIBUTING.md says, and commit and push to it without asking. Merge, flash or deploy only when asked.
- Less code is better. Delete what nothing uses rather than keeping it just in case, keep files and lists in a consistent order, and say what you removed so the owner can object. Don't bring back removed readouts or settings unless asked.
- Keep answers short. No en or em dashes in anything a user reads: the page, phone messages, the docs.
- No AI attribution: no Co-Authored-By trailer in commits and no "Generated with" line in pull requests.
- `gh pr create` and `gh issue create` skip GitHub's templates, so write what they would ask for. A pull request body has the two headings of .github/PULL_REQUEST_TEMPLATE.md, "What changed and why" and "How it was checked". An issue has the fields of the matching form in .github/ISSUE_TEMPLATE as headings: for a bug, What happened, Release, Car, charger and area, What the page showed, Log; for an improvement, The problem, The proposal, Who it helps; for a grid plan, Country or area, Grid operator, Plan, Prices and hours. Label it bug or enhancement.
- The owner's names: "charging", not "smart charging"; "market" for the day-ahead prices from Nord Pool, SMARD, OMIE, OKTE or SEMOpx (`market:`), and "plan" for what comes on top of them by the hour, usually a grid operator's fees: a grid operator's, like "ESO's Standartinis plan", kept in plans/ (`grid: plan:`), or a custom one in the same format, uploaded on the page with Upload custom plan; "prices" are those; the board's is a "schedule", as in the Tesla app; a "slot" is one quarter-hour and a "window" a run of them; car settings start with `tesla_`. The docs say "plan", not "tariff", by request: "tariff" stays for the code (tariff.h) and names on bills, like Tidstariff.
- README.md, docs/status.md, plans/README.md (the countries and the plans' format) and each country's README.md next to its plans, by request, are for non-technical users: short, plain and in English only (a translation was removed on request).
- Install tools with Homebrew or mise, never pip; run one-off tools with uvx or npx.

## The live board

It charges the owner's car every night.

- Never change its settings (Ready by, charge limit, buttons, switches) to try something. Read instead: `curl http://<board>/events` streams every entity, and `curl "http://<board>/text_sensor/Charging%20status"` returns one. Try writes on the simulation (test/) or a page mock (web/).
- The owner's settings.yaml, config.yaml and secrets.yaml hold real values (the VIN and the ntfy topic in settings.yaml, Wi-Fi and the API key in secrets.yaml), and git ignores them. The board's /settings answers its settings file, VIN and topic included, and takes the owner's settings.yaml POSTed with curl, as the page does, as long as the file has only what the page writes. Never print them, the output of `esphome config` on config.yaml or the generated main.cpp, which has the Wi-Fi password and API key in plain text because the firmware needs them; edit the files with sed, without printing.
- Secrets stay in secrets.yaml, read with `!secret`, so config.yaml can be shown. ESPHome masks none of them here: each remote package it loads, as device.yaml loads one, empties its list of secrets, so `esphome config`, its failed-config block and main.cpp's comments show them in clear.
- The owner's private repository, where this project started, has those values in its history: never make it public or push its history anywhere public. The public repository started from a fresh history.
- Deploy with `esphome run config.yaml --device <board IP> --no-logs`, with ci.yml's ESPHOME_VERSION (`uvx "esphome@<version>"` while Homebrew's is behind it, as with the 2026.10 beta). Read Ready by, Ready by once and Charging mode before and after: they should match. Uptime starts again from 0, and Prices until comes back within about 10 s.
- The Version sensor (ESPHome's, with the config hash; Release is the project version) changes only with the YAML. To see whether a C++-only change went out, look for a new string in the build: `LC_ALL=C grep -a -c -F '<string>' .esphome/build/tesla/build/firmware.ota.bin`.
- `esphome logs` never exits; give it a time limit (`perl -e 'alarm 30; exec @ARGV' esphome logs config.yaml --device <board IP>`). After a crash, ESPHome replays its report (reason, PC, backtrace) once, to the first `esphome logs` connection: save that output, and read it before flashing another build, as the addresses belong to the build that crashed.

## How users get it

- A change to device.yaml reaches users only with a release (CONTRIBUTING.md's The code says how releases and custom builds are made). The owner's config.yaml points at this folder (`source: .` for the component, `!include device.yaml`) and adds Wi-Fi, an API key and OTA.
- A change in plans/ reaches every board within a day, without a release (CONTRIBUTING.md's Plans), so it has to read with the code that's out: a format change needs a new folder.
- In device.yaml, a relative path that's missing next to a custom build's config.yaml resolves next to device.yaml (the page files), but a local `external_components` path always resolves in the config's folder: that's why the component's source is in config.yaml and release.yaml, not in device.yaml.

## Settled; don't propose again

- The board's key stays `role: CHARGING_MANAGER`, without a DRIVER key or PIN to Drive: solve wake and charge problems within that role.
- No password on the page: the owner trusts their network.
- No learning the battery size or charging power from the car: the first schedule would be wrong.
- No charging the part above 80% late, just before Ready by.
- No limit on the number of windows.
- No guessed prices: a schedule uses only published quarter-hours and waits for the rest.
- No ranking slots by the fraction the car would use of them: that pushes the partly used slot and the spare one to the dear edge of the trough instead of keeping them in it.
- No settings file for users: setting the board up on the page is easy, and the only file left to upload is a custom plan (Upload custom plan). The settings file holds only what the page writes: no Download or Upload settings, no settings the page doesn't show (like ntfy_server or a currency of one's own), and nothing merged over a plan from the list.

Parked for later, as issues #12, #25 and #76: a per-start penalty, so the scheduler splits charging only when the saving beats a pause (about 2 ct), a phone message at plug-in when a later Ready by is much cheaper (the threshold is undecided), and charging from the solar surplus, which pays only where the car is home in daylight and grid power costs far more than an exported kWh earns, not with a morning Ready by and night charging, nor where a home stores its surplus in the grid for a fee, as in Lithuania; on sunny days the market price drops at midday anyway, which an evening Ready by catches.

## The owner's Mac

- zsh doesn't split `$flags` into words: use `${=flags}` or an array. It also reads `$sha:r` as a modifier, so write `${sha}:refs/heads/main`.
- sed and grep are BSD's: `sed -i ''`, and `grep -F` for patterns like `][scheduler`.
- `script -q` fails when its output is redirected. Give a program a terminal with `python3 -c 'import pty, sys; pty.spawn(sys.argv[1:])' <command>`.
- Docker runs `ubuntu:24.04`, which has CI's compilers. Run clang-tidy there too before pushing C++: `int64_t` is `long long` on macOS and `long` on Linux, so some checks fire only in CI.
