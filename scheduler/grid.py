"""Checks the grid settings when you build: the grid: block of config.yaml and the price list it starts from (format
in docs/grid-fees.md), as grid.h reads them on the board. Writes config.yaml's out for the board.
"""

import re
from pathlib import Path

import esphome.config_validation as cv

CONF_CALENDAR = "calendar"
CONF_CLOCK = "clock"
CONF_CURRENCY = "currency"
CONF_EXCEPTIONS = "exceptions"
CONF_PRICELIST = "pricelist"
CONF_RATES = "rates"

# The price lists, which the build reads from this release and the board downloads from GitHub every day.
PRICE_LISTS = Path(__file__).resolve().parent.parent / "pricelists"

# The days from Sunday and the months from January, as in grid.h.
DAY_NAMES = ["sun", "mon", "tue", "wed", "thu", "fri", "sat"]
MONTH_NAMES = ["jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"]
# A day's rates are letters on the board, so a grid has 26 at most, and prices are below MAX_PRICE, as MAX_RATES and
# MAX_PRICE in grid.h.
MAX_RATES = 26
MAX_PRICE = 1e6
# Words YAML reads as true, false or nothing.
NOT_NAMES = ["on", "off", "yes", "no", "true", "false", "null"]


def _named(key, names):
    """The indexes `key` names among `names`: one name, or a range like fri-mon or nov-mar, which may wrap. None if
    it's neither."""
    first, dash, last = key.partition("-")
    last = last if dash else first
    if first not in names or last not in names:
        return None
    i, j = names.index(first), names.index(last)
    return [(i + k) % len(names) for k in range((j - i) % len(names) + 1)]


def _words(line):
    """A line's words, split at spaces as the board splits them."""
    return [word for word in line.split(" ") if word]


def _rate(value):
    """A rate's name: a word like night or p1."""
    if isinstance(value, bool) or value is None or str(value).lower() in NOT_NAMES:
        raise cv.Invalid(f"YAML reads {', '.join(NOT_NAMES)} as true, false or nothing, so they can't be rate names")
    if not isinstance(value, str) or not re.fullmatch(r"[^\W\d_][\w-]*", value):
        raise cv.Invalid(f'"{value}" isn\'t a rate name: a word like night or p1')
    return value


def _line(value):
    """A day's rates: the rate from midnight, then each time it changes, on a quarter-hour, and the rate from then."""
    words = _words(value) if isinstance(value, str) else [value]
    if len(words) % 2 == 0:
        raise cv.Invalid(f'"{value}" isn\'t rates and times by turns, like night 07:00 day 23:00 night')
    for rate in words[::2]:
        _rate(rate)
    minutes = 0
    for when in words[1::2]:
        m = re.fullmatch(r"([0-9]{2}):(00|15|30|45)", when)
        if not m or not minutes < int(m[1]) * 60 + int(m[2]) < 24 * 60:
            raise cv.Invalid(f"{when} isn't a later quarter-hour, like 07:00 or 22:15")
        minutes = int(m[1]) * 60 + int(m[2])
    return value


def _each_once(names, what, example):
    """A table keyed like mon-fri or nov-mar, naming each of `names` once."""

    def validate(config):
        seen = []
        for key in config:
            found = _named(key, names)
            if found is None:
                raise cv.Invalid(f'"{key}" isn\'t a {what} or a range like {example}', [key])
            if set(found) & set(seen):
                raise cv.Invalid(f"{key} has a {what} that's there already", [key])
            seen += found
        if len(seen) < len(names):
            raise cv.Invalid(f"needs every {what}, like {example}")
        return config

    return validate


def _date(value):
    """A date like 12-25. 02-29 passes: it exists in leap years."""
    value = cv.string_strict(value)
    m = re.fullmatch(r"([0-9]{2})-([0-9]{2})", value)
    if (
        not m
        or not 1 <= int(m[1]) <= 12
        or not 1 <= int(m[2]) <= (31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31)[int(m[1]) - 1]
    ):
        raise cv.Invalid(f'"{value}" isn\'t a date like 12-25')
    return value


def _keys(check, values):
    """A table whose keys pass `check`, with its message, and whose values pass `values`."""

    def validate(config):
        config = cv.Schema({cv.valid: values})(config)
        for key in config:
            with cv.prepend_path(key):
                check(key)
        return config

    return validate


# The weeks by month, and the tables both price lists and config.yaml have.
CALENDAR = cv.All(
    cv.Schema(
        {cv.string_strict: cv.All(cv.Schema({cv.string_strict: _line}), _each_once(DAY_NAMES, "day", "mon-fri"))}
    ),
    _each_once(MONTH_NAMES, "month", "jan-dec"),
)
TABLES = {
    cv.Optional(CONF_CLOCK): cv.one_of("local", "winter"),
    cv.Optional(CONF_EXCEPTIONS): _keys(_date, _line),
    cv.Optional(CONF_RATES): _keys(_rate, cv.float_range(min=0.0, max=MAX_PRICE, max_included=False)),
}
PRICE_LIST_SCHEMA = cv.Schema(
    {cv.Required(CONF_CALENDAR): CALENDAR, **TABLES, cv.Required(CONF_CURRENCY): cv.string_strict}
)


def _price_list_name(value):
    """A price list's name, like lt/eso-standartinis-4-zones."""
    value = cv.string_strict(value)
    if not re.fullmatch(r"[a-z]{2}/[a-z0-9-]+", value) or not (PRICE_LISTS / f"{value}.yaml").is_file():
        names = sorted(str(path.relative_to(PRICE_LISTS).with_suffix("")) for path in PRICE_LISTS.glob("*/*.yaml"))
        raise cv.Invalid(f"there's no price list {value}; there are {', '.join(names)}")
    return value


def _read(text):
    """Grid settings as the board reads them (read_grid() in grid.h), into tables with each key once."""
    if text and not text.endswith("\n"):
        raise cv.Invalid("ends inside a line, as if cut off")
    settings, section, week = {}, None, None
    for number, line in enumerate(text.split("\n"), 1):
        line = line.rstrip(" \r")
        body = line.lstrip(" ")
        indent = len(line) - len(body)
        key, colon, rest = body.partition(":")
        value = rest.lstrip(" ")
        table = None
        if not body or body.startswith("#"):
            continue
        if not colon or not key or rest[:1] not in ("", " "):
            pass
        elif indent == 0 and not value and key in (CONF_CALENDAR, CONF_EXCEPTIONS, CONF_RATES):
            table, section, value = settings, key, {}
        elif indent == 0 and value and key in (CONF_CLOCK, CONF_CURRENCY):
            table, section = settings, None
        elif indent == 2 and not value and section == CONF_CALENDAR:
            table, week, value = settings[section], key, {}
        elif indent == 4 and value and section == CONF_CALENDAR and week is not None:
            table = settings[section][week]
        elif indent == 2 and value and section in (CONF_EXCEPTIONS, CONF_RATES):
            table = settings[section]
        if table is None or key in table:
            raise cv.Invalid(f"line {number} isn't a key and a value where it can be, or its key is there already")
        table[key] = value
    return settings


def price_list(name):
    """A price list's settings, checked as the build checks config.yaml's."""
    try:
        return PRICE_LIST_SCHEMA(_read((PRICE_LISTS / f"{name}.yaml").read_text(encoding="utf-8")))
    except cv.Invalid as error:
        raise cv.Invalid(f"price list {name}: {error}") from error


def _with_price_list(config):
    """config.yaml's grid settings over the price list's, as make_grid() in grid.h puts them together: every rate the
    days use has a price, and every rate config.yaml sets is used."""
    settings = price_list(config[CONF_PRICELIST]) if CONF_PRICELIST in config else {}
    calendar = config.get(CONF_CALENDAR, settings.get(CONF_CALENDAR))
    if calendar is None:
        raise cv.Invalid("needs a pricelist, or a calendar of its own")
    exceptions = {**settings.get(CONF_EXCEPTIONS, {}), **config.get(CONF_EXCEPTIONS, {})}
    rates = {**settings.get(CONF_RATES, {}), **config.get(CONF_RATES, {})}
    lines = [*(line for week in calendar.values() for line in week.values()), *exceptions.values()]
    used = {rate for line in lines for rate in _words(line)[::2]}
    if missing := sorted(used - set(rates)):
        raise cv.Invalid(f"rate {missing[0]} needs a price per kWh, with VAT", [CONF_RATES])
    if unused := sorted(set(config.get(CONF_RATES, {})) - used):
        raise cv.Invalid(f"rate {unused[0]} isn't used on any day", [CONF_RATES, unused[0]])
    if len(rates) > MAX_RATES:
        raise cv.Invalid(f"has more than {MAX_RATES} rates, the price list's and yours together", [CONF_RATES])
    return config


GRID_SCHEMA = cv.All(
    cv.Schema({cv.Optional(CONF_CALENDAR): CALENDAR, **TABLES, cv.Optional(CONF_PRICELIST): _price_list_name}),
    _with_price_list,
)


def write_grid(grid):
    """config.yaml's grid settings as the board reads them (read_grid() in grid.h)."""
    lines = []
    if CONF_CALENDAR in grid:
        lines.append(f"{CONF_CALENDAR}:")
        for months, week in grid[CONF_CALENDAR].items():
            lines += [f"  {months}:", *(f"    {days}: {line}" for days, line in week.items())]
    if CONF_CLOCK in grid:
        lines.append(f"{CONF_CLOCK}: {grid[CONF_CLOCK]}")
    for table in (CONF_EXCEPTIONS, CONF_RATES):
        if table in grid:
            lines += [f"{table}:", *(f"  {key}: {value}" for key, value in grid[table].items())]
    return "".join(f"{line}\n" for line in lines)
