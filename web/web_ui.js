// Phone-first page for the Tesla BLE board, served by ESPHome's web server in place of its default page (js_url: "").
// It reads live state from /events and sends actions to the REST API (POST /<domain>/<entity name>/<action>), and
// shows only what the charging-only key can use. Add ?full to the address for ESPHome's own page (needs internet).
//
//   Page           entity names (E), head tags and the page markup
//   State          what the board reported, the connection, draft (picked, not sent), pending (sent, not reported)
//   Render         one render per frame, from state to the page
//   Charge limit   the limit dropdown and sending it
//   Ready by       the deadline dropdown and sending it: the daily time and the one-off
//   Plan           the plan card: the mode, the charge windows and the plan buttons
//   Savings        the savings card
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
  createPlan: "button/Create charging plan",
  limit: "number/Charging Limit",
  mode: "text_sensor/Charging mode",
  pair: "button/Pair BLE Key",
  plugged: "binary_sensor/Charger",
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
    '<meta name="theme-color" content="#f2f2f7" media="(prefers-color-scheme: light)">' +
    '<meta name="theme-color" content="#000000" media="(prefers-color-scheme: dark)">',
);

const PAGE = `
<main>
  <section class="card">
    <div class="row"><span>Current charge</span><strong id="soc">-</strong></div>
    <div class="row"><span>Status</span><strong id="status">Connecting …</strong></div>
  </section>

  <section id="target-card" class="card">
    <label class="row"><span>Charge limit</span><span class="dropdown"><select id="limit-select" aria-label="Charge limit"></select></span></label>
    <label id="ready-row" class="row"><span>Ready by</span><span class="dropdown"><select id="ready-select" aria-label="Ready by"></select></span></label>
  </section>

  <section id="plan-card" class="card" hidden>
    <div id="plan-start">
      <button id="create-plan" class="primary">Create charging plan</button>
      <div class="or">or</div>
      <button id="charge-now">Start charging now</button>
    </div>
    <div id="plan-rows">
      <div id="windows"></div>
      <button id="delete-plan" class="red">Delete charging plan</button>
    </div>
    <button id="stop-charging" class="red">Stop charging</button>
  </section>

  <section id="savings-card" class="card" hidden>
    <div class="title">Savings</div>
    <div class="row"><span id="month-label">Last 30 days</span><strong id="saved-month">-</strong></div>
    <div id="year-row" class="row"><span id="year-label">Last 12 months</span><strong id="saved-year">-</strong></div>
    <p id="against-average" class="note"></p>
    <p id="against-at-once" class="note"></p>
    <button id="reset-savings" class="danger">Reset savings</button>
  </section>

  <details class="card">
    <summary>Board</summary>
    <div class="row"><span>Bluetooth</span><strong id="ble">-</strong></div>
    <div class="row"><span>Wi-Fi</span><strong id="wifi">-</strong></div>
    <div class="row"><span>Uptime</span><strong id="uptime">-</strong></div>
    <div class="row"><span>Version</span><strong id="version">-</strong></div>
    <button id="pair">Pair BLE key</button>
    <button id="restart" class="danger">Restart board</button>
  </details>

  <div id="toast" class="toast" role="status"></div>
</main>`;

// --- State -----------------------------------------------------------------

const states = {}; // entity id -> latest state event
// null until the first connection, then whether live updates from the board are coming in. The Status row says when
// they aren't.
let live = null;
// Charge limit and Ready by picked here but not sent yet: the plan buttons send them.
const draft = { limit: null, deadline: null };
// The mode a button should bring and the limit just sent, as { value, until }, shown until the board reports them or
// `until` passes.
const pending = { mode: null, limit: null };

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
function scheduleRender() {
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
  renderPlan(); // first: it drops the draft when the dropdowns can't change
  renderLimit();
  renderReady();
  renderSavings();

  $("ble").textContent = dbm(value(E.ble));
  $("wifi").textContent = dbm(value(E.wifi));
  $("uptime").textContent = duration(value(E.uptime));
  $("version").textContent = text(E.version) || "-";
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

// The half-hours from now to the end of the published prices, since the board doesn't plan on guesses. The time picked
// becomes the daily Ready by, which the board remembers; a later day than that time's next occurrence is a one-off.

const HALF_HOUR_MS = 30 * 60 * 1000;
const CLEAR_ONCE = "2000-01-01 00:00:00"; // Ready by once's "unset" value
// After Create charging plan, keeps the dropdown on the deadline just sent for a moment, so the board's old one doesn't
// flash back.
let readyLockUntil = 0;

// The daily time, the one-off while it's ahead, and the deadline in force.
function readyBy() {
  const daily = text(E.readyBy).slice(0, 5);
  const once = parseBoardTime(text(E.readyByOnce));
  if (once != null && once > Date.now()) return { daily, once, deadline: once };
  return { daily, once: null, deadline: /^\d\d:\d\d$/.test(daily) ? nextAt(daily) : null };
}

// Fills the dropdown with each half-hour that can be picked, "23:30" or "07:00 +1", and the deadline
// shown, greyed out when it can't be (a daily time whose prices aren't out yet).
function fillReady(deadline) {
  const now = Date.now();
  const until = pricesUntil();
  const times = [];
  for (let t = (Math.floor(now / HALF_HOUR_MS) + 1) * HALF_HOUR_MS; t <= until; t += HALF_HOUR_MS) times.push(t);
  if (!times.includes(deadline)) times.push(deadline);
  times.sort((a, b) => a - b);
  const options = times.map((t) =>
    Object.assign(new Option(`${hhmm(t)}${plus(t)}`, t), { disabled: t <= now || t > until }),
  );
  setOptions($("ready-select"), options, deadline);
}

function renderReady() {
  $("ready-row").hidden = pricesUntil() <= Date.now(); // nothing to pick until the board has prices
  if (Date.now() < readyLockUntil || document.activeElement === $("ready-select")) return;
  const deadline = draft.deadline ?? readyBy().deadline;
  if (deadline != null) fillReady(deadline);
}

// Sends the daily time ("HH:MM") and the one-off (a timestamp, or null to clear it) where they differ from `current`
// (readyBy()); false if sending failed.
async function sendReadyBy(time, once, current) {
  if (time !== current.daily && !(await post(E.readyBy, "set", `${time}:00`))) return false;
  if (once === current.once) return true;
  return post(E.readyByOnce, "set", once == null ? CLEAR_ONCE : boardTime(once));
}

// --- Plan ------------------------------------------------------------------

// The board's mode ("plan", "now", "none" or "wait"), or the one a button just asked for until the board has it.
function shownMode() {
  const board = text(E.mode) || "wait";
  pending.mode = stillPending(pending.mode, board);
  return pending.mode?.value ?? board;
}
function expectMode(mode) {
  pending.mode = { value: mode, until: Date.now() + 10000 };
  scheduleRender();
}

// Both buttons, the plan with Delete, or Stop while charging regardless of price. Charge limit and Ready by are for a
// new plan or charge, so only then can they change.
function renderPlan() {
  const plugged = states[E.plugged]?.value === true;
  const mode = shownMode();
  $("target-card").hidden = mode === "wait"; // nothing to set or show until the board can plan
  $("plan-card").hidden = !plugged || mode === "wait";
  $("plan-start").hidden = mode !== "none";
  $("plan-rows").hidden = mode !== "plan";
  $("stop-charging").hidden = mode !== "now";
  const editable = plugged && mode === "none";
  if (!editable) draft.limit = draft.deadline = null;
  for (const id of ["limit-select", "ready-select"]) $(id).disabled = !editable;
  // No plan without prices: the board downloads them after it starts, which takes about a minute.
  const priced = pricesUntil() > Date.now();
  $("create-plan").disabled = !priced || busy;
  $("create-plan").textContent = priced ? "Create charging plan" : "Getting prices …";
  renderWindows();
}

// "<currency>;<start>,<end>,<price>[,spare];..." from format_windows() in charging.h: the windows in UTC
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

// Sends what changed in the draft, then asks the board to plan, which cancels Start charging now.
async function createPlan() {
  const current = readyBy();
  let deadline = draft.deadline ?? current.deadline;
  if (deadline == null) return;
  const time = hhmm(deadline);
  if (deadline <= Date.now()) deadline = nextAt(time);
  const once = deadline === nextAt(time) ? null : deadline;
  fillReady(deadline);
  readyLockUntil = Date.now() + 3000;
  const ok = (await sendLimit()) && (await sendReadyBy(time, once, current)) && (await post(E.createPlan, "press"));
  if (ok) {
    expectMode("plan");
    toast(`Plan: ${shownLimit()}% by ${deadlineText(deadline)}`);
  } else {
    readyLockUntil = 0;
    scheduleRender();
  }
}

// Start charging now: charges at once to the limit shown (sending it if changed); Ready by doesn't apply.
async function chargeNow() {
  if ((await sendLimit()) && (await post(E.chargeNow, "press"))) {
    expectMode("now");
    toast("Charging now until you unplug");
  }
  scheduleRender();
}

// Stop charging and Delete charging plan: the board stops charging and keeps the car waiting until a
// button here, or the car is unplugged.
async function stopCharging(message) {
  if (await post(E.stopCharging, "press")) {
    expectMode("none");
    toast(message);
  }
}

// --- Savings ---------------------------------------------------------------

// "<currency>;<since>;<last 30 days>;<last 365 days>" from format_savings() in charging.h, each period as
// "<Wh>,<paid>,<at the day's average>,<at once>", the money in hundredths and since as the board's day number. Saved is
// the difference. A period that reaches back to when counting began says so, and the year shows once it differs from
// the month.
function renderSavings() {
  const [currency, since, ...periods] = text(E.savings).split(";");
  $("savings-card").hidden = !currency;
  if (!currency) return;
  const [month, year] = periods.map((period) => period.split(",").map(Number));
  const money = (hundredths) => `${(hundredths / 100).toFixed(2)}\u00a0${currency}`; // one piece when it wraps
  const counted = today() - Number(since); // whole days before today
  const start = dayText(Number(since));
  const monthText = counted < 30 ? `since ${start}` : "in the last 30 days";
  $("month-label").textContent = counted < 30 ? `Since ${start}` : "Last 30 days";
  $("saved-month").textContent = money(month[2] - month[1]);
  $("year-row").hidden = counted < 30;
  $("year-label").textContent = counted < 365 ? `Since ${start}` : "Last 12 months";
  $("saved-year").textContent = money(year[2] - year[1]);
  $("against-average").textContent =
    `Compared with the day's average price: ${Math.round(month[0] / 1000)} kWh for ${money(month[1])} ${monthText}`;
  $("against-at-once").textContent =
    `Compared with charging at once on plug-in: ${money(month[3] - month[1])} saved ${monthText}`;
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
    scheduleRender();
  });
}

// The board pings every 10 s. A connection it dropped without closing (Restart, a power cycle) stays open with
// nothing arriving, so reconnect after 30 s without an event. iOS suspends pages in the background, and the board
// asks browsers to wait 30 s before reconnecting: reconnect at once when the page comes back.
function reconnectIfDead() {
  if (document.hidden || !events) return;
  if (events.readyState === EventSource.OPEN && Date.now() - lastEvent < 30000) return;
  setLive(false);
  events.close();
  connect();
}
document.addEventListener("visibilitychange", reconnectIfDead);
setInterval(reconnectIfDead, 10000);

function setLive(on) {
  if (live === on) return;
  live = on;
  scheduleRender();
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
// and renderPlan() keeps Create charging plan off meanwhile.
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
      scheduleRender();
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

// "Today 23:30", "Tomorrow 07:00" or "Mon 00:00".
function deadlineText(ms) {
  const days = daysAhead(ms);
  const day =
    days === 0 ? "Today" : days === 1 ? "Tomorrow" : new Date(ms).toLocaleDateString("en-GB", { weekday: "short" });
  return `${day} ${hhmm(ms)}`;
}

// Today's day number (days since 1970-01-01, local time), as the board counts them, and "2 Oct 2026" for one.
const today = () => Math.floor((Date.now() - new Date().getTimezoneOffset() * 60000) / DAY_MS);
function dayText(day) {
  const options = { day: "numeric", month: "short", year: "numeric", timeZone: "UTC" };
  return new Date(day * DAY_MS).toLocaleDateString("en-GB", options);
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
function confirmPress(elementId, entity, message, question) {
  press($(elementId), async () => {
    if (!confirm(question)) return;
    if (await post(entity, "press")) toast(message);
  });
}

function bind() {
  $("limit-select").addEventListener("change", (e) => (draft.limit = Number(e.target.value)));
  $("ready-select").addEventListener("change", (e) => (draft.deadline = Number(e.target.value)));
  press($("create-plan"), createPlan);
  press($("charge-now"), chargeNow);
  press($("delete-plan"), () => stopCharging("Charging plan deleted"));
  press($("stop-charging"), () => stopCharging("Charging stopped"));
  confirmPress(
    "pair",
    E.pair,
    "Pairing started: tap your key card",
    "Pair a new key? Sit in the car and tap your key card on the console when asked.",
  );
  confirmPress(
    "reset-savings",
    E.resetSavings,
    "Savings reset",
    "Reset the savings? They start again from zero today.",
  );
  confirmPress("restart", E.restart, "Restarting …", "Restart the board?");
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
