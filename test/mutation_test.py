#!/usr/bin/env python3
"""Mutation testing of charger.h, settings.h and the headers they include, from the repository root (it takes about
40 minutes):

    python3 test/mutation_test.py ArduinoJson/src

universalmutator (run with uvx, from uv) writes copies of each header with one small change each. The unit tests
build against every copy that changes the code, with the other headers as they are, and run with undefined behavior
and memory errors caught; a failing run kills the mutant. A survivor is a change no test notices: a missing test, or
code that makes no difference. Copies that build the same program as the headers or another copy count once.
The Mutation workflow runs it weekly with --at-least, the score to keep in percent, under which the run exits with an
error, and with CXX=clang++ for the compiler, c++ otherwise: the tests take minutes with GCC's AddressSanitizer and
seconds with clang's.
"""

import argparse
import concurrent.futures
import difflib
import hashlib
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path

SOURCES = [
    Path("scheduler") / f"{name}.h"
    for name in ("calendar", "charger", "market", "savings", "schedule", "settings", "tariff")
]
CHECKS = [
    "-fsanitize=address,undefined",
    "-fno-sanitize-recover=all",
    "-D_GLIBCXX_ASSERTIONS",
    "-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE",
]
MUTATOR = "universalmutator==1.14.1"  # pinned, so a new version's rules don't move the score
CXX = os.environ.get("CXX", "c++")


def code(text):
    """The text without comments and with whitespace collapsed."""
    literal_or_comment = r'("(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\')|//[^\n]*|/\*.*?\*/'
    return " ".join(re.sub(literal_or_comment, lambda m: m.group(1) or " ", text, flags=re.S).split())


def build(headers, work, json_src):
    """The unit tests built against `headers`, the text of each of SOURCES, in `work`, or None when they don't build.
    The paths in the program are the same in every `work`, so the same code builds the same program."""
    (work / "scheduler").mkdir(parents=True)
    for path, text in headers.items():
        (work / path).write_text(text)
    test = Path("test/scheduler_test.cpp").resolve()
    command = [CXX, "-std=c++17", *CHECKS, "-I", ".", "-I", json_src, test, "-o", "scheduler_test"]
    built = subprocess.run(command, cwd=work, capture_output=True).returncode == 0
    return work / "scheduler_test" if built else None


def change(original, mutant):
    """The line a mutant changes, as (number, before, after); a line it inserts counts as part of the one before."""
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, original, mutant, autojunk=False).get_opcodes():
        if tag != "equal":
            if tag == "insert":
                i1, j1 = i1 - 1, j1 - 1
            return (
                i1 + 1,
                " ".join(line.strip() for line in original[i1:i2]),
                " / ".join(m.strip() for m in mutant[j1:j2]),
            )


def digest(binary):
    return hashlib.sha256(binary.read_bytes()).hexdigest()


def passes(binary, limit=None):
    """Whether the tests pass, within `limit` seconds where there is one. They take a few seconds with the checks, more
    while the other builds run: a mutant fails only when it takes far longer, looping forever."""
    try:
        return subprocess.run([binary], capture_output=True, timeout=limit).returncode == 0
    except subprocess.TimeoutExpired:
        return False


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("json_src", type=Path, help="ArduinoJson's src folder")
    parser.add_argument("--at-least", type=float, default=0, metavar="SCORE", help="fail under this score, in percent")
    args = parser.parse_args()
    json_src = args.json_src.resolve()
    originals = {path: path.read_text() for path in SOURCES}
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        mutants = []  # (the header, its text with one change)
        for path in SOURCES:
            out = tmp / path.stem
            out.mkdir()
            mutate = ["uvx", "--no-build", "--from", MUTATOR, "mutate", path, "cpp", "--noCheck", "--mutantDir", out]
            subprocess.run(mutate, check=True, stdout=subprocess.DEVNULL)  # its errors and uv's show
            files = sorted(out.glob(f"{path.stem}.mutant.*.h"), key=lambda f: int(f.name.split(".")[2]))
            texts = (f.read_text() for f in files)
            mutants += [(path, m) for m in texts if code(m) != code(originals[path])]  # not just comments
        reference = build(originals, tmp / "original", json_src)
        started = time.monotonic()
        if reference is None or not passes(reference):
            sys.exit("The unit tests don't build, or fail, without mutants")
        limit = max(20, 5 * (time.monotonic() - started))  # the tests' own time here, before the pool loads the machine
        same = digest(reference)
        tested = {same}
        lock = threading.Lock()

        def outcome(index):
            """The program's digest (None when it doesn't build), and whether its tests pass when it's the first
            mutant to build that program."""
            work = tmp / str(index)
            path, text = mutants[index]
            try:
                binary = build({**originals, path: text}, work, json_src)
                if binary is None:
                    return None, None
                program = digest(binary)
                with lock:
                    if program in tested:
                        return program, None
                    tested.add(program)
                return program, passes(binary, limit)
            finally:
                shutil.rmtree(work, ignore_errors=True)

        with concurrent.futures.ThreadPoolExecutor(os.cpu_count()) as pool:
            outcomes = list(pool.map(outcome, range(len(mutants))))

    passed = {program: result for program, result in outcomes if result is not None}
    first = {}  # each new program's first mutant, to list
    for index, (program, _) in enumerate(outcomes):
        if program not in (None, same):
            first.setdefault(program, index)
    for program, index in first.items():
        if passed[program]:
            path, text = mutants[index]
            number, before, after = change(originals[path].splitlines(), text.splitlines())
            print(f"{path}:{number}: {before}  ->  {after}")
    survivors = sum(passed[program] for program in first)
    invalid = sum(program is None for program, _ in outcomes)
    unchanged = sum(program == same for program, _ in outcomes)
    score = 1 - survivors / max(1, len(first))
    print(
        f"{len(mutants)} mutants: {invalid} don't build, {unchanged} build the same program as the headers, "
        f"{len(mutants) - invalid - unchanged - len(first)} the same as another mutant. Of the other {len(first)}, "
        f"{len(first) - survivors} were killed and {survivors} survived: score {score:.1%}"
    )
    if score < args.at_least / 100:
        sys.exit(f"The score is under {args.at_least:g}%")


if __name__ == "__main__":
    main()
