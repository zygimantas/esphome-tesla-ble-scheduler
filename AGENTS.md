# Notes for AI agents

What the code, README.md and CONTRIBUTING.md don't say. Each folder has its own AGENTS.md.

## Working with the owner

- Work on a branch with a pull request, as CONTRIBUTING.md says, and commit and push to it without asking. Merge, flash or deploy only when asked.
- Less code is better. Delete what nothing uses rather than keeping it just in case, keep files and lists in a consistent order, and say what you removed so the owner can object. Don't bring back removed readouts or settings unless asked.
- Keep answers short. No en or em dashes in anything a user reads: the page, phone messages, the docs.
- No AI attribution: no Co-Authored-By trailer in commits and no "Generated with" line in pull requests.
- `gh pr create` and `gh issue create` skip GitHub's templates, so write what they would ask for. A pull request body has the two headings of .github/PULL_REQUEST_TEMPLATE.md, "What changed and why" and "How it was checked". An issue has the fields of the matching form in .github/ISSUE_TEMPLATE as headings: for a bug, What happened, Release, Car, charger and area, What the page showed, Log; for an improvement, The problem, The proposal, Who it helps. Label it bug or enhancement.
- The owner's names: "charging", not "smart charging"; "grid" for the network's fees (`grid: rates:`) and "market" for Nord Pool's prices (`market:`), not "tariff"; "prices" are Nord Pool prices; a "price list" is a grid operator's plan in pricelists/ (`grid: pricelist:`), and "plan" means only the charging plan; a "slot" is one quarter-hour and a "window" a run of them; car settings start with `tesla_`.
- README.md and docs/ are for non-technical users: short, plain and in English only (a translation was removed on request).
- Install tools with Homebrew or mise, never pip; run one-off tools with uvx or npx.

## The live board

It charges the owner's car every night.

- Never change its settings (Ready by, charge limit, buttons, switches) to try something. Read instead: `curl http://<board>/events` streams every entity, and `curl "http://<board>/text_sensor/Charging%20status"` returns one. Try writes on the simulation (test/) or a page mock (web/).
- The owner's config.yaml and secrets.yaml hold real values (the VIN, the ntfy topic, Wi-Fi, the API key), and git ignores them. Never print them, the output of `esphome config` on any config that includes this folder's device.yaml (its `!secret` reads the secrets.yaml next to it first, even from a config elsewhere) or the generated main.cpp, which has the Wi-Fi password and API key in plain text because the firmware needs them; edit the files with sed, without printing.
- Secrets stay in secrets.yaml, read with `!secret`: ESPHome masks only those in `esphome config` and in the settings echoed in main.cpp's comments. Moved into settings, they leak into both, which once put them in a chat.
- The owner's private repository, where this project started, has those values in its history: never make it public or push its history anywhere public. The public repository started from a fresh history.
- Deploy with `esphome run config.yaml --device <board IP> --no-logs`. Read Ready by, Ready by once and Charging mode before and after: they should match. Uptime starts again from 0, and Prices until comes back within about 10 s.
- The Version sensor (ESPHome's, with the config hash; Release is the project version) changes only with the YAML. To see whether a C++-only change went out, look for a new string in the build: `LC_ALL=C grep -a -c -F '<string>' .esphome/build/tesla/build/firmware.ota.bin`.
- `esphome logs` never exits; give it a time limit (`perl -e 'alarm 30; exec @ARGV' esphome logs config.yaml --device <board IP>`). After a crash, ESPHome replays its report (reason, PC, backtrace) once, to the first `esphome logs` connection: save that output, and read it before flashing another build, as the addresses belong to the build that crashed.

## How users get it

- Users' config.yaml loads device.yaml and the scheduler component from a release tag on GitHub; the owner's points at this folder instead (`source: .` for the component, `!include device.yaml`). A change to device.yaml reaches users only with a release.
- Boards download their price list from pricelists/ on `main` every day, while the build checks the one in the release it installs. A change there reaches every board within a day, without a release, so it has to read with the code that's out: a format change needs a new folder.
- In device.yaml, a relative path that's missing next to the user's config.yaml resolves next to device.yaml (the page files), but a local `external_components` path always resolves in the user's folder: that's why the component's source is in config.yaml, not in device.yaml. It's also why users download only config.example.yaml and secrets.example.yaml, attached to each release: a web/ folder next to their config.yaml would win over the release's page.

## Settled; don't propose again

- The board's key stays `role: CHARGING_MANAGER`, without a DRIVER key or PIN to Drive: solve wake and charge problems within that role.
- No password on the page: the owner trusts their network.
- No learning the battery size or charging power from the car: the first plan would be wrong.
- No charging the part above 80% late, just before Ready by.
- No limit on the number of windows.
- No guessed prices: a plan uses only published quarter-hours and waits for the rest.
- No ranking slots by the fraction the car would use of them: that pushes the partly used slot and the spare one to the dear edge of the trough instead of keeping them in it.

Parked for later, as issues #12 and #25: a per-start penalty, so the planner splits charging only when the saving beats a pause (about 2 ct), and a phone message at plug-in when a later Ready by is much cheaper (the threshold is undecided).

## The owner's Mac

- zsh doesn't split `$flags` into words: use `${=flags}` or an array. It also reads `$sha:r` as a modifier, so write `${sha}:refs/heads/main`.
- sed and grep are BSD's: `sed -i ''`, and `grep -F` for patterns like `][scheduler`.
- `script -q` fails when its output is redirected. Give a program a terminal with `python3 -c 'import pty, sys; pty.spawn(sys.argv[1:])' <command>`.
- Docker runs `ubuntu:24.04`, which has CI's compilers.
