# .github/

- Check that an action's tag exists before using it: `gh api repos/<owner>/<repo>/git/refs/tags/<tag>`. astral-sh/setup-uv publishes only full versions such as v10.2.0, no v10.
- Lint workflow changes: `uvx --with shellcheck-py --from actionlint-py actionlint .github/workflows/ci.yml`.
- The ESPHome job takes about 6 minutes and isn't cached on purpose: a cached build folder keeps the paths of the previous run's temporary tools (ninja in uvx's environment), and the next build fails.
- The versions in the workflows' `env` go together: ESPHOME_VERSION (in ci.yml and codeql.yml, and `min_version` in device.yaml) is the owner's Homebrew ESPHome, and ARDUINOJSON_VERSION (in ci.yml and codeql.yml) the ArduinoJson that this ESPHome uses (after a firmware build, in the folder name .esphome/.espressif/service_*/bblanchon__arduinojson_<version>_*). Update them together when ESPHome changes.
- CI publishes a release when it passes on main with a device.yaml version that has none yet, so bump that version only when the owner asks for a release.
- CI runs only the simulation's CHEAP_NOW scenario, as the other one depends on the time of day.
- To reproduce the unit-test job, run its steps in Docker's ubuntu:24.04 on a copy of the repository, as the builds write files.
- `gh run list` shows a new run only a few seconds after the push.
- label.yml runs on pull_request_target with a write token, so it must never check out or run the pull request's code: it only reads the title and adds a label. release.yml groups the generated release notes by those labels.
