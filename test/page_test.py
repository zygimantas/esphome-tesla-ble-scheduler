#!/usr/bin/env python3
"""The page in a headless browser, on the fake board (fake_board.py), from the repository root:
    uvx --from "playwright==1.63.0" python test/page_test.py
It drives Google Chrome, which GitHub's runners and a Mac have, through Playwright, so nothing is downloaded, and needs
test/settings_tool built like the unit tests (CONTRIBUTING.md's Checks). Each scenario starts the board afresh through
its POST /fake, and asserts on the page's text and on the POSTs the board received.
"""

import json
import socket
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path

from playwright.sync_api import expect, sync_playwright

HERE = Path(__file__).resolve().parent
VIN = "5YJ3E1EA6KF000000"  # its 9th character is the check digit the page computes from the others
PLAN = "lt/eso-standartinis-4-zones"
PRICES = f"market:\n  area: LT\n  vat: 0.21\ngrid:\n  plan: {PLAN}\n"
CAR = f"tesla_battery_kwh: 75\ntesla_charging_kw: 11\ntesla_vin: {VIN}\n"
ZONE = "timezone: Europe/Vilnius\n"
SETTINGS = PRICES + CAR + ZONE


class Board:
    """The fake board, started here, and its control: POST /fake sets its state, GET /fake reads it."""

    def __init__(self, port, log):
        self.url = f"http://127.0.0.1:{port}"
        self.process = subprocess.Popen([sys.executable, HERE / "fake_board.py", str(port)], stdout=log, stderr=log)
        for _ in range(50):
            try:
                urllib.request.urlopen(f"{self.url}/fake").close()
                return
            except OSError:
                time.sleep(0.1)
        raise RuntimeError("the fake board didn't start")

    def reset(self, **state):
        request = urllib.request.Request(f"{self.url}/fake", json.dumps(state).encode(), method="POST")
        urllib.request.urlopen(request).close()

    def posts(self):
        """The POSTs received, as (url, body), without the page's check for an update as it opens."""
        with urllib.request.urlopen(f"{self.url}/fake") as answer:
            posts = json.load(answer)["posts"]
        return [(p["url"], p["body"]) for p in posts if p["url"] != "/button/Check for update/press"]


def setup_from_scratch(page, board):
    """A new board's setup: the prices, the phone, the car and its key, then the page."""
    board.reset()
    page.goto(board.url)
    status = page.locator("#status")
    title = page.locator("#setup-card .title")
    expect(status).to_have_text("No settings yet")
    expect(title).to_have_text("Setup: prices")
    page.select_option("#set-area", "LT")
    page.select_option("#set-plan", PLAN)
    page.click("#prices-step .save")
    expect(title).to_have_text("Setup: your car")
    expect(page.locator("#phone-step")).to_be_visible()
    expect(status).to_have_text("No car yet")
    page.click("#here")
    expect(page.locator("#car-step")).to_be_visible()
    page.fill("#set-vin", VIN[:8] + "0" + VIN[9:])  # a check digit that's off
    expect(page.locator("#set-battery")).to_have_value("75")  # the Model 3's, guessed from the VIN
    page.click("#car-step .save")
    expect(page.locator("#car-step > p.error")).to_contain_text("This VIN doesn't add up")
    page.fill("#set-vin", VIN)
    page.click("#car-step .save")
    expect(title).to_have_text("Setup: charging key")
    expect(status).to_have_text("Not paired")
    pair = page.locator("#pair-now")
    expect(pair).to_have_text("Create key")  # once the board has the car's Bluetooth signal
    expect(pair).to_be_disabled()  # until the box is ticked
    page.check("#key-card")
    pair.click()
    expect(page.locator("#key-wait")).to_be_visible()
    expect(pair).to_have_text("Waiting for the car …")
    expect(page.locator("#done-step")).to_contain_text("The setup is done")  # the car took the key
    expect(status).to_have_text("Unplugged")
    page.click("#done")
    expect(page.locator("#setup-card")).to_be_hidden()
    expect(page.locator("#settings-card")).to_be_visible()
    assert board.posts() == [
        ("/settings", PRICES + ZONE),
        ("/settings", SETTINGS),
        ("/button/Pair BLE Key/press", ""),
    ]


def settings_round_trip(page, board):
    """Settings: a save the board restarts for, and its fields after the restart."""
    board.reset(settings=SETTINGS, paired=True)
    page.goto(board.url)
    status = page.locator("#status")
    expect(status).to_have_text("Unplugged")
    page.click("#settings-card .title")
    page.fill("#set-battery", "80")
    page.select_option("#set-price", "fixed")
    page.fill("#set-fixed", "0.15")
    page.click("#save-settings")
    expect(status).to_have_text("Restarting …")
    expect(status).to_have_text("Unplugged")  # the page loaded afresh, as the board's uptime started again
    saved = f"fixed_price: 0.15\ngrid:\n  plan: {PLAN}\n" + CAR.replace("75", "80") + ZONE
    assert board.posts() == [("/settings", saved)]
    page.click("#settings-card .title")
    expect(page.locator("#set-battery")).to_have_value("80")
    expect(page.locator("#set-price")).to_have_value("fixed")
    expect(page.locator("#set-fixed")).to_have_value("0.15")


def schedule_card(page, board):
    """The Schedule card with the car plugged in and no schedule: Create schedule, then Delete schedule."""
    board.reset(settings=SETTINGS, paired=True, plugged=True, hold="none")
    page.goto(board.url)
    status = page.locator("#status")
    expect(status).to_have_text("No schedule")
    expect(page.locator("#create-schedule")).to_be_enabled()
    page.select_option("#limit-select", "90")
    # a Ready by other than the board's, which wouldn't be sent: the first half-hour offered that isn't it
    ready = page.locator("#ready-select")
    deadline = ready.evaluate("select => [...select.options].find(o => !o.disabled && o.value !== select.value).value")
    page.select_option("#ready-select", deadline)
    picked = ready.evaluate("select => select.selectedOptions[0].text.slice(-5)")  # "Thu 07:30" -> "07:30"
    page.click("#create-schedule")
    expect(status).to_contain_text("Charges at")
    expect(page.locator("#windows .row")).to_have_count(1)
    expect(page.locator("#windows")).to_contain_text("EUR/kWh")
    expect(page.locator("#limit-select")).to_have_value("90")
    page.click("#delete-schedule")
    expect(status).to_have_text("No schedule")
    expect(page.locator("#create-schedule")).to_be_visible()
    assert board.posts() == [
        ("/number/Charging Limit/set?value=90", ""),
        (f"/time/Ready by/set?value={picked}:00", ""),
        ("/button/Create schedule/press", ""),
        ("/button/Stop charging/press", ""),
    ]


def update_card(page, board):
    """Update available: the card, Update, Updating …, and the page afresh on the new release."""
    board.reset(settings=SETTINGS, paired=True, update="9.9.9")
    page.goto(board.url)
    card = page.locator("#update-card")
    expect(card).to_be_visible()
    expect(card).to_contain_text("Release 9.9.9 is out")
    page.click("#update")
    status = page.locator("#status")
    expect(status).to_have_text("Updating …")
    expect(status).to_have_text("Unplugged")
    expect(card).to_be_hidden()
    page.click("#settings-card .title")
    expect(page.locator("#version")).to_have_text("9.9.9")
    assert board.posts() == [("/update/Firmware/install", "")]


def restart_board(page, board):
    """Restart board: Restarting …, then the page afresh; and Restart setup, into the setup."""
    board.reset(settings=SETTINGS, paired=True)
    page.goto(board.url)
    status = page.locator("#status")
    expect(status).to_have_text("Unplugged")
    page.click("#settings-card .title")
    page.click("#restart")  # its confirm() is accepted
    expect(status).to_have_text("Restarting …")
    expect(status).to_have_text("Unplugged")
    page.click("#settings-card .title")
    page.click("#restart-setup")
    expect(status).to_have_text("Restarting …")
    expect(status).to_have_text("No settings yet")
    expect(page.locator("#setup-card .title")).to_have_text("Setup: prices")
    assert board.posts() == [("/button/Restart/press", ""), ("/button/Restart setup/press", "")]


SCENARIOS = [setup_from_scratch, settings_round_trip, schedule_card, update_card, restart_board]


def main():
    expect.set_options(timeout=15000)  # a restart takes the board away for a few seconds
    with socket.socket() as free:
        free.bind(("127.0.0.1", 0))
        port = free.getsockname()[1]
    with tempfile.TemporaryFile("w+") as log:
        board = Board(port, log)
        try:
            with sync_playwright() as playwright:
                browser = playwright.chromium.launch(channel="chrome")
                page = browser.new_page()
                page.on("dialog", lambda dialog: dialog.accept())
                errors = []
                page.on("pageerror", lambda error: errors.append(str(error)))
                for scenario in SCENARIOS:
                    scenario(page, board)
                    assert not errors, errors
                    print(f"ok: {scenario.__doc__}")
                browser.close()
        except Exception:
            log.seek(0)
            print(f"The fake board's log:\n{log.read()}", file=sys.stderr)
            raise
        finally:
            board.process.terminate()


if __name__ == "__main__":
    main()
