// Phone-first page for the Tesla BLE board, served by ESPHome's web server in place of its default page (js_url: "").
// It reads live state from /events and sends actions to the REST API (POST /<domain>/<entity name>/<action>), and
// shows only what the charging-only key can use. Add ?full to the address for ESPHome's own page (needs internet).
//
//   Page           entity names (E), head tags and the page markup
//   State          what the board reported, the connection, draft (picked, not sent), pending (sent, not reported)
//   Render         one render per frame, from state to the page
//   Charge limit   the limit dropdown and sending it
//   Ready by       the deadline dropdown and sending it: the daily time and the one-off
//   Schedule       the schedule card: the mode, the charge windows and the schedule buttons
//   Savings        the savings card
//   Settings       the setup's steps and the settings form, and the settings file they make, sent to the board and back
//   Board link     /events, POST and toasts
//   Time and text  clock times, the board's dates, dBm and uptime as text
//   Start          wiring, then this page or ESPHome's (?full)

// --- Page ------------------------------------------------------------------

const $ = (id) => document.getElementById(id);

const E = {
  battery: "sensor/Battery",
  ble: "sensor/BLE Signal",
  chargeNow: "button/Start charging now",
  charging: "text_sensor/Charging",
  createSchedule: "button/Create schedule",
  factoryReset: "button/Factory reset",
  limit: "number/Charging Limit",
  mode: "text_sensor/Charging mode",
  pair: "button/Pair BLE Key",
  power: "sensor/Charger Power",
  pricesUntil: "text_sensor/Prices until",
  readyBy: "time/Ready by",
  readyByOnce: "datetime/Ready by once",
  resetSavings: "button/Reset savings",
  restart: "button/Restart",
  savings: "text_sensor/Savings",
  status: "text_sensor/Charging status",
  stopCharging: "button/Stop charging",
  uptime: "sensor/Uptime",
  version: "text_sensor/Release",
  wifi: "sensor/WiFi Signal",
  windows: "text_sensor/Charge windows",
};

document.head.insertAdjacentHTML(
  "beforeend",
  '<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">' +
    '<meta name="apple-mobile-web-app-capable" content="yes">' +
    '<meta name="apple-mobile-web-app-title" content="Tesla">' +
    '<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">' +
    '<meta name="theme-color" content="#009ac7">',
);

// The logo: a calendar with a plug, white with its details in the header's blue, as ESPHome's logo is.
const LOGO = `<svg viewBox="0 0 40 40" aria-hidden="true">
  <rect x="1" y="5" width="32" height="30" rx="4" fill="#fff"/>
  <rect x="8" y="1" width="5" height="9" rx="2.5" fill="#fff" stroke="currentColor" stroke-width="1.5"/>
  <rect x="21" y="1" width="5" height="9" rx="2.5" fill="#fff" stroke="currentColor" stroke-width="1.5"/>
  <g fill="currentColor">
    <rect x="1" y="12" width="32" height="1.5"/>
    <rect x="14.5" y="17" width="5" height="5" rx="1"/>
    <rect x="5.5" y="25.5" width="5" height="5" rx="1"/>
    <rect x="14.5" y="25.5" width="5" height="5" rx="1"/>
    <circle cx="29" cy="29" r="11"/>
  </g>
  <path d="M5.5 20.2l2.3 2.3 4.2-4.6" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
  <circle cx="29" cy="29" r="9.25" fill="#fff"/>
  <g fill="currentColor">
    <rect x="25.9" y="22.6" width="1.7" height="3.6" rx="0.7"/>
    <rect x="30.4" y="22.6" width="1.7" height="3.6" rx="0.7"/>
    <path d="M24.6 26h8.8v2.2a4.4 4.4 0 0 1-8.8 0z"/>
    <rect x="28.15" y="31.5" width="1.7" height="4.3" rx="0.7"/>
  </g>
</svg>`;

const PAGE = `
<header class="bar">
  ${LOGO}
  <div><h1>Tesla BLE Scheduler</h1><p>Charges when it's cheapest</p></div>
  <span id="link" class="pill">Connecting …</span>
</header>
<main>
  <section class="card">
    <div class="row"><span>Current charge</span><strong id="soc">-</strong></div>
    <div class="row"><span>Status</span><strong id="status">Connecting …</strong></div>
  </section>

  <section id="settings-card" class="card" hidden>
    <div class="title">Settings</div>
    <label id="vin-row" class="row"><span>VIN</span><input id="set-vin" class="wide" placeholder="17 letters and digits" required pattern="[A-HJ-NPR-Za-hj-npr-z0-9]{17}" title="17 letters and digits, none of them I, O or Q, on the car's screen under Controls, Software" autocapitalize="characters" autocomplete="off" spellcheck="false"></label>
    <label id="area-row" class="row"><span>Market area</span><span class="dropdown"><select id="set-area" required></select></span></label>
    <label id="plan-row" class="row"><span>Grid plan</span><span class="dropdown"><select id="set-plan"></select></span></label>
    <label class="row"><span>Battery (kWh)</span><input id="set-battery" type="number" required min="1" step="any" inputmode="decimal" placeholder="75"></label>
    <label class="row"><span>Charging power (kW)</span><input id="set-power" type="number" required min="1" step="any" inputmode="decimal" placeholder="11"></label>
    <label class="row"><span>VAT (%)</span><input id="set-vat" type="number" required min="0" max="99" step="any" inputmode="decimal" placeholder="21"></label>
    <label class="row"><span>Margin per kWh</span><input id="set-margin" type="number" min="0" step="any" inputmode="decimal" placeholder="0"></label>
    <label class="row"><span>Time zone</span><span class="dropdown"><select id="set-zone" required></select></span></label>
    <label class="row"><span>ntfy topic</span><input id="set-topic" class="wide" pattern="[A-Za-z0-9_\\-]{0,64}" title="The topic's name: up to 64 letters, digits, - and _" autocomplete="off" spellcheck="false" placeholder="none"></label>
    <p id="settings-more" class="note" hidden>Your settings have more than this form shows, which saving it drops: to keep it, change the file instead.</p>
    <p id="settings-error" class="note error" hidden></p>
    <button id="save-settings" class="primary">Save</button>
    <button id="cancel-settings">Cancel</button>
    <a id="download-settings" class="button" href="/settings" download="settings.yaml">Download settings</a>
    <button class="upload">Upload settings</button>
    <input id="settings-file" type="file" accept=".yaml,.yml,.txt" hidden>
  </section>

  <section class="card step" hidden>
    <div class="title">VIN<span class="summary"></span></div>
    <div class="body">
      <p class="note">The board needs your car's VIN to find it over Bluetooth and talk to it. It's on the car's screen under Controls → Software, and at the bottom of the Tesla app's home screen. It stays on the board.</p>
      <p id="vin-error" class="note error" hidden></p>
    </div>
  </section>
  <section class="card step" hidden>
    <div class="title">Key<span class="summary"></span></div>
    <div class="body">
      <p class="note">The car only takes orders from keys it knows, so the board makes a key of its own for the car to add, like a phone key. It can only charge: it can't unlock or drive the car, and you can remove it in the car under Controls → Locks.</p>
      <ol class="note">
        <li>Put the board by the car.</li>
        <li>Sit in the car with your Tesla key card.</li>
        <li>Press Create key button below.</li>
        <li>Tap the card on the console.</li>
        <li>Confirm on the car's screen.</li>
      </ol>
      <p class="note">The setup moves on once the car answers.</p>
      <button id="pair-now" class="primary">Create key</button>
    </div>
  </section>
  <section class="card step" hidden>
    <div class="title">Market area<span class="summary"></span></div>
    <div class="body"></div>
  </section>
  <section class="card step" hidden>
    <div class="title">Grid plan<span class="summary"></span></div>
    <div class="body"></div>
  </section>
  <details id="advanced" class="card" hidden>
    <summary>Advanced</summary>
    <button class="upload">Upload settings</button>
    <button class="restart danger">Restart board</button>
    <button class="factory-reset danger">Factory reset</button>
  </details>

  <section id="target-card" class="card">
    <label class="row"><span>Charge limit</span><span class="dropdown"><select id="limit-select" aria-label="Charge limit"></select></span></label>
    <label id="ready-row" class="row"><span>Ready by</span><span class="dropdown"><select id="ready-select" aria-label="Ready by"></select></span></label>
  </section>

  <section id="schedule-card" class="card" hidden>
    <div class="title">Schedule</div>
    <div id="schedule-start">
      <button id="create-schedule" class="primary">Create schedule</button>
      <div class="or">or</div>
      <button id="charge-now">Start charging now</button>
    </div>
    <div id="schedule-rows">
      <div id="windows"></div>
      <button id="delete-schedule" class="red">Delete schedule</button>
    </div>
    <button id="stop-charging" class="red">Stop charging</button>
  </section>

  <section id="savings-card" class="card" hidden>
    <div class="title">Savings</div>
    <div class="row"><span>Last 30 days</span><strong id="saved-month">-</strong></div>
    <div class="row"><span>Last 12 months</span><strong id="saved-year">-</strong></div>
    <p id="against-average" class="note"></p>
    <p id="against-at-once" class="note"></p>
    <a class="button" href="https://buymeacoffee.com/zygimantas_berziunas" target="_blank" rel="noopener">Buy me a coffee</a>
    <button id="reset-savings" class="danger">Reset savings</button>
  </section>

  <details class="card">
    <summary>Board</summary>
    <div class="row"><span>Bluetooth</span><strong id="ble">-</strong></div>
    <div class="row"><span>Wi-Fi</span><strong id="wifi">-</strong></div>
    <div class="row"><span>Uptime</span><strong id="uptime">-</strong></div>
    <div class="row"><span>Version</span><strong id="version">-</strong></div>
    <button id="change-settings">Change settings</button>
    <button id="pair">Create key</button>
    <button class="restart danger">Restart board</button>
    <button class="factory-reset danger">Factory reset</button>
  </details>

  <div id="toast" class="toast" role="status"></div>
</main>`;

// --- State -----------------------------------------------------------------

// The board's settings file ("" without one, null until read), what the form offers, from /settings/options, and
// whether Change settings opened the form.
const settings = { text: null, options: null, open: false };
// The setup's open step, 1 to 4, or 0; the furthest it got, as the steps up to it open with a click; after its last
// step saved the settings, that the board hasn't got them yet; and that a step was opened by hand, which keeps Key
// from moving on by itself.
const setup = { step: 0, reached: 0, saving: false, stay: false };

const states = {}; // entity id -> latest state event
// null until the first connection, then whether live updates from the board are coming in. The Status row says when
// they aren't.
let live = null;
// Charge limit and Ready by picked here but not sent yet: the schedule buttons send them.
const draft = { limit: null, deadline: null };
// The mode a button should bring, and the limit and Ready by just sent, as { value, until }, shown until the board
// reports them or `until` passes.
const pending = { mode: null, limit: null, deadline: null };

// Numbers arrive as JSON numbers from sensors but as strings ("80") from number entities.
const value = (id) => {
  const v = states[id]?.value;
  const n = typeof v === "string" && v.trim() !== "" ? Number(v) : v;
  return typeof n === "number" && Number.isFinite(n) ? n : null;
};
const text = (id) => {
  const s = states[id]?.state;
  return s == null || s === "NA" ? "" : String(s);
};
// End of the published prices, in ms (the board sends UTC seconds).
const pricesUntil = () => Number(text(E.pricesUntil)) * 1000;

// A value just sent from here, kept until the board reports it or `until` passes; else null.
const stillPending = (sent, board) => (sent && Date.now() < sent.until && board !== sent.value ? sent : null);

// --- Render ----------------------------------------------------------------

let renderQueued = false;
function requestRender() {
  if (renderQueued) return;
  renderQueued = true;
  requestAnimationFrame(() => {
    renderQueued = false;
    render();
  });
}

function render() {
  const soc = value(E.battery);
  const charging = ["Charging", "Starting"].includes(text(E.charging));
  $("soc").textContent = soc == null ? "-" : `${Math.round(soc)}%`;
  // The board's status, with the charging power while the car charges (non-breaking spaces keep it one piece).
  const kw = value(E.power);
  const power = charging && kw != null ? ` ·\u00a0${kw.toFixed(1)}\u00a0kW` : "";
  $("status").textContent = live === false ? "No connection" : (text(E.status) || "Connecting …") + power;
  $("link").textContent = live === null ? "Connecting …" : live ? "Connected" : "No connection";
  renderSchedule(); // first: it drops the draft when the dropdowns can't change
  renderLimit();
  renderReady();
  renderSavings();

  $("ble").textContent = dbm(value(E.ble));
  $("wifi").textContent = dbm(value(E.wifi));
  $("uptime").textContent = duration(value(E.uptime));
  $("version").textContent = text(E.version) || "-";
  renderSetup();
  // Settings the board can't use come first; others open from Board.
  const needed = text(E.status).startsWith("Settings: ");
  $("settings-card").hidden = setup.step > 0 || (!needed && !settings.open);
  $("cancel-settings").hidden = needed;
}

// Replaces a dropdown's options only when their values or greying changed, as render() runs on every board
// update; then selects `selected`.
function setOptions(select, options, selected) {
  const old = select.options;
  if (
    old.length !== options.length ||
    options.some((o, i) => old[i].value !== o.value || old[i].disabled !== o.disabled)
  )
    select.replaceChildren(...options);
  select.value = selected;
}

// --- Charge limit ----------------------------------------------------------

const LIMIT_STEPS = [100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50];

// The charge limit to show: one just sent from here until the car confirms it, else the car's own
// (80% until it reports one).
function shownLimit() {
  pending.limit = stillPending(pending.limit, value(E.limit));
  return Math.round(pending.limit?.value ?? value(E.limit) ?? 80);
}

// The steps, plus the car's own limit when it's in between (say 83% from the Tesla app).
function renderLimit() {
  const select = $("limit-select");
  if (document.activeElement === select) return;
  const shown = draft.limit ?? shownLimit();
  const limits = LIMIT_STEPS.includes(shown) ? LIMIT_STEPS : [...LIMIT_STEPS, shown].sort((a, b) => b - a);
  const options = limits.map((v) => new Option(`${v}%`, v));
  setOptions(select, options, shown);
}

// Sends a changed charge limit to the car; false only if sending failed.
async function sendLimit() {
  const limit = draft.limit;
  if (limit == null || limit === value(E.limit)) return true;
  pending.limit = { value: limit, until: Date.now() + 20000 };
  if (await post(E.limit, "set", limit)) return true;
  pending.limit = null;
  return false;
}

// --- Ready by --------------------------------------------------------------

// The half-hours from now to the end of the published prices, as the board doesn't schedule on guesses. The time picked
// becomes the daily Ready by, which the board remembers; a later day than that time's next occurrence is a one-off.

const HALF_HOUR_MS = 30 * 60 * 1000;
const CLEAR_ONCE = "2000-01-01 00:00:00"; // Ready by once's "unset" value

// The daily time, the one-off while it's ahead, and the deadline in force.
function readyBy() {
  const daily = text(E.readyBy).slice(0, 5);
  const once = parseBoardTime(text(E.readyByOnce));
  if (once != null && once > Date.now()) return { daily, once, deadline: once };
  return { daily, once: null, deadline: /^\d\d:\d\d$/.test(daily) ? nextAt(daily) : null };
}

// Fills the dropdown with each half-hour that can be picked, "23:30" or "07:00 +1", and the deadline
// shown, greyed out when it can't be (a daily time whose prices aren't out yet). The hour repeated when the clocks
// go back is offered once, as the board takes a time in it as the first.
function fillReady(deadline) {
  const now = Date.now();
  const until = pricesUntil();
  const times = [];
  for (let t = (Math.floor(now / HALF_HOUR_MS) + 1) * HALF_HOUR_MS; t <= until; t += HALF_HOUR_MS) times.push(t);
  if (!times.includes(deadline)) times.push(deadline);
  times.sort((a, b) => a - b);
  const options = times
    .map((t) => Object.assign(new Option(`${hhmm(t)}${plus(t)}`, t), { disabled: t <= now || t > until }))
    .filter((o, i, all) => all.findIndex((p) => p.text === o.text) === i);
  setOptions($("ready-select"), options, deadline);
}

function renderReady() {
  $("ready-row").hidden = pricesUntil() <= Date.now(); // nothing to pick until the board has prices
  if (document.activeElement === $("ready-select")) return;
  const board = readyBy().deadline;
  pending.deadline = stillPending(pending.deadline, board);
  const deadline = draft.deadline ?? pending.deadline?.value ?? board;
  if (deadline != null) fillReady(deadline);
}

// Sends the daily time ("HH:MM") and the one-off (a timestamp, or null to clear it) where they differ from `current`
// (readyBy()); false if sending failed.
async function sendReadyBy(time, once, current) {
  if (time !== current.daily && !(await post(E.readyBy, "set", `${time}:00`))) return false;
  if (once === current.once) return true;
  return post(E.readyByOnce, "set", once == null ? CLEAR_ONCE : boardTime(once));
}

// --- Schedule --------------------------------------------------------------

// The board's mode ("schedule", "now", "none", "unplugged" or "wait"), or the one a button just asked for until the
// board has it.
function shownMode() {
  const board = text(E.mode) || "wait";
  pending.mode = stillPending(pending.mode, board);
  return pending.mode?.value ?? board;
}
function expectMode(mode) {
  pending.mode = { value: mode, until: Date.now() + 10000 };
  requestRender();
}

// Both buttons, the schedule with Delete, or Stop while charging regardless of price. Charge limit and Ready by are for
// a new schedule or charge, so only then can they change.
function renderSchedule() {
  const mode = shownMode();
  $("target-card").hidden = mode === "wait"; // nothing to set or show until the board can schedule
  $("schedule-card").hidden = mode === "unplugged" || mode === "wait";
  $("schedule-start").hidden = mode !== "none";
  $("schedule-rows").hidden = mode !== "schedule";
  $("stop-charging").hidden = mode !== "now";
  const editable = mode === "none";
  if (!editable) draft.limit = draft.deadline = null;
  for (const id of ["limit-select", "ready-select"]) $(id).disabled = !editable;
  // No schedule without prices: the board downloads them after it starts, which takes about a minute.
  const priced = pricesUntil() > Date.now();
  $("create-schedule").disabled = !priced || busy;
  $("create-schedule").textContent = priced ? "Create schedule" : "Getting prices …";
  renderWindows();
}

// "<currency>;<start>,<end>,<price>[,spare];..." from format_windows() in schedule.h: the windows in UTC
// seconds with their price per kWh, shown as "00:00 - 01:00 +1" and "0.076 EUR/kWh", spare ones faded.
function renderWindows() {
  const [currency, ...entries] = text(E.windows).split(";");
  const rows = entries.filter(Boolean).map((entry) => {
    const [start, end, amount, spare] = entry.split(",");
    const row = document.createElement("div");
    row.className = spare ? "row spare" : "row";
    const when = document.createElement("span");
    when.textContent = `${hhmm(start * 1000)} - ${hhmm(end * 1000)}${plus(start * 1000)}`;
    const price = document.createElement("strong");
    price.textContent = `${amount} ${currency}/kWh`;
    row.append(when, price);
    return row;
  });
  $("windows").replaceChildren(...rows);
}

// Sends what changed in the draft, then asks the board to schedule, which cancels Start charging now.
async function createSchedule() {
  const current = readyBy();
  let deadline = draft.deadline ?? current.deadline;
  if (deadline == null) return;
  const time = hhmm(deadline);
  if (deadline <= Date.now()) deadline = nextAt(time);
  const once = deadline === nextAt(time) ? null : deadline;
  pending.deadline = { value: deadline, until: Date.now() + 20000 };
  const ok = (await sendLimit()) && (await sendReadyBy(time, once, current)) && (await post(E.createSchedule, "press"));
  if (ok) {
    expectMode("schedule");
    toast(`Schedule: ${shownLimit()}% by ${hhmm(deadline)}${plus(deadline)}`);
  } else {
    pending.deadline = null;
  }
}

// Start charging now: charges at once to the limit shown (sending it if changed); Ready by doesn't apply.
async function chargeNow() {
  if ((await sendLimit()) && (await post(E.chargeNow, "press"))) {
    expectMode("now");
    toast("Charging now until you unplug");
  }
  requestRender();
}

// Stop charging and Delete schedule: the board stops charging and keeps the car waiting until a
// button here, or the car is unplugged.
async function stopCharging(message) {
  if (await post(E.stopCharging, "press")) {
    expectMode("none");
    toast(message);
  }
}

// --- Savings ---------------------------------------------------------------

// "<currency>;<last 30 days>;<last 365 days>" from format_savings() in savings.h, each period as
// "<Wh>,<paid>,<at the day's average>,<at once>" with the money in hundredths. Saved is the difference.
function renderSavings() {
  const [currency, ...periods] = text(E.savings).split(";");
  $("savings-card").hidden = !currency;
  if (!currency) return;
  const [month, year] = periods.map((period) => period.split(",").map(Number));
  const money = (hundredths) => `${(hundredths / 100).toFixed(2)}\u00a0${currency}`; // one piece when it wraps
  $("saved-month").textContent = money(month[2] - month[1]);
  $("saved-year").textContent = money(year[2] - year[1]);
  $("against-average").textContent =
    `Compared with the day's average price: ${Math.round(month[0] / 1000)} kWh for ${money(month[1])} in the last 30 days`;
  $("against-at-once").textContent =
    `Compared with charging at once on plug-in: ${money(month[3] - month[1])} saved in the last 30 days`;
}

// --- Settings --------------------------------------------------------------

// What a new board starts with, as of October 2026: each market country's VAT on household electricity in %, which
// northern Norway (NO4) doesn't charge, and its time zones, to guess the country from the phone's.
const COUNTRIES = {
  AT: [20, "Europe/Vienna"],
  BE: [6, "Europe/Brussels"],
  BG: [20, "Europe/Sofia"],
  CH: [8.1, "Europe/Zurich"],
  CZ: [21, "Europe/Prague"],
  DE: [19, "Europe/Berlin", "Europe/Busingen"],
  DK: [25, "Europe/Copenhagen"],
  EE: [24, "Europe/Tallinn"],
  ES: [21, "Europe/Madrid", "Africa/Ceuta", "Atlantic/Canary"],
  FI: [25.5, "Europe/Helsinki", "Europe/Mariehamn"],
  FR: [20, "Europe/Paris"],
  HR: [13, "Europe/Zagreb"],
  HU: [27, "Europe/Budapest"],
  IT: [10, "Europe/Rome"],
  LT: [21, "Europe/Vilnius"],
  LU: [8, "Europe/Luxembourg"],
  LV: [21, "Europe/Riga"],
  NL: [21, "Europe/Amsterdam"],
  NO: [25, "Europe/Oslo"],
  PL: [23, "Europe/Warsaw"],
  PT: [23, "Europe/Lisbon", "Atlantic/Madeira"],
  RO: [21, "Europe/Bucharest"],
  SE: [25, "Europe/Stockholm"],
  SI: [22, "Europe/Ljubljana"],
};
const vatOf = (area) => (area === "NO4" ? 0 : (COUNTRIES[area.slice(0, 2)]?.[0] ?? ""));

// The places in the settings file that the form shows, like "market: area"; ntfy_server only as the default.
const FORM_PLACES = [
  "market",
  "market: area",
  "market: margin",
  "market: vat",
  "ntfy_topic",
  "tariff",
  "tariff: plan",
  "tesla_battery_kwh",
  "tesla_charging_kw",
  "tesla_vin",
  "timezone",
];

// The settings file's values by place, as the board reads them, and whether it has more than the form shows.
function readSettings(file) {
  const values = {};
  let section = "";
  let more = false;
  for (const line of file.split("\n")) {
    const m = /^( *)([^\s#:][^:]*):(?: +(.*))?$/.exec(line.replace(/ #.*/, "").trimEnd());
    if (!m) continue;
    const [, indent, key, raw = ""] = m;
    const value = raw.replace(/^(["'])(.*)\1$/, "$2");
    const place = indent ? `${section}: ${key}` : key;
    if (!indent) section = raw ? "" : key;
    if (place === "ntfy_server" && value === "https://ntfy.sh") continue;
    if (indent.length > 2 || !FORM_PLACES.includes(place)) more = true;
    else values[place] = value;
  }
  return { values, more };
}

// Settings without prices, a new board's or what the setup's first step saved, still need the setup's last steps.
// What's wrong with a VIN, or "": a Tesla's starts with one of its makers' codes, and its 9th character is a check
// digit, computed from the others as in North America, which Tesla does for its Shanghai and Berlin cars too.
const TESLA_MAKERS = ["5YJ", "7SA", "7G2", "LRW", "XP7"];
function vinProblem(vin) {
  if (vin.length !== 17) return `A VIN has 17 letters and digits, and this one has ${vin.length}.`;
  if (/[IOQ]/.test(vin)) return "A VIN has no I, O or Q: they're 1 or 0.";
  if (!/^[A-Z0-9]+$/.test(vin)) return "A VIN has only letters and digits.";
  if (!TESLA_MAKERS.includes(vin.slice(0, 3)))
    return "A Tesla's VIN starts with 5YJ, 7SA, 7G2, LRW or XP7: check the first three.";
  const value = (c) => (c <= "9" ? Number(c) : Number("12345678123457923456789"["ABCDEFGHJKLMNPRSTUVWXYZ".indexOf(c)]));
  const weights = [8, 7, 6, 5, 4, 3, 2, 10, 0, 9, 8, 7, 6, 5, 4, 3, 2];
  const sum = [...vin].reduce((total, c, i) => total + value(c) * weights[i], 0);
  return "0123456789X"[sum % 11] === vin[8] ? "" : "This VIN doesn't add up: a letter or digit is off. Check it again.";
}

const unfinished = () => settings.text === "" || (settings.text !== null && !/^(market|tariff):/m.test(settings.text));
// The setup's last step: Key once the settings have prices, else Market area, or Grid plan where the country has plans.
const lastStep = () => (!unfinished() ? 2 : $("plan-row").hidden ? 3 : 4);

// The setup, for a new board, settings without prices and a key the car doesn't know yet: the open step shows its
// fields, done ones fold to their titles and what they hold, later ones show only their titles. Steps 1, 3 and 4
// borrow the form's VIN, market area and grid plan rows, below their text and above their buttons, which go back to
// the form after.
function renderSetup() {
  const unpaired = text(E.status) === "Not paired";
  if (settings.text === null || !(unfinished() || unpaired)) setup.step = setup.reached = 0;
  else if (!setup.step && !setup.saving) setup.step = setup.reached = settings.text === "" ? 1 : unpaired ? 2 : 3;
  if (unpaired) {
    // a key the car doesn't know closes the steps after Key
    setup.reached = Math.min(setup.reached, 2);
    setup.step = Math.min(setup.step, setup.reached);
  }
  // Key moves on by itself once the car answers, unless it was opened by hand
  const answered = settings.text !== "" && !unpaired && !["", "No settings yet"].includes(text(E.status));
  if (setup.step === 2 && answered && !setup.stay && setup.step < lastStep()) setup.step = setup.reached = 3;
  document.body.classList.toggle("setup", setup.step > 0);
  $("advanced").hidden = !setup.step;
  const steps = document.querySelectorAll(".step");
  for (const [i, card] of steps.entries()) {
    card.hidden = !setup.step || i + 1 > lastStep();
    card.classList.toggle("open", i + 1 === setup.step);
    card.classList.toggle("done", i + 1 !== setup.step && i + 1 <= setup.reached);
  }
  steps[1].querySelector(".summary").textContent = unpaired ? "" : "Created";
  const rows = ["vin-row", "area-row", "plan-row"].map($);
  const homes = [steps[0], steps[2], steps[3]].map((card) => card.querySelector(".body"));
  if (setup.step) {
    for (const [i, row] of rows.entries())
      if (row.parentNode !== homes[i]) homes[i].querySelector(".error, .save").before(row);
  } else if (rows[0].parentNode !== $("settings-card")) {
    $("set-battery")
      .closest("label")
      .before(...rows);
  }
  steps[0].querySelector(".summary").textContent = $("set-vin").value.toUpperCase();
  steps[2].querySelector(".summary").textContent = $("set-area").selectedOptions[0]?.text ?? "";
  steps[3].querySelector(".summary").textContent = $("set-plan").selectedOptions[0]?.text ?? "";
}

// Save: on to the next step. VIN's, Validate VIN, checks the VIN and saves it, as the board needs it to find the car,
// with the guesses and no prices yet on a new board; the last step saves the prices, which restarts the board. Both
// keep the settings the setup doesn't show.
async function nextStep() {
  if (setup.step === 1) {
    const problem = vinProblem($("set-vin").value.trim().toUpperCase());
    $("vin-error").textContent = problem;
    $("vin-error").hidden = !problem;
    if (problem) return;
  }
  const fields = document.querySelectorAll(".step.open input, .step.open select");
  const wrong = [...fields].find((field) => !field.checkValidity());
  if (wrong) return wrong.reportValidity();
  if (setup.step === 1 && $("set-vin").value.trim().toUpperCase() !== readSettings(settings.text).values.tesla_vin) {
    if (!$("set-zone").value) $("set-zone").value = "Europe/Brussels"; // for now, if the phone's isn't one the board knows
    if (!(await sendSettings(settings.text === "" ? formSettings(false) : withForm(["tesla_vin"])))) return;
  }
  if (setup.step < lastStep()) {
    setup.step += 1;
    setup.reached = Math.max(setup.reached, setup.step);
    setup.stay = false;
    return requestRender();
  }
  const file = unfinished() ? withForm(["market", "tariff", "timezone"]) : settings.text;
  if (file !== settings.text && !(await sendSettings(file))) return;
  setup.saving = file !== settings.text;
  setup.step = 0;
  requestRender();
}

// The plans of the market area's country, with the one to select if it's among them; no row where there are none.
function fillPlans(plan) {
  const country = $("set-area").value.slice(0, 2).toLowerCase();
  const plans = settings.options.plans.filter(([name]) => name.startsWith(`${country}/`));
  $("set-plan").replaceChildren(new Option("None", ""), ...plans.map(([name, title]) => new Option(title, name)));
  $("set-plan").value = plans.some(([name]) => name === plan) ? plan : "";
  $("plan-row").hidden = !plans.length;
}

// A new board's guesses for its market area: the VAT, and the area's time zone unless the phone's is one the board
// knows.
function guessFromArea() {
  const area = $("set-area").value;
  $("set-vat").value = vatOf(area);
  const phone = Intl.DateTimeFormat().resolvedOptions().timeZone;
  if (!settings.options.time_zones.includes(phone)) $("set-zone").value = COUNTRIES[area.slice(0, 2)]?.[1] ?? "";
}

// The form, from the board's settings file and what it offers. What the file doesn't have starts as the phone's time
// zone, the market area's VAT, a 75 kWh battery and 11 kW, and on a new board, the market area of the phone's
// country, where it has only one.
function fillSettings() {
  const { values, more } = readSettings(settings.text);
  const { areas, time_zones: zones } = settings.options;
  const country = new Intl.DisplayNames(["en"], { type: "region" });
  const areaOptions = areas.map((area) => new Option(`${country.of(area.slice(0, 2))} (${area})`, area));
  areaOptions.sort((a, b) => a.text.localeCompare(b.text));
  $("set-area").replaceChildren(new Option("Choose", ""), ...areaOptions);
  const phone = Intl.DateTimeFormat().resolvedOptions().timeZone;
  const home = Object.keys(COUNTRIES).find((code) => COUNTRIES[code].includes(phone));
  const ofHome = unfinished() ? areas.filter((area) => area.slice(0, 2) === home) : [];
  $("set-area").value = (values["market: area"] ?? (ofHome.length === 1 ? ofHome[0] : "")).toUpperCase();
  fillPlans(values["tariff: plan"]);
  const zone = values.timezone ?? phone;
  $("set-zone").replaceChildren(new Option("Choose", ""), ...zones.map((name) => new Option(name, name)));
  $("set-zone").value = zones.includes(zone) ? zone : "";
  const vat = values["market: vat"];
  $("set-vat").value = vat ? Math.round(Number(vat) * 10000) / 100 : vatOf($("set-area").value);
  $("set-margin").value = values["market: margin"] ?? "";
  $("set-vin").value = values.tesla_vin ?? "";
  $("set-battery").value = values.tesla_battery_kwh ?? 75;
  $("set-power").value = values.tesla_charging_kw ?? 11;
  $("set-topic").value = values.ntfy_topic ?? "";
  $("settings-more").hidden = !more;
  $("settings-error").hidden = true;
}

// Reads the board's settings and what the form offers, when the page connects and again when the board comes back,
// as it does after saving settings.
async function loadSettings() {
  try {
    const [file, options] = await Promise.all([
      fetch("/settings").then((r) => r.text()),
      fetch("/settings/options").then((r) => r.json()),
    ]);
    if (file !== settings.text || !settings.options) {
      // the setup starts again from what the board has now, as after an upload
      Object.assign(setup, { step: 0, saving: false, stay: false });
      settings.text = file;
      settings.options = options;
      fillSettings();
    }
  } catch {
    // the next connection tries again
  }
  requestRender();
}

// A settings file's top-level settings, each with the lines below it, comments included; "" holds those above the
// first.
function settingsBlocks(text) {
  const blocks = new Map([["", ""]]);
  let key = "";
  for (const line of text ? text.replace(/\n$/, "").split("\n") : []) {
    if (/^[a-z_]+:/.test(line)) key = line.slice(0, line.indexOf(":"));
    blocks.set(key, `${blocks.get(key) ?? ""}${line}\n`);
  }
  return blocks;
}

// The board's settings file with the form's `keys`, top-level settings with the lines below them, put in, and the
// rest kept, in the order the form writes them.
function withForm(keys) {
  const blocks = settingsBlocks(settings.text);
  const form = settingsBlocks(formSettings());
  for (const key of keys) {
    if (form.has(key)) blocks.set(key, form.get(key));
    else blocks.delete(key);
  }
  return [...blocks.keys()]
    .sort()
    .map((key) => blocks.get(key))
    .join("");
}

// The form as a settings file, which the board checks as any other; without prices, as the setup's first step saves
// them for a new board.
function formSettings(prices = true) {
  const v = (id) => $(id).value.trim();
  const lines = [];
  if (prices) {
    lines.push("market:", `  area: ${v("set-area")}`);
    if (v("set-margin")) lines.push(`  margin: ${v("set-margin")}`);
    lines.push(`  vat: ${Number(v("set-vat")) / 100}`);
  }
  if (v("set-topic")) lines.push(`ntfy_topic: ${v("set-topic")}`);
  if (prices && v("set-plan")) lines.push("tariff:", `  plan: ${v("set-plan")}`);
  lines.push(
    `tesla_battery_kwh: ${v("set-battery")}`,
    `tesla_charging_kw: ${v("set-power")}`,
    `tesla_vin: ${v("set-vin").toUpperCase()}`,
    `timezone: ${v("set-zone")}`,
  );
  return `${lines.join("\n")}\n`;
}

// Sends a settings file. The board checks it and restarts with it, or answers what's wrong, which stays on the page
// until the next try.
async function sendSettings(file) {
  const abort = new AbortController();
  const timer = setTimeout(() => abort.abort(), 8000);
  try {
    const response = await fetch("/settings", {
      method: "POST",
      headers: { "Content-Type": "text/plain" },
      body: file,
      signal: abort.signal,
    });
    const answer = await response.text();
    $("settings-error").textContent = response.ok ? "" : `Not saved: ${answer}`;
    $("settings-error").hidden = response.ok || setup.step > 0;
    if (!response.ok && setup.step) toast(`Not saved: ${answer}`);
    if (response.ok) {
      settings.open = false;
      // A board that had settings restarts: reconnect soon, rather than when the browser would. A new board takes
      // its first at once, and the setup moves on.
      if (settings.text !== "") {
        toast("Settings saved: the board restarts");
        setTimeout(reconnect, 3000);
      }
    }
    return response.ok;
  } catch (e) {
    toast(`Didn't work (${e.name === "AbortError" ? "no answer" : e.message}). Try again.`);
    return false;
  } finally {
    clearTimeout(timer);
    requestRender();
  }
}

// Save: the form's own checks first, as the browser shows them by the field.
async function saveSettings() {
  $("set-vin").setCustomValidity(vinProblem($("set-vin").value.trim().toUpperCase())); // the setup's checks
  const fields = document.querySelectorAll("#settings-card input:not([type=file]), #settings-card select");
  const wrong = [...fields].find((field) => !field.checkValidity());
  if (wrong) wrong.reportValidity();
  else await sendSettings(formSettings());
}

// --- Board link ------------------------------------------------------------

let events;
let lastEvent = 0;
function connect() {
  events = new EventSource("/events");
  const seen = () => {
    lastEvent = Date.now();
  };
  seen();
  events.onopen = () => setLive(true);
  events.onerror = () => setLive(false);
  events.addEventListener("ping", seen);
  events.addEventListener("state", (e) => {
    seen();
    const data = JSON.parse(e.data);
    states[data.id] = data;
    // A new board takes its first settings without a restart: read them once its status moves on.
    if (data.id === E.status && settings.text === "" && data.state !== "No settings yet") void loadSettings();
    requestRender();
  });
}

// The board pings every 10 s. A connection it dropped without closing (Restart, a power cycle) stays open with
// nothing arriving, so reconnect after 30 s without an event. iOS suspends pages in the background, and the board
// asks browsers to wait 30 s before reconnecting: reconnect at once when the page comes back.
function reconnectIfDead() {
  if (document.hidden || !events) return;
  if (events.readyState === EventSource.OPEN && Date.now() - lastEvent < 30000) return;
  reconnect();
}
function reconnect() {
  setLive(false);
  events.close();
  connect();
}
document.addEventListener("visibilitychange", reconnectIfDead);
setInterval(reconnectIfDead, 10000);

function setLive(on) {
  if (live === on) return;
  live = on;
  if (on) void loadSettings(); // it catches its own errors
  requestRender();
}

async function post(entity, action, param) {
  const [domain, name] = entity.split("/");
  const query = param == null ? "" : `?value=${encodeURIComponent(param)}`;
  // The board answers at once; a restarting or absent one never does. An AbortController rather than
  // AbortSignal.timeout(), which Safari got only in 16.4.
  const abort = new AbortController();
  const timer = setTimeout(() => abort.abort(), 8000);
  try {
    const response = await fetch(`/${domain}/${encodeURIComponent(name)}/${action}${query}`, {
      method: "POST",
      body: "",
      signal: abort.signal,
    });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    return true;
  } catch (e) {
    toast(`Didn't work (${e.name === "AbortError" ? "no answer" : e.message}). Try again.`);
    return false;
  } finally {
    clearTimeout(timer);
  }
}

// One press at a time: the pressed button stays off until its requests settle, so a second tap can't repeat them,
// and renderSchedule() keeps Create schedule off meanwhile.
let busy = false;
function press(button, handler) {
  button.addEventListener("click", async () => {
    if (busy) return;
    busy = true;
    button.disabled = true;
    try {
      await handler();
    } finally {
      busy = false;
      button.disabled = false;
      requestRender();
    }
  });
}

let toastTimer;
function toast(message) {
  const el = $("toast");
  el.textContent = message;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => (el.textContent = ""), 2800);
}

// --- Time and text ---------------------------------------------------------

const DAY_MS = 24 * 60 * 60 * 1000;
const pad = (n) => String(n).padStart(2, "0");
function hhmm(ms) {
  const d = new Date(ms);
  return `${pad(d.getHours())}:${pad(d.getMinutes())}`;
}
const midnight = (ms) => new Date(ms).setHours(0, 0, 0, 0);
const daysAhead = (ms) => Math.round((midnight(ms) - midnight(Date.now())) / DAY_MS);
// " +1" after a time tomorrow, " +2" the day after, like a flight's arrival; nothing today.
function plus(ms) {
  const days = daysAhead(ms);
  return days ? ` +${days}` : "";
}

// The next time the clock shows "HH:MM": today if it's still ahead, else tomorrow.
function nextAt(time) {
  const [h, m] = time.split(":").map(Number);
  const d = new Date();
  d.setHours(h, m, 0, 0);
  if (d > Date.now()) return d.getTime();
  d.setDate(d.getDate() + 1);
  d.setHours(h, m, 0, 0); // again, as moving across a summer-time change can shift the clock time
  return d.getTime();
}

// The board's local "YYYY-MM-DD HH:MM:SS" (datetime entity) from and to a timestamp.
function parseBoardTime(local) {
  const m = /^(\d{4})-(\d{2})-(\d{2}) (\d{2}):(\d{2})/.exec(local);
  if (!m) return null;
  const [, y, mo, d, h, min] = m.map(Number);
  return new Date(y, mo - 1, d, h, min).getTime();
}
function boardTime(ms) {
  const d = new Date(ms);
  return `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())} ${hhmm(ms)}:00`;
}

const dbm = (v) => (v == null ? "-" : `${v.toFixed(0)} dBm`);

function duration(seconds) {
  if (seconds == null || seconds <= 0) return "-";
  const minutes = Math.round(seconds / 60);
  const hours = Math.floor(minutes / 60);
  const days = Math.floor(hours / 24);
  if (days) return `${days} d ${hours % 24} h`;
  if (hours) return `${hours} h ${minutes % 60} min`;
  return `${minutes} min`;
}

// --- Start -----------------------------------------------------------------

// A button that asks first, then presses the board's button.
function confirmPress(selector, entity, message, question) {
  for (const button of document.querySelectorAll(selector))
    press(button, async () => {
      if (!confirm(question)) return;
      if (await post(entity, "press")) toast(message);
    });
}

function bind() {
  $("limit-select").addEventListener("change", (e) => (draft.limit = Number(e.target.value)));
  $("ready-select").addEventListener("change", (e) => (draft.deadline = Number(e.target.value)));
  press($("create-schedule"), createSchedule);
  press($("charge-now"), chargeNow);
  press($("delete-schedule"), () => stopCharging("Schedule deleted"));
  press($("stop-charging"), () => stopCharging("Charging stopped"));
  confirmPress(
    "#pair",
    E.pair,
    "Creating the key: tap your key card",
    "Create a new key? Sit in the car and tap your key card on the console when asked.",
  );
  confirmPress(
    "#reset-savings",
    E.resetSavings,
    "Savings reset",
    "Reset the savings? They start again from zero today.",
  );
  confirmPress(
    ".factory-reset",
    E.factoryReset,
    "Erasing: the board restarts as new",
    "Erase the board's settings, Wi-Fi, car key and savings? It restarts as a new board, without Wi-Fi.",
  );
  $("set-area").addEventListener("change", () => {
    fillPlans($("set-plan").value);
    if (unfinished()) guessFromArea();
  });
  press($("save-settings"), saveSettings);
  press($("pair-now"), async () => {
    if (await post(E.pair, "press")) toast("Creating the key: tap your key card");
  });
  for (const [i, card] of document.querySelectorAll(".step").entries()) {
    // a card the setup got to before opens with a click, and stays open
    card.addEventListener("click", () => {
      if (!card.classList.contains("done")) return;
      setup.step = i + 1;
      setup.stay = true;
      requestRender();
    });
    if (i === 1) continue; // Key has only Create key, as it moves on once the car answers
    const body = card.querySelector(".body");
    body.insertAdjacentHTML("beforeend", `<button class="primary save">${i ? "Save" : "Validate VIN"}</button>`);
    press(body.querySelector(".save"), nextStep);
    if (i === 0) continue; // nothing comes before VIN
    body.insertAdjacentHTML("beforeend", '<button class="back">Back</button>');
    body.querySelector(".back").addEventListener("click", () => {
      setup.step -= 1;
      setup.stay = true;
      requestRender();
    });
  }
  // Restart board, under Board and the setup's Advanced: the page reconnects soon after, rather than when the
  // browser would.
  for (const button of document.querySelectorAll(".restart"))
    press(button, async () => {
      if (!confirm("Restart the board?")) return;
      if (await post(E.restart, "press")) {
        toast("Restarting …");
        setTimeout(reconnect, 3000);
      }
    });
  $("change-settings").addEventListener("click", () => {
    settings.open = true;
    requestRender();
    $("settings-card").scrollIntoView({ behavior: "smooth" });
  });
  $("cancel-settings").addEventListener("click", () => {
    settings.open = false;
    fillSettings(); // back to the board's
    requestRender();
  });
  // Upload settings, under the settings form and the setup's Advanced: a settings file of the user's own.
  for (const button of document.querySelectorAll(".upload"))
    button.addEventListener("click", () => $("settings-file").click());
  $("settings-file").addEventListener("change", async (e) => {
    const [file] = e.target.files;
    e.target.value = ""; // so the same file can go again
    if (file) await sendSettings(await file.text());
  });
}

// Last, so every declaration above is initialised before the page starts.
if (new URLSearchParams(location.search).has("full")) {
  document.querySelector('link[href="/0.css"]')?.remove();
  const script = document.createElement("script");
  script.src = "https://oi.esphome.io/v2/www.js";
  document.body.append(script);
} else {
  document.title = "Tesla charging";
  document.body.insertAdjacentHTML("afterbegin", PAGE);
  bind();
  connect();
}
