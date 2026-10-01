#!/usr/bin/env python3
"""Mutation testing of charging/charging.h, from the repository root (it takes about half an hour):

    python3 test/mutation_test.py path/to/ArduinoJson/src

universalmutator (run with uvx, from uv) writes copies of charging.h with one small change each. The
unit tests build against every copy that changes the code, and run with undefined behavior caught; a
failing run kills the mutant. A survivor is a change no test notices: a missing test, or code that
makes no difference. Copies that build the same program as charging.h or another copy count once.
"""

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
from pathlib import Path

SOURCE = Path("charging/charging.h")
CHECKS = [
    "-fsanitize=undefined",
    "-fno-sanitize-recover=all",
    "-D_GLIBCXX_ASSERTIONS",
    "-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE",
]


def code(text):
    """The text without comments and with whitespace collapsed."""
    literal_or_comment = r'("(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\')|//[^\n]*|/\*.*?\*/'
    return " ".join(re.sub(literal_or_comment, lambda m: m.group(1) or " ", text, flags=re.S).split())


def build(header, work, json_src):
    """The unit tests built against `header` in `work`, or None when they don't build. The paths in the
    program are the same in every `work`, so the same code builds the same program."""
    (work / "charging").mkdir(parents=True)
    (work / "charging" / "charging.h").write_text(header)
    test = Path("test/charging_test.cpp").resolve()
    command = ["c++", "-std=c++17", *CHECKS, "-I", ".", "-I", json_src, test, "-o", "charging_test"]
    built = subprocess.run(command, cwd=work, capture_output=True).returncode == 0
    return work / "charging_test" if built else None


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


def passes(binary):
    try:
        return subprocess.run([binary], capture_output=True, timeout=5).returncode == 0
    except subprocess.TimeoutExpired:
        return False


def main():
    json_src = Path(sys.argv[1]).resolve()
    original = SOURCE.read_text()
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        mutate = ["uvx", "--from", "universalmutator", "mutate", SOURCE, "cpp", "--noCheck", "--mutantDir", tmp]
        subprocess.run(mutate, check=True, capture_output=True)
        files = sorted(tmp.glob("charging.mutant.*.h"), key=lambda f: int(f.name.split(".")[2]))
        mutants = [m for m in (f.read_text() for f in files) if code(m) != code(original)]  # not just comments
        reference = build(original, tmp / "original", json_src)
        if reference is None or not passes(reference):
            sys.exit("The unit tests fail without mutants")
        same = digest(reference)
        tested = {same}
        lock = threading.Lock()

        def outcome(index):
            """The program's digest (None when it doesn't build), and whether its tests pass when it's the first
            mutant to build that program."""
            work = tmp / str(index)
            try:
                binary = build(mutants[index], work, json_src)
                if binary is None:
                    return None, None
                program = digest(binary)
                with lock:
                    if program in tested:
                        return program, None
                    tested.add(program)
                return program, passes(binary)
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
            number, before, after = change(original.splitlines(), mutants[index].splitlines())
            print(f"{SOURCE}:{number}: {before}  ->  {after}")
    survivors = sum(passed[program] for program in first)
    invalid = sum(program is None for program, _ in outcomes)
    unchanged = sum(program == same for program, _ in outcomes)
    print(
        f"{len(mutants)} mutants: {invalid} don't build, {unchanged} build the same program as {SOURCE.name}, "
        f"{len(mutants) - invalid - unchanged - len(first)} the same as another mutant. Of the other {len(first)}, "
        f"{len(first) - survivors} were killed and {survivors} survived: score {1 - survivors / max(1, len(first)):.1%}"
    )


if __name__ == "__main__":
    main()
