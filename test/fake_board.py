#!/usr/bin/env python3
"""A fake board for the page, without a board: python3 test/fake_board.py [port] [settings file], then open
http://127.0.0.1:8765. page_test.py drives it in a headless browser.

It serves the page as the board does, web/web_ui.js as /0.js and web/web_ui.css as /0.css, and answers the page's
requests: /events, /settings, /settings/options and the entities' REST routes. A settings file goes through the board's
own checks in settings_tool (settings_tool.cpp, built next to this file or named with --tool). Of the board it models
what the page needs: a status that follows the settings, a car that takes the key, plugs in and follows the schedule
buttons, a restart, which drops the connections and starts the uptime again, and the update entity. POST /fake sets its
state, as page_test.py does, and GET /fake answers it, with the POSTs received.
"""

import argparse
import http.server
import json
import re
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path
from urllib.parse import parse_qs, unquote, urlparse

ROOT = Path(__file__).resolve().parent.parent
RELEASE = re.search(r"^    version: (\S+)$", (ROOT / "device.yaml").read_text(), re.M)[1]
INDEX = (
    '<!DOCTYPE html><html><head><meta charset="utf-8"><link rel="stylesheet" href="/0.css"></head>'
    '<body><script src="/0.js"></script></body></html>'
)
TEXT = "text/plain; charset=utf-8"
SAVINGS = "EUR;12000,150,210,320;240000,3000,4200,6400"
PING_SECONDS = 10
TICK_SECONDS = 0.2  # the board publishes what a request changed on its next tick, after its answer
DOWN_SECONDS = 2  # how long a restart takes
HOLDS = {"Create schedule": "schedule", "Start charging now": "now", "Stop charging": "none"}  # the schedule buttons


def later(seconds, action):
    timer = threading.Timer(seconds, action)
    timer.daemon = True
    timer.start()


class Board:
    """What the page sees of the board. `changed` wakes the streams, and `version` counts the changes."""

    def __init__(self, tool, settings_path):
        self.tool = tool
        self.settings_path = settings_path
        self.changed = threading.Condition()
        self.version = 0
        self.generation = 0  # a restart ends the streams of the generation before
        self.options = self.run("options").stdout
        self.reset(settings=Path(settings_path).read_text() if settings_path and Path(settings_path).exists() else "")
        self.paired = self.has_car()  # a board keeps its pairing through a restart

    def reset(self, **state):
        """A board as it starts, with `state` over its defaults: settings, paired, plugged, hold, update."""
        with self.changed:
            self.settings = ""
            self.paired = False
            self.plugged = False
            self.hold = "schedule"  # what the schedule buttons chose
            self.limit = 80
            self.ready_by = "07:00:00"
            self.ready_by_once = "2000-01-01 00:00:00"
            self.release = RELEASE
            self.update = None  # a release ahead, or none
            self.checked = False  # Check for update pressed since the start
            self.installing = False
            self.started = time.time()
            self.down_until = 0.0
            self.posts = []
            for key, value in state.items():
                if key not in vars(self):
                    raise ValueError(f"the board has no {key}")
                setattr(self, key, value)
            self.error = self.check(self.settings)[0] if self.settings else ""
            self.wake()

    def wake(self):
        with self.changed:
            self.version += 1
            self.changed.notify_all()

    def run(self, *args):
        done = subprocess.run([self.tool, *args], capture_output=True, text=True, cwd=ROOT, check=False)
        if done.returncode not in (0, 1):
            raise RuntimeError(f"{self.tool} failed: {done.stderr}")
        return done

    def check(self, text, before=None):
        """What's wrong with a settings file, as the board says, or "", and with the file the board had, whether
        saving it restarts the board and whether it deletes the schedule."""
        with tempfile.TemporaryDirectory() as folder:
            files = [Path(folder, "settings.yaml")]
            files[0].write_text(text)
            if before is not None:
                files.append(Path(folder, "before.yaml"))
                files[1].write_text(before)
            done = self.run("check", *map(str, files))
        lines = done.stdout.splitlines()
        return done.stdout.strip() if done.returncode else "", "restarts" in lines, "deletes schedule" in lines

    def save(self, text):
        """POST /settings: the board's answer, and the file saved."""
        error, restarts, deletes = self.check(text, self.settings)
        if error:
            return 400, error
        with self.changed:
            self.settings = text
            self.error = ""
            if deletes and self.hold == "schedule":
                self.hold = "none"
            if self.settings_path:
                Path(self.settings_path).write_text(text)
        later(TICK_SECONDS, self.wake)
        if restarts:
            self.restart()
        return 200, "Saved: the board restarts" if restarts else "Saved"

    def restart(self, then=None):
        """A restart, after the answer went out: the connections drop, and the board is back in a moment with its
        uptime from 0, with what `then` changes."""

        def go():
            with self.changed:
                if then:
                    then()
                self.down_until = time.time() + DOWN_SECONDS
                self.started = self.down_until
                self.checked = self.installing = False
                self.generation += 1
                self.wake()

        later(0.5, go)

    def forget(self):
        """Restart setup: the settings, the key and Ready by go, and the savings stay."""
        self.settings = self.error = ""
        self.paired = False
        self.hold = "schedule"
        self.ready_by, self.ready_by_once = "07:00:00", "2000-01-01 00:00:00"
        if self.settings_path:
            Path(self.settings_path).write_text("")

    def press(self, name):
        """A button's press; False for one the board doesn't have."""
        if name == "Pair BLE Key":
            later(1, self.pair)  # the car answers the key in a moment
        elif name == "Restart":
            self.restart()
        elif name == "Restart setup":
            self.restart(self.forget)
        elif name == "Check for update":
            self.checked = True
            later(TICK_SECONDS, self.wake)
        elif name in HOLDS:
            self.hold = HOLDS[name]
            later(TICK_SECONDS, self.wake)
        elif name not in ("Reset savings", "Wake up", "Force data update", "Regenerate key"):
            return False
        return True

    def pair(self):
        with self.changed:
            self.paired = self.has_car()
            self.wake()

    def set(self, domain, name, value):
        """A number, time or datetime set; False for one the board doesn't have."""
        with self.changed:
            if (domain, name) == ("number", "Charging Limit"):
                self.limit = int(float(value))
            elif (domain, name) == ("time", "Ready by"):
                self.ready_by = value
            elif (domain, name) == ("datetime", "Ready by once"):
                self.ready_by_once = value
            else:
                return False
        later(TICK_SECONDS, self.wake)
        return True

    def install(self):
        """Update's install: the release ahead downloads, then the board restarts with it."""
        with self.changed:
            self.installing = self.update is not None
            self.wake()

        def then():
            self.release, self.update = self.update, None

        if self.installing:
            self.restart(then)

    def has_car(self):
        return bool(re.search(r"^tesla_vin:", self.settings, re.M))

    def down(self):
        return time.time() < self.down_until

    def uptime(self):
        return max(0.0, time.time() - self.started)

    def window(self):
        """The schedule's one window: the next full hour."""
        start = (int(time.time()) // 3600 + 1) * 3600
        return start, start + 3600

    def status(self):
        """The status and the mode, as the controller reports them for the settings and the car."""
        if not self.settings:
            return "No settings yet", "wait"
        if self.error:
            return "Settings: " + self.error, "wait"
        if not self.has_car():
            return "No car yet", "wait"
        if not self.paired:
            return "Not paired", "wait"
        if not self.plugged:
            return "Unplugged", "unplugged"
        if self.hold == "now":
            return "Charging now", "now"
        if self.hold == "none":
            return "No schedule", "none"
        return "Charges at " + time.strftime("%H:%M", time.localtime(self.window()[0])), "schedule"

    def states(self):
        """Every entity's state, as /events reports them."""
        status, mode = self.status()
        currency = (re.search(r"^currency: (\w+)", self.settings, re.M) or [None, "EUR"])[1]
        charging = self.plugged and self.hold == "now"
        start, end = self.window()
        if self.installing:
            firmware = ("INSTALLING", self.update)
        elif not self.checked:
            firmware = ("UNKNOWN", "")
        else:
            firmware = ("UPDATE AVAILABLE", self.update) if self.update else ("NO UPDATE", self.release)
        states = [
            text("Charging status", status),
            text("Charging mode", mode),
            text("Charge windows", f"{currency};{start},{end},0.076" if mode == "schedule" else ""),
            text("Prices until", str((int(time.time()) // 86400 + 2) * 86400) if self.has_car() else "0"),
            text("Savings", SAVINGS if self.paired else ""),
            text("Release", self.release),
            sensor("WiFi Signal", -60, "dBm"),
            sensor("Uptime", int(self.uptime()), "s"),
            {"id": "number/Charging Limit", "value": str(self.limit), "state": str(self.limit)},
            {"id": "time/Ready by", "value": self.ready_by, "state": self.ready_by},
            {"id": "datetime/Ready by once", "value": self.ready_by_once, "state": self.ready_by_once},
            {"id": "update/Firmware", "value": firmware[1], "state": firmware[0]},
        ]
        if self.has_car():  # the car reports its signal and sleep to any key, and the rest to one it knows
            states += [sensor("BLE Signal", -70, "dBm"), binary("Asleep", False)]
        if self.paired:
            states += [
                binary("Charger", self.plugged),
                text("Charging", "Charging" if charging else "Stopped" if self.plugged else "Disconnected"),
                sensor("Battery", 55, "%"),
                sensor("Charger Power", 11 if charging else 0, "kW"),
            ]
        return states

    def state(self):
        """What GET /fake answers."""
        keys = ("settings", "error", "paired", "plugged", "hold", "limit", "ready_by", "release", "update", "posts")
        return {key: getattr(self, key) for key in keys}


def text(name, value):
    return {"id": f"text_sensor/{name}", "value": value, "state": value}


def sensor(name, value, unit):
    return {"id": f"sensor/{name}", "value": value, "state": f"{value} {unit}"}


def binary(name, on):
    return {"id": f"binary_sensor/{name}", "value": on, "state": "ON" if on else "OFF"}


class Handler(http.server.BaseHTTPRequestHandler):
    """The board's routes, and /fake. do_GET and do_POST are http.server's names."""

    board: Board

    def reply(self, code, kind, body):
        body = body.encode() if isinstance(body, str) else body
        self.send_response(code)
        self.send_header("Content-Type", kind)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def own(self):
        """Only the board's own page reads or changes the settings, as on the board: another Origin gets 400."""
        origin = self.headers.get("Origin")
        if origin is None or origin.endswith("://" + self.headers.get("Host", "")):
            return True
        self.reply(400, TEXT, "Only the board's own page can read or change the settings")
        return False

    def do_GET(self):  # noqa: N802
        if self.board.down():  # a restarting board answers nothing: the connection drops
            return
        path = unquote(urlparse(self.path).path)
        if path.startswith("/settings") and not self.own():
            return
        if path == "/":
            self.reply(200, "text/html", INDEX)
        elif path in ("/0.js", "/0.css"):
            file = ROOT / "web" / ("web_ui.js" if path == "/0.js" else "web_ui.css")
            self.reply(200, "text/javascript" if path == "/0.js" else "text/css", file.read_bytes())
        elif path == "/events":
            self.stream()
        elif path == "/settings":
            self.reply(200, TEXT, self.board.settings)
        elif path == "/settings/options":
            self.reply(200, "application/json", self.board.options)
        elif path == "/fake":
            self.reply(200, "application/json", json.dumps(self.board.state()))
        else:
            self.reply(404, TEXT, "")

    def do_POST(self):  # noqa: N802
        if self.board.down():
            return
        url = urlparse(self.path)
        path = unquote(url.path)
        body = self.rfile.read(int(self.headers.get("Content-Length", 0))).decode()
        if path == "/fake":
            self.board.reset(**json.loads(body or "{}"))
            self.reply(200, TEXT, "")
            return
        self.board.posts.append({"url": path + (f"?{unquote(url.query)}" if url.query else ""), "body": body})
        parts = path.strip("/").split("/")
        if path == "/settings":
            if self.own():
                code, answer = self.board.save(body)
                self.reply(code, TEXT, answer)
        elif len(parts) == 3 and parts[0] == "button" and parts[2] == "press":
            self.reply(200 if self.board.press(parts[1]) else 404, TEXT, "")
        elif len(parts) == 3 and parts[0] in ("number", "time", "datetime") and parts[2] == "set":
            value = parse_qs(url.query).get("value", [""])[0]
            self.reply(200 if self.board.set(parts[0], parts[1], value) else 404, TEXT, "")
        elif path == "/update/Firmware/install":
            self.board.install()
            self.reply(200, TEXT, "")
        else:
            self.reply(404, TEXT, "")

    def event(self, name, data):
        self.wfile.write(f"event: {name}\ndata: {json.dumps(data)}\n\n".encode())
        self.wfile.flush()

    def stream(self):
        """/events: a ping on connecting and every 10 s, with the uptime, and each state as it changes, until the board
        restarts or the page goes."""
        board = self.board
        generation = board.generation
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Cache-Control", "no-cache")
        self.end_headers()
        sent = {}
        next_ping = 0.0
        try:
            while generation == board.generation and not board.down():
                with board.changed:
                    version, states, uptime = board.version, board.states(), board.uptime()
                if time.monotonic() >= next_ping:
                    self.event("ping", {"title": "tesla", "uptime": int(uptime)})
                    next_ping = time.monotonic() + PING_SECONDS
                for state in states:
                    if sent.get(state["id"]) != state:
                        self.event("state", state)
                        sent[state["id"]] = state
                with board.changed:
                    if board.version == version:  # nothing new since: until something is, or the next ping
                        board.changed.wait(max(0.0, next_ping - time.monotonic()))
        except (BrokenPipeError, ConnectionResetError):
            pass


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("port", nargs="?", type=int, default=8765)
    parser.add_argument("settings", nargs="?", help="the board's settings file: read at the start, written at a save")
    parser.add_argument("--tool", default=Path(__file__).with_name("settings_tool"), help="settings_tool, built")
    args = parser.parse_args()
    if not Path(args.tool).exists():
        sys.exit(f"{args.tool} is missing: build settings_tool.cpp like the unit tests (CONTRIBUTING.md's Checks)")
    Handler.board = Board(str(args.tool), args.settings)
    server = http.server.ThreadingHTTPServer(("127.0.0.1", args.port), Handler)
    print(f"The fake board is at http://127.0.0.1:{args.port}", flush=True)
    server.serve_forever()


if __name__ == "__main__":
    main()
