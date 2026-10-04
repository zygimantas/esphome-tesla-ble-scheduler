# .github/

- Actions are pinned to a full commit hash with the version as a trailing comment, the form Dependabot updates in one go; to add or move one, resolve the tag with `gh api repos/<owner>/<repo>/git/ref/tags/<tag>` (an annotated tag needs a second call to `git/tags/<sha>` for the commit). astral-sh/setup-uv publishes only full versions such as v10.2.0, no v10.
- SonarCloud flags a new line that runs ESPHome through uvx without `--no-build` (githubactions:S8541). ESPHome can't take it, as paho-mqtt 1.6.1 has no wheels: the owner marks the finding safe in SonarCloud.
- The ESPHome job takes about 6 minutes and isn't cached on purpose: a cached build folder keeps the paths of the previous run's temporary tools (ninja in uvx's environment), and the next build fails.
- The versions in the workflows' `env` go together: ESPHOME_VERSION (in ci.yml and codeql.yml, and `min_version` in device.yaml) is the owner's Homebrew ESPHome, and ARDUINOJSON_VERSION (in ci.yml and codeql.yml) the ArduinoJson that this ESPHome uses (after a firmware build, in the folder name .esphome/.espressif/service_*/bblanchon__arduinojson_<version>_*). Update them together when ESPHome changes.
- CI publishes a release when it passes on main with a device.yaml version that has none yet, so bump that version only when the owner asks for a release. Every board installs it by itself within a day, signed with the SIGNING_KEY secret: CI on main fails without it rather than publish firmware signed with another key, which boards would refuse.
- CI runs only the simulation's CHEAP_NOW scenario, as the other one depends on the time of day.
- To reproduce the unit-test job, run its steps in Docker's ubuntu:24.04 on a copy of the repository, as the builds write files.
- `gh run list` shows a new run only a few seconds after the push.
- label.yml runs on pull_request_target with a write token, so it must never check out or run the pull request's code: it only reads the title and adds a label. release.yml groups the generated release notes by those labels.
