# Security

## Supported versions

Only the latest release gets fixes. Updating is one line in `config.yaml` (see [Settings](README.md#settings) in the README), so there is no reason to stay on an older one.

## Reporting a vulnerability

Please don't open a public issue for a security problem. Report it privately through GitHub: [Report a vulnerability](https://github.com/zygimantas/esphome-tesla-ble-scheduler/security/advisories/new). Say what you found, how to reproduce it, and what it lets an attacker do.

You get an answer within a week. A confirmed problem is fixed in a release, with credit to you in the release notes if you want it.

## What counts

The firmware (`device.yaml`, `scheduler/`, `web/`), the example settings files and the GitHub workflows are in scope. Examples of things worth reporting:

- The board sending the car a command it shouldn't, or accepting one from someone who isn't on your Wi-Fi.
- Your Wi-Fi password, API key, VIN or ntfy topic ending up somewhere they shouldn't, such as a log, the page or a release file.
- A Nord Pool or ntfy response, or a crafted web request, crashing the board or running code on it.
- A workflow that could publish a release or change the repository from a pull request.

## What doesn't

- The page has no password: anyone on your Wi-Fi can use it. That is a design choice, documented in the README.
- Someone with physical access to the board or its USB port.
- Reading the phone messages with a guessed ntfy topic: the README says to use a long random name.
- Problems in ESPHome or esphome-tesla-ble themselves: report those to their projects, but tell us too if this project's settings make them worse.
