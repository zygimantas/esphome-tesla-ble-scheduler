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
//   QR code        the page's address as a QR code, for the setup on a computer
//   Start          wiring, then this page or ESPHome's (?full)

// --- Page ------------------------------------------------------------------

const $ = (id) => document.getElementById(id);

const E = {
  battery: "sensor/Battery",
  ble: "sensor/BLE Signal",
  chargeNow: "button/Start charging now",
  charging: "text_sensor/Charging",
  createSchedule: "button/Create schedule",
  firmware: "update/Firmware",
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
    '<meta name="apple-mobile-web-app-title" content="ETBS">' +
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

// How a plan the list doesn't have gets into it, and what to do until then.
const REPOSITORY = "https://github.com/zygimantas/esphome-tesla-ble-scheduler";
const planLinks = (ask) =>
  `<a href="${REPOSITORY}/issues/new?template=grid_plan.yml" target="_blank" rel="noopener">${ask}</a>, or ` +
  `<a href="${REPOSITORY}/blob/main/CONTRIBUTING.md#plans" target="_blank" rel="noopener">add it yourself</a>, and ` +
  "choose it in Change settings, under Board, once a release brings it. Until then, fees that change with the hour " +
  "can go in a settings file, uploaded there.";

// A field's "?", which opens its hint under the field.
const info = (hint, label) =>
  `<button type="button" class="info" data-hint="${hint}" aria-label="${label}" aria-expanded="false">?</button>`;

const PAGE = `
<header class="bar">
  ${LOGO}
  <div><h1>ESPHome Tesla BLE Scheduler</h1><p>Charges when it's cheapest</p></div>
  <span id="link" class="pill">Connecting …</span>
</header>
<main>
  <section id="update-card" class="card" hidden>
    <div class="title">Update available</div>
    <p class="note">Release <span id="update-version"></span> is out: <a id="update-notes" target="_blank" rel="noopener">what's new</a>. The board downloads it and restarts, in about a minute.</p>
    <button id="update" class="primary">Update</button>
  </section>

  <section id="status-card" class="card">
    <div class="row"><span>Current charge</span><strong id="soc">-</strong></div>
    <div class="row"><span>Status</span><strong id="status">Connecting …</strong></div>
  </section>

  <section id="plan-card" class="card" hidden>
    <div class="title">No grid plan</div>
    <p class="note"><span id="plan-why"></span> ${planLinks("Ask for your plan")}</p>
  </section>

  <section id="settings-card" class="card" hidden>
    <div class="title">Settings</div>
    <label id="vin-row" class="row"><span>VIN${info("vin-hint", "About the VIN")}</span><input id="set-vin" class="wide" placeholder="17 letters and digits" required pattern="[A-HJ-NPR-Z0-9]{17}" title="17 letters and digits, none of them I, O or Q, on the car's screen under Controls, Software" autocapitalize="characters" autocomplete="off" autocorrect="off" spellcheck="false"></label>
    <p id="vin-hint" class="note hint" hidden>The board needs your car's VIN to find it over Bluetooth and talk to it. It's on the car's screen under Controls → Software, and at the bottom of the Tesla app's home screen. It stays on the board.</p>
    <label id="area-row" class="row"><span>Country / Area${info("area-hint", "About the country or area")}</span><span class="dropdown"><select id="set-area" required></select></span></label>
    <p id="area-hint" class="note hint" hidden>Where you buy electricity: your country, or in Sweden, Norway and Denmark your price area, which your contract names. It sets the market prices, the VAT and the grid plans to choose from.</p>
    <label id="plan-row" class="row"><span>Grid plan${info("plan-hint", "About the grid plan")}</span><span class="dropdown"><select id="set-plan" required></select></span></label>
    <p id="plan-hint" class="note hint" hidden>Your grid operator's plan, the part of your bill for bringing the electricity, which your bill names: a plan, a package or a tariff group. Its hours make some times cheaper, and the board charges when the grid fee and the supplier's price together cost the least.</p>
    <label id="unlisted-row" class="row check"><input id="set-unlisted" type="checkbox"><span>My plan isn't listed</span></label>
    <p id="plans-note" class="note">No grid plans here yet: ${planLinks("ask for yours")}</p>
    <p id="unlisted-note" class="note">${planLinks("Ask for your plan")}</p>
    <label id="price-row" class="row"><span>Contract type${info("price-hint", "About the contract type")}</span><span class="dropdown"><select id="set-price"><option value="market">Dynamic (spot, exchange)</option><option value="fixed">Fixed (or a monthly average)</option></select></span></label>
    <p id="price-hint" class="note hint" hidden>What your contract with the supplier says: Dynamic if its price follows the exchange or spot price by the hour, Fixed for a fixed price or one set by the month's average.</p>
    <label id="margin-row" class="row"><span>Supplier's margin${info("margin-hint", "About the supplier's margin")}</span><input id="set-margin" type="number" min="0" step="any" inputmode="decimal"></label>
    <p id="margin-hint" class="note hint" hidden>What your supplier adds per kWh on top of the exchange price, as your contract says. It doesn't change when the car charges, only the costs the page shows.</p>
    <label id="fixed-row" class="row"><span>Supplier's part${info("fixed-hint", "About the supplier's part")}</span><input id="set-fixed" type="number" min="0" step="any" inputmode="decimal"></label>
    <p id="fixed-hint" class="note hint" hidden>Your supplier's own price per kWh, without the grid fees, as on its line of the bill. It doesn't change when the car charges, only the costs the page shows.</p>
    <label id="battery-row" class="row"><span>Battery (kWh)${info("battery-hint", "About the battery")}</span><input id="set-battery" type="number" required min="20" max="200" step="any" inputmode="decimal" placeholder="75"></label>
    <p id="battery-hint" class="note hint" hidden>The battery's usable size tells the board how much to charge. A new board guesses it from the car's model: about 60 kWh for a standard range Model 3 or Y, 75 to 79 for a Long Range, 95 to 100 for a Model S or X.</p>
    <label id="power-row" class="row"><span>Charging power (kW)${info("power-hint", "About the charging power")}</span><input id="set-power" type="number" required min="1" max="22" step="any" inputmode="decimal" placeholder="11"></label>
    <p id="power-hint" class="note hint" hidden>What the Tesla app shows while the car charges at home, like 11 kW on three phases or 7.4 kW on one. With the battery's size, it tells the board how long charging takes.</p>
    <label class="row"><span>VAT (%)</span><input id="set-vat" type="number" required min="0" max="30" step="any" inputmode="decimal" placeholder="21"></label>
    <label class="row"><span>Time zone</span><span class="dropdown"><select id="set-zone" required></select></span></label>
    <label class="row"><span>ntfy topic</span><input id="set-topic" class="wide" maxlength="64" pattern="[A-Za-z0-9_\\-]{0,64}" title="The topic's name: up to 64 letters, digits, - and _" autocomplete="off" spellcheck="false" placeholder="none"></label>
    <p id="settings-more" class="note" hidden>Your settings have more than this form shows, which saving it drops: to keep it, change the file instead.</p>
    <p id="settings-error" class="note error" hidden></p>
    <button id="save-settings" class="primary">Save</button>
    <button id="cancel-settings">Cancel</button>
    <a class="button" href="/settings" download="settings.yaml">Download settings</a>
    <button id="upload-settings">Upload settings</button>
    <input id="settings-file" type="file" accept=".yaml,.yml,.txt" hidden>
  </section>

  <section id="phone-card" class="card" hidden>
    <div class="title">Continue on your phone</div>
    <div id="qr" class="qr"></div>
    <div>
      <p class="note">Scan the code with your phone's camera to open this page there, or open <span id="address"></span> on it. Then finish the setup in the car, with these:</p>
      <ul class="note">
        <li>The board</li>
        <li>A USB charger and cable for it</li>
        <li>Your Tesla key card</li>
      </ul>
    </div>
  </section>
  <section class="card step" hidden>
    <div class="title">Car<span class="summary">Saved</span></div>
    <div class="body">
      <p class="note error" hidden></p>
      <button class="primary save">Continue</button>
    </div>
  </section>
  <section class="card step" hidden>
    <div class="title">Key<span class="summary">Saved</span></div>
    <div class="body">
      <p class="note">The car only takes orders from keys it knows, so the board makes a key of its own for the car to add, like a phone key. It can only charge: it <strong>can't unlock or drive the car</strong>, and you can remove it in the car under Controls → Locks.</p>
      <ol class="note">
        <li>Make sure the board is plugged into a USB charger next to the car.</li>
        <li>Sit in the car with your Tesla key card.</li>
        <li>Press Continue below.</li>
        <li>Tap the Tesla key card on the console.</li>
        <li>Confirm on the car's screen.</li>
      </ol>
      <button id="pair-now" class="primary">Continue</button>
    </div>
  </section>
  <section class="card step" hidden>
    <div class="title">Prices</div>
    <div class="body">
      <p class="note error" hidden></p>
      <button class="primary save">Finish</button>
    </div>
  </section>

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
    <button id="restart" class="danger">Restart board</button>
  </details>

  <div id="toast" class="toast" role="status"></div>
</main>`;

// --- State -----------------------------------------------------------------

// The board's settings file ("" without one, null until read), what the form offers, from /settings/options, and
// whether Change settings opened the form.
const settings = { text: null, options: null, open: false };
// The setup's open step, 1 to 3, or 0; the furthest it got, as the steps up to it open with a click; and that a step
// was opened by hand, which keeps Key from moving on by itself.
const setup = { step: 0, reached: 0, stay: false };

const states = {}; // entity id -> latest state event
// null until the first connection, then whether live updates from the board are coming in. The Status row says when
// they aren't.
let live = null;
// When the page asked the board to restart, saving settings, with Restart board or with Update, or 0: until the board
// is back, the page shows only its status, and then loads afresh.
let restarting = 0;
// Whether that was Update, which downloads the release before the board restarts, or says it couldn't.
let updating = false;
// Charge limit and Ready by picked here but not sent yet: the schedule buttons send them.
const draft = { limit: null, deadline: null };
// The mode a button should bring, and the limit and Ready by just sent, as { value, until }, shown until the board
// reports them or `until` passes.
const pending = { mode: null, limit: null, deadline: null };

// Numbers arrive as JSON numbers from sensors but as strings ("80") from number entities.
const value = (id) => {
  const v = states[id]?.value;
  const n = typeof v === "string" && v.trim() !== "" ? Number(v) : v;
  return Number.isFinite(n) ? n : null;
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
  const link = live === null ? "Connecting …" : live ? "Connected" : "No connection";
  const status = live === false ? link : (text(E.status) || "Connecting …") + power;
  const waiting = updating ? "Updating …" : "Restarting …";
  $("status").textContent = restarting ? waiting : status;
  $("link").textContent = restarting ? waiting : link;
  document.body.classList.toggle("restarting", restarting > 0);
  renderSchedule(); // first: it drops the draft when the dropdowns can't change
  renderLimit();
  renderReady();
  renderSavings();

  $("ble").textContent = dbm(value(E.ble));
  $("wifi").textContent = dbm(value(E.wifi));
  $("uptime").textContent = duration(value(E.uptime));
  $("version").textContent = text(E.version) || "-";
  renderSetup();
  // A release the board found, which installs only from here.
  const release = states[E.firmware]?.value ?? "";
  $("update-card").hidden = text(E.firmware) !== "UPDATE AVAILABLE";
  $("update-version").textContent = release;
  $("update-notes").href = `${REPOSITORY}/releases/tag/v${release}`;
  // Without a grid plan or hours of the owner's own: what the board leaves out, and how a plan gets in.
  $("plan-card").hidden = !settings.text || unfinished() || /^tariff:/m.test(settings.text);
  $("plan-why").textContent = /^market:/m.test(settings.text)
    ? "Without it, the board picks the hours by the market price alone, without your grid fees."
    : "Without it, a fixed price costs the same in every hour, so the board charges at once.";
  // Settings the board can't use come first; others open from Board.
  const needed = text(E.status).startsWith("Settings: ");
  $("settings-card").hidden = setup.step > 0 || (!needed && !settings.open);
  const fixed = $("set-price").value === "fixed";
  for (const id of ["set-vat", "set-margin"]) $(id).closest("label").hidden = fixed;
  const unlisted = $("set-unlisted").checked;
  $("set-plan").disabled = unlisted;
  $("plans-note").hidden = !$("plan-row").hidden || !$("set-area").value;
  $("unlisted-note").hidden = !unlisted;
  $("fixed-row").hidden = !fixed;
  // the prices' names, with their unit, in the currency they're in; a fixed price's supplier part goes on top of the
  // grid plan's fees, so it's without them, even where the supplier quotes one price with them in
  const country = $("set-area").value.slice(0, 2);
  const currency = CURRENCIES[country] ?? "EUR";
  const unit = `${currency} with VAT per kWh`;
  $("margin-row").firstElementChild.firstChild.nodeValue = `Supplier's margin (${unit})`;
  $("fixed-row").firstElementChild.firstChild.nodeValue = `Supplier's part (${unit}, without grid fees)`;
  $("set-margin").max = $("set-fixed").max = String(EURO[currency] ?? 1);
  // a hint shows while its "?" is open and its field is shown
  for (const button of document.querySelectorAll(".info"))
    $(button.dataset.hint).hidden =
      button.getAttribute("aria-expanded") !== "true" || Boolean(button.closest("[hidden]"));
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
const countryOf = (zone) => Object.keys(COUNTRIES).find((code) => COUNTRIES[code].includes(zone));
// The countries without the euro, by their currency, which their prices are in: Nord Pool's market prices come in it,
// and the board converts SMARD's, in euros, at the ECB's daily rate where the settings set the currency.
const CURRENCIES = { CH: "CHF", CZ: "CZK", DK: "DKK", HU: "HUF", NO: "NOK", PL: "PLN", RO: "RON", SE: "SEK" };
const SMARD_ONLY = ["CH", "CZ", "HU"];
// The currency the form writes: a fixed price's, and a market's from SMARD, which the board otherwise keeps in euros.
const writtenCurrency = (area, fixed) =>
  fixed || SMARD_ONLY.includes(area.slice(0, 2)) ? CURRENCIES[area.slice(0, 2)] : undefined;
// About a euro in each of those currencies: the most a supplier's margin or part per kWh can be, which turns away cents
// typed for euros.
const EURO = { CHF: 1, CZK: 25, DKK: 7.5, HUF: 400, NOK: 12, PLN: 4.5, RON: 5, SEK: 12 };

// The places in the settings file that the form shows, like "market: area"; ntfy_server only as the default.
const FORM_PLACES = [
  "currency",
  "fixed_price",
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

// What's wrong with a VIN, or "": a Tesla's starts with one of its makers' codes, and its 9th character is a check
// digit, computed from the others as in North America, which Tesla does for its Shanghai and Berlin cars too.
const TESLA_MAKERS = ["5YJ", "7SA", "7G2", "LRW", "XP7"];
function vinProblem(vin) {
  if (vin.length !== 17) return `A VIN has 17 letters and digits, and this one has ${vin.length}.`;
  if (/[IOQ]/.test(vin)) return "A VIN has no I, O or Q: they're 1 or 0.";
  if (!/^[A-Z0-9]+$/.test(vin)) return "A VIN has only letters and digits.";
  if (!TESLA_MAKERS.includes(vin.slice(0, 3)))
    return `A Tesla's VIN starts with ${TESLA_MAKERS.slice(0, -1).join(", ")} or ${TESLA_MAKERS[TESLA_MAKERS.length - 1]}: check the first three.`;
  const value = (c) => (c <= "9" ? Number(c) : Number("12345678123457923456789"["ABCDEFGHJKLMNPRSTUVWXYZ".indexOf(c)]));
  const weights = [8, 7, 6, 5, 4, 3, 2, 10, 0, 9, 8, 7, 6, 5, 4, 3, 2];
  const sum = [...vin].reduce((total, c, i) => total + value(c) * weights[i], 0);
  return "0123456789X"[sum % 11] === vin[8] ? "" : "This VIN doesn't add up: a letter or digit is off. Check it again.";
}

// Whether the battery's size is the owner's, typed or saved, rather than a guess the VIN may change.
let batteryTyped = false;

// Whether the settings still have no prices: a market's, a tariff or a fixed price, the same in every hour without a
// grid plan.
const unfinished = () => settings.text !== null && !/^(fixed_price|market|tariff):/m.test(settings.text);
// The setup's last step: Key once the settings have prices, else Prices.
const lastStep = () => (unfinished() ? 3 : 2);

// A battery's usable size by a Tesla's model, the VIN's 4th character: a new board's guess, which the setup's first
// step fills in as the VIN is typed, for the owner to check, as the VIN tells the battery itself only in codes that
// differ by year and by source.
const BATTERIES = { S: 95, X: 95, 3: 75, Y: 75 };
const batteryOf = (vin) => BATTERIES[vin[3]] ?? 75;

// The setup, for a new board, settings without prices and a key the car doesn't know yet: the open step shows its
// fields, done ones fold to their titles and Saved (Prices to its title alone, as only its Finish saves it, which ends
// the setup), later ones show only their titles. Car and Prices borrow the form's rows, the VIN, battery and power
// and those from the country to the supplier's part, around their notes and above their buttons, and give them back
// to the form after.
function renderSetup() {
  const unpaired = text(E.status) === "Not paired";
  if (settings.text === null || !(unfinished() || unpaired)) setup.step = setup.reached = 0;
  else if (!setup.step) setup.step = setup.reached = settings.text === "" ? 1 : unpaired ? 2 : 3;
  if (unpaired) {
    // a key the car doesn't know closes the steps after Key
    setup.reached = Math.min(setup.reached, 2);
    setup.step = Math.min(setup.step, setup.reached);
  }
  // Key moves on by itself once the car answers, unless it was opened by hand
  const answered = settings.text !== "" && !unpaired && !["", "No settings yet"].includes(text(E.status));
  if (setup.step === 2 && answered && !setup.stay && setup.step < lastStep()) setup.step = setup.reached = 3;
  document.body.classList.toggle("setup", setup.step > 0 && !restarting);
  // On a computer, as after ESPHome Web's Visit Device, the setup goes on on the phone, in the car: the page's address
  // as a QR code in place of the steps.
  const computer = matchMedia("(pointer: fine)").matches;
  $("phone-card").hidden = !setup.step || !computer;
  const steps = document.querySelectorAll(".step");
  for (const [i, card] of steps.entries()) {
    card.hidden = !setup.step || computer || i + 1 > lastStep();
    card.classList.toggle("open", i + 1 === setup.step);
    card.classList.toggle("done", i + 1 !== setup.step && i + 1 <= setup.reached);
  }
  // Key's is saved once the car knows the key
  steps[1].querySelector(".summary").hidden = unpaired;
  const names = [
    "vin-row",
    "vin-hint",
    "area-row",
    "area-hint",
    "plan-row",
    "plan-hint",
    "unlisted-row",
    "plans-note",
    "unlisted-note",
    "price-row",
    "price-hint",
    "margin-row",
    "margin-hint",
    "fixed-row",
    "fixed-hint",
    "battery-row",
    "battery-hint",
    "power-row",
    "power-hint",
  ];
  const rows = names.map($);
  // each before its step's error: the car's fields, with their hints, in Car, the prices in Prices
  const places = rows.map((row) => steps[/^(vin|battery|power)-/.test(row.id) ? 0 : 2].querySelector(".error"));
  if (setup.step) {
    for (const [i, row] of rows.entries()) if (row.parentNode !== places[i].parentNode) places[i].before(row);
  } else if (rows[0].parentNode !== $("settings-card")) {
    $("set-vat")
      .closest("label")
      .before(...rows);
  }
}

// Save: on to the next step. Car's Continue checks the VIN and saves it with the battery and the power where the VIN
// changed, as the board needs it to find the car, with the guesses and no prices yet on a new board, and where the
// battery or the power changed and Key is the last step, as no Finish follows to save them; Prices' Finish saves the
// prices, the battery and the power, which restarts the board. Each says what's wrong on its card, and the saves keep
// the settings the setup doesn't show.
async function nextStep() {
  const vin = $("set-vin").value.trim().toUpperCase();
  const problem = setup.step === 1 ? vinProblem(vin) : "";
  const error = document.querySelector(".step.open .error");
  error.textContent = problem;
  error.hidden = !problem;
  if (problem) return;
  const fields = document.querySelectorAll(".step.open input, .step.open select");
  const wrong = [...fields].find((field) => !field.closest("[hidden]") && !field.checkValidity());
  if (wrong) return wrong.reportValidity();
  const car = ["tesla_battery_kwh", "tesla_charging_kw", "tesla_vin"];
  const saved = readSettings(settings.text).values;
  const form = readSettings(formSettings(false)).values;
  const changed = car.filter((key) => form[key] !== saved[key]);
  if (setup.step === 1 && (changed.includes("tesla_vin") || (lastStep() === 2 && changed.length))) {
    if (!$("set-zone").value) $("set-zone").value = "Europe/Brussels"; // for now, if the phone's isn't one the board knows
    if (!(await sendSettings(settings.text === "" ? formSettings(false) : withForm(car)))) return;
  }
  if (setup.step < lastStep()) {
    setup.step += 1;
    setup.reached = Math.max(setup.reached, setup.step);
    setup.stay = false;
    return;
  }
  const keys = ["currency", "fixed_price", "market", "tariff", "tesla_battery_kwh", "tesla_charging_kw", "timezone"];
  await sendSettings(withForm(keys));
}

// The grid plan chosen, or "" for one that isn't listed or none chosen yet.
const gridPlan = () => ($("set-plan").value === "none" ? "" : $("set-plan").value);

// The country's plans, with `plan` selected, and My plan isn't listed ticked for "", a home's real plan that
// isn't in plans/: a box under the list rather than an option in it, as no one should have to search a long list for
// a way out. A new board's, with `plan` undefined, starts at the country's only plan, as everyone in Spain and
// Slovenia pays it, or at Choose where there are several, as nearly every home is on one and skipping it would leave
// out its hours. Choose and Not listed show only in the closed list. No rows, and no box ticked, where the country has
// no plans, as its own note says the same.
function fillPlans(plan) {
  const country = $("set-area").value.slice(0, 2).toLowerCase();
  const plans = settings.options.plans.filter(([name]) => name.startsWith(`${country}/`));
  const closed = [new Option("Choose", ""), new Option("Not listed", "none")];
  for (const option of closed) option.disabled = option.hidden = true;
  $("set-plan").replaceChildren(...closed, ...plans.map(([name, title]) => new Option(title, name)));
  $("set-unlisted").checked = plan === "" && plans.length > 0;
  const fallback = plan === "" ? "none" : plans.length === 1 ? plans[0][0] : "";
  $("set-plan").value = plans.some(([name]) => name === plan) ? plan : fallback;
  $("plan-row").hidden = $("unlisted-row").hidden = !plans.length;
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
// zone, the market area's VAT, a 75 kWh battery and 11 kW, and on a new board, a dynamic price and the phone's
// country, where it has only one market area. Without a market, the supplier's price is fixed, and the country the
// grid plan's, the currency's or the time zone's, in the first of its market areas, as any of them does.
function fillSettings() {
  const { values, more } = readSettings(settings.text);
  const { areas, time_zones: zones } = settings.options;
  // a country by its name, with its market area where it has several, like "Sweden, SE3", or a part, "Italy (north)"
  const regions = new Intl.DisplayNames(["en"], { type: "region" });
  const areaName = (area) => {
    const [code, part] = [area.slice(0, 2), area.split("-")[1]];
    if (part) return `${regions.of(code)} (${part.toLowerCase()})`;
    return areas.filter((a) => a.startsWith(code)).length > 1 ? `${regions.of(code)}, ${area}` : regions.of(code);
  };
  const areaOptions = areas.map((area) => new Option(areaName(area), area));
  areaOptions.sort((a, b) => a.text.localeCompare(b.text));
  const phone = Intl.DateTimeFormat().resolvedOptions().timeZone;
  const plan = values["tariff: plan"] ?? "";
  const ofCurrency = Object.keys(CURRENCIES).find((code) => CURRENCIES[code] === values.currency?.toUpperCase());
  const country = unfinished()
    ? countryOf(phone)
    : plan.slice(0, 2).toUpperCase() || ofCurrency || countryOf(values.timezone);
  const ofCountry = areas.filter((area) => area.slice(0, 2) === country);
  const guess = ofCountry.length === 1 || !unfinished() ? ofCountry[0] : "";
  // Choose only where there's no guess, like a country with several market areas
  const area = (values["market: area"] ?? guess ?? "").toUpperCase();
  $("set-area").replaceChildren(...(areas.includes(area) ? [] : [new Option("Choose", "")]), ...areaOptions);
  $("set-area").value = areas.includes(area) ? area : "";
  $("set-price").value = values["market: area"] || unfinished() ? "market" : "fixed";
  fillPlans(unfinished() ? undefined : plan);
  const zone = values.timezone ?? phone;
  const known = zones.includes(zone);
  $("set-zone").replaceChildren(
    ...(known ? [] : [new Option("Choose", "")]),
    ...zones.map((name) => new Option(name, name)),
  );
  $("set-zone").value = known ? zone : "";
  const vat = values["market: vat"];
  $("set-vat").value = vat ? Math.round(Number(vat) * 10000) / 100 : vatOf($("set-area").value);
  $("set-margin").value = values["market: margin"] ?? "0.00";
  $("set-fixed").value = values.fixed_price ?? "0.00";
  $("set-vin").value = values.tesla_vin ?? "";
  $("set-battery").value = values.tesla_battery_kwh ?? 75;
  batteryTyped = values.tesla_battery_kwh !== undefined; // a saved size is the owner's
  $("set-power").value = values.tesla_charging_kw ?? 11;
  $("set-topic").value = values.ntfy_topic ?? "";
  // a currency of the file's own, other than the one the form writes, is more than the form shows
  const written = writtenCurrency($("set-area").value, $("set-price").value === "fixed");
  $("settings-more").hidden = !more && values.currency?.toUpperCase() === written;
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
    if (file !== settings.text) {
      // the setup starts again from what the board has now, as after an upload
      Object.assign(setup, { step: 0, stay: false });
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
  const fixed = prices && v("set-price") === "fixed";
  const currency = prices && writtenCurrency(v("set-area"), fixed);
  if (currency) lines.push(`currency: ${currency}`);
  // the supplier's fixed price goes on top of a grid plan's fees; even at 0, it says the contract is a fixed one
  if (fixed) lines.push(`fixed_price: ${v("set-fixed") || 0}`);
  if (prices && v("set-price") === "market") {
    lines.push("market:", `  area: ${v("set-area")}`);
    if (Number(v("set-margin"))) lines.push(`  margin: ${v("set-margin")}`);
    lines.push(`  vat: ${Number(v("set-vat")) / 100}`);
  }
  if (v("set-topic")) lines.push(`ntfy_topic: ${v("set-topic")}`);
  if (prices && gridPlan()) lines.push("tariff:", `  plan: ${gridPlan()}`);
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
  const saved = await send("/settings", { headers: { "Content-Type": "text/plain" }, body: file }, async (response) => {
    const answer = await response.text();
    $("settings-error").textContent = response.ok ? "" : `Not saved: ${answer}`;
    $("settings-error").hidden = response.ok || setup.step > 0;
    if (!response.ok && setup.step) toast(`Not saved: ${answer}`);
    if (response.ok) {
      settings.open = false;
      // A board that had settings restarts: the page waits for it, reconnecting soon rather than when the browser
      // would. A new board takes its first at once, and the setup moves on.
      if (settings.text !== "") {
        toast("Settings saved: the board restarts");
        restarting = Date.now();
        setTimeout(reconnect, 3000);
      }
    }
    return response.ok;
  });
  requestRender();
  return saved;
}

// Save: the form's own checks first, as the browser shows them by the field.
async function saveSettings() {
  $("set-vin").setCustomValidity(vinProblem($("set-vin").value.trim().toUpperCase())); // the setup's checks
  // the shown fields only, as a hidden one can't say what's wrong, and isn't saved
  const fields = document.querySelectorAll("#settings-card input, #settings-card select");
  const wrong = [...fields].find((field) => !field.closest("[hidden]") && !field.checkValidity());
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
  events.addEventListener("ping", (e) => {
    seen();
    // Each ping has the board's uptime: shorter than the time since the page asked for a restart, the board is back
    // from it, and the page loads afresh.
    if (restarting && JSON.parse(e.data).uptime * 1000 < Date.now() - restarting) {
      events.close(); // nothing more from the board into this page, like the states that follow the ping
      location.reload();
    }
  });
  events.addEventListener("state", (e) => {
    seen();
    const data = JSON.parse(e.data);
    states[data.id] = data;
    // A release that couldn't install, or an install that never started, as on a board that had just restarted and
    // not checked for releases yet, leaves the board as it was.
    if (updating && data.id === E.firmware && data.state !== "INSTALLING") {
      updating = false;
      restarting = 0;
      toast("The update didn't install. Try again later.");
    }
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

// POSTs to the board and hands its answer to read(); false, with a toast, if it fails. The board answers at once; a
// restarting or absent one never does. An AbortController rather than AbortSignal.timeout(), which Safari got only in
// 16.4.
async function send(url, init, read) {
  const abort = new AbortController();
  const timer = setTimeout(() => abort.abort(), 8000);
  try {
    return await read(await fetch(url, { ...init, method: "POST", signal: abort.signal }));
  } catch (e) {
    toast(`Didn't work (${e.name === "AbortError" ? "no answer" : e.message}). Try again.`);
    return false;
  } finally {
    clearTimeout(timer);
  }
}

async function post(entity, action, param) {
  const [domain, name] = entity.split("/");
  const query = param == null ? "" : `?value=${encodeURIComponent(param)}`;
  return send(`/${domain}/${encodeURIComponent(name)}/${action}${query}`, { body: "" }, (response) => {
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    return true;
  });
}

// One press at a time: the pressed button stays off until its requests settle, so a second tap can't repeat them,
// and renderSchedule() keeps Create schedule off meanwhile. A render follows, so handlers needn't ask for one.
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

// --- QR code ---------------------------------------------------------------

// A short text, like the page's address, as a QR code in SVG, black on white with its quiet zone: version 3, 29
// modules a side, error correction M, the text's bytes, 42 at most, and the mask that scores best (ISO/IEC 18004).
function qrCode(text) {
  const bytes = new TextEncoder().encode(text);
  if (bytes.length > 42) return "";
  const size = 29;
  // 44 data codewords: byte mode, the length, the bytes and a terminator, then padding
  const bits = [];
  const put = (value, length) => {
    for (let i = length - 1; i >= 0; i--) bits.push((value >> i) & 1);
  };
  put(4, 4);
  put(bytes.length, 8);
  for (const byte of bytes) put(byte, 8);
  put(0, Math.min(4, 352 - bits.length));
  while (bits.length % 8) bits.push(0);
  const codewords = [];
  for (let i = 0; i < bits.length; i += 8) codewords.push(Number.parseInt(bits.slice(i, i + 8).join(""), 2));
  for (let pad = 0xec; codewords.length < 44; pad ^= 0xec ^ 0x11) codewords.push(pad);
  // and 26 Reed-Solomon ones over GF(256)
  const exp = [];
  const log = [];
  for (let i = 0, x = 1; i < 255; i++, x = x & 0x80 ? (x << 1) ^ 0x11d : x << 1) [exp[i], log[x]] = [x, i];
  const mul = (a, b) => (a && b ? exp[(log[a] + log[b]) % 255] : 0);
  let divisor = [1];
  for (let i = 0; i < 26; i++) divisor = [...divisor, 0].map((c, j) => c ^ mul(divisor[j - 1] ?? 0, exp[i]));
  const ecc = new Array(26).fill(0);
  for (const byte of codewords) {
    const factor = byte ^ ecc.shift();
    ecc.push(0);
    for (let i = 0; i < 26; i++) ecc[i] ^= mul(divisor[i + 1], factor);
  }
  codewords.push(...ecc);
  // the fixed patterns: the finders with their light borders, the alignment pattern, the timing lines, the dark
  // module, and the places of the format's two copies
  const modules = [...Array(size)].map(() => new Array(size).fill(false));
  const fixed = [...Array(size)].map(() => new Array(size).fill(false));
  const set = (r, c, dark) => {
    modules[r][c] = dark;
    fixed[r][c] = true;
  };
  for (const [top, left] of [
    [0, 0],
    [0, size - 7],
    [size - 7, 0],
  ])
    for (let r = -1; r <= 7; r++)
      for (let c = -1; c <= 7; c++) {
        const ring = Math.max(Math.abs(r - 3), Math.abs(c - 3));
        if (top + r >= 0 && top + r < size && left + c >= 0 && left + c < size)
          set(top + r, left + c, ring !== 2 && ring !== 4);
      }
  for (let r = -2; r <= 2; r++)
    for (let c = -2; c <= 2; c++) set(22 + r, 22 + c, Math.max(Math.abs(r), Math.abs(c)) !== 1);
  for (let i = 8; i < size - 8; i++) {
    set(6, i, i % 2 === 0);
    set(i, 6, i % 2 === 0);
  }
  set(size - 8, 8, true);
  const format = [
    [...[0, 1, 2, 3, 4, 5, 7, 8].map((r) => [r, 8]), ...[7, 5, 4, 3, 2, 1, 0].map((c) => [8, c])],
    [...[0, 1, 2, 3, 4, 5, 6, 7].map((i) => [8, size - 1 - i]), ...[6, 5, 4, 3, 2, 1, 0].map((i) => [size - 1 - i, 8])],
  ];
  for (const [r, c] of format.flat()) fixed[r][c] = true;
  // the codewords' bits, two columns at a time from the right, up and down in turn, round the fixed patterns
  let i = 0;
  for (let right = size - 1; right >= 1; right -= 2)
    for (let vert = 0; vert < size; vert++)
      for (const c of right <= 6 ? [right - 1, right - 2] : [right, right - 1]) {
        const r = (right <= 6 ? right : right + 1) & 2 ? vert : size - 1 - vert;
        if (fixed[r][c] || i >= codewords.length * 8) continue;
        modules[r][c] = ((codewords[i >> 3] >> (7 - (i & 7))) & 1) === 1;
        i++;
      }
  // each mask with its format bits, scored by runs, blocks, finder-like patterns and the share of dark modules
  const masks = [
    (r, c) => (r + c) % 2 === 0,
    (r) => r % 2 === 0,
    (_, c) => c % 3 === 0,
    (r, c) => (r + c) % 3 === 0,
    (r, c) => (Math.floor(r / 2) + Math.floor(c / 3)) % 2 === 0,
    (r, c) => ((r * c) % 2) + ((r * c) % 3) === 0,
    (r, c) => (((r * c) % 2) + ((r * c) % 3)) % 2 === 0,
    (r, c) => (((r + c) % 2) + ((r * c) % 3)) % 2 === 0,
  ];
  let best = null;
  let bestScore = Number.POSITIVE_INFINITY;
  for (const [mask, flips] of masks.entries()) {
    const grid = modules.map((row, r) => row.map((dark, c) => (fixed[r][c] ? dark : dark !== flips(r, c))));
    let rem = mask; // error correction M is 00
    for (let j = 0; j < 10; j++) rem = (rem << 1) ^ ((rem >> 9) * 0x537);
    const word = ((mask << 10) | rem) ^ 0x5412;
    for (const copy of format) for (const [j, [r, c]] of copy.entries()) grid[r][c] = ((word >> j) & 1) === 1;
    const lines = [...grid, ...grid.map((_, c) => grid.map((row) => row[c]))].map((line) => line.map(Number).join(""));
    let score = 0;
    for (const line of lines) {
      for (const run of line.match(/0{5,}|1{5,}/g) ?? []) score += run.length - 2;
      score += 40 * (line.match(/(?=10111010000|00001011101)/g) ?? []).length;
    }
    for (let r = 0; r < size - 1; r++)
      for (let c = 0; c < size - 1; c++) {
        const dark = grid[r][c] + grid[r][c + 1] + grid[r + 1][c] + grid[r + 1][c + 1];
        if (dark === 0 || dark === 4) score += 3;
      }
    score += 10 * Math.floor(Math.abs((grid.flat().filter(Boolean).length * 20) / (size * size) - 10));
    if (score < bestScore) [best, bestScore] = [grid, score];
  }
  const path = best.flatMap((row, r) => row.map((dark, c) => (dark ? `M${c + 4} ${r + 4}h1v1h-1z` : ""))).join("");
  return `<svg viewBox="0 0 ${size + 8} ${size + 8}" role="img" aria-label="QR code" shape-rendering="crispEdges"><path fill="#fff" d="M0 0h${size + 8}v${size + 8}H0z"/><path fill="#000" d="${path}"/></svg>`;
}

// --- Start -----------------------------------------------------------------

// A button that asks first, then presses the board's button.
function confirmPress(id, entity, message, question) {
  press($(id), async () => {
    if (!confirm(question)) return;
    if (await post(entity, "press")) toast(message);
  });
}

function bind() {
  $("qr").innerHTML = qrCode(location.origin);
  $("address").textContent = location.origin;
  $("limit-select").addEventListener("change", (e) => (draft.limit = Number(e.target.value)));
  $("ready-select").addEventListener("change", (e) => (draft.deadline = Number(e.target.value)));
  press($("create-schedule"), createSchedule);
  press($("charge-now"), chargeNow);
  press($("delete-schedule"), () => stopCharging("Schedule deleted"));
  press($("stop-charging"), () => stopCharging("Charging stopped"));
  confirmPress(
    "pair",
    E.pair,
    "Creating the key: tap your key card",
    "Create a new key? Sit in the car and tap your key card on the console when asked.",
  );
  confirmPress(
    "reset-savings",
    E.resetSavings,
    "Savings reset",
    "Reset the savings? They start again from zero today.",
  );
  // The VIN field takes only what a VIN can hold, as it's typed or pasted: capitals and digits, I, O and Q as the 1
  // and 0 they're taken for, as no VIN has them, and no more than 17. The caret stays where it was.
  $("set-vin").addEventListener("input", (e) => {
    const field = e.target;
    const vin = (text) =>
      text
        .toUpperCase()
        .replace(/[IOQ]/g, (c) => (c === "I" ? "1" : "0"))
        .replace(/[^A-Z0-9]/g, "");
    const caret = vin(field.value.slice(0, field.selectionStart)).length;
    field.value = vin(field.value).slice(0, 17);
    field.setSelectionRange(caret, caret);
    // the setup's guess of the battery, from the car's model, while the size is no one's own
    if (setup.step && !batteryTyped && field.value.length === 17) $("set-battery").value = batteryOf(field.value);
  });
  $("set-battery").addEventListener("input", () => (batteryTyped = true));
  for (const button of document.querySelectorAll(".info"))
    button.addEventListener("click", (e) => {
      e.preventDefault();
      button.setAttribute("aria-expanded", String(button.getAttribute("aria-expanded") !== "true"));
      requestRender();
    });
  // another country's plans, with its only one chosen
  $("set-area").addEventListener("change", () => {
    fillPlans();
    if (unfinished()) guessFromArea();
    requestRender();
  });
  $("set-price").addEventListener("change", requestRender);
  $("set-unlisted").addEventListener("change", (e) => {
    $("set-plan").value = e.target.checked ? "none" : "";
    requestRender();
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
  }
  for (const button of document.querySelectorAll(".step .save")) press(button, nextStep);
  // Update: the board downloads the release, then restarts with it.
  press($("update"), async () => {
    if (await post(E.firmware, "install")) {
      updating = true;
      restarting = Date.now();
    }
  });
  // Restart board, under Board: the page waits for the board, reconnecting soon rather than when the browser would.
  press($("restart"), async () => {
    if (!confirm("Restart the board?")) return;
    if (await post(E.restart, "press")) {
      restarting = Date.now();
      setTimeout(reconnect, 3000);
    }
  });
  $("change-settings").addEventListener("click", () => {
    settings.open = true;
    render(); // now, as a hidden card has nowhere to scroll to
    $("settings-card").scrollIntoView({ behavior: "smooth" });
  });
  $("cancel-settings").addEventListener("click", () => {
    settings.open = false;
    fillSettings(); // back to the board's
    requestRender();
  });
  // Upload settings, under the settings form: a settings file of the user's own.
  $("upload-settings").addEventListener("click", () => $("settings-file").click());
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
  document.title = "ESPHome Tesla BLE Scheduler";
  document.body.insertAdjacentHTML("afterbegin", PAGE);
  bind();
  connect();
}
