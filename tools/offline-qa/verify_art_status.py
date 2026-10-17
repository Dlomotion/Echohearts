#!/usr/bin/env python3
"""Standalone synthetic fixture test for COBOL / FreeBASIC art-status counters.

Without --program this verifies ONLY fixture expectations, not either compiler.
With --program it runs the actual compiled program and checks stdout/exit codes.
No approved Eco-Kin IDs or real artwork are represented in the sample fixtures.
"""
from __future__ import annotations

import argparse
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent
LABELS = ("ACTIVE_CANON_ART", "PENDING_CANON_REVIEW",
          "REFERENCE_RETIRED_NEEDS_REDESIGN")
VALID = ROOT / "fixtures" / "art-status-valid.txt"
INVALID = ROOT / "fixtures" / "art-status-invalid.txt"
VALID_EXPECTED = {"ACTIVE_CANON_ART": 2, "PENDING_CANON_REVIEW": 1,
                  "REFERENCE_RETIRED_NEEDS_REDESIGN": 1, "INVALID": 0}
INVALID_EXPECTED = {"ACTIVE_CANON_ART": 1, "PENDING_CANON_REVIEW": 1,
                    "REFERENCE_RETIRED_NEEDS_REDESIGN": 0, "INVALID": 1}


def reference_counts(filename: Path) -> dict[str, int]:
    counts = {name: 0 for name in LABELS}
    counts["INVALID"] = 0
    for line in filename.read_text(encoding="utf-8").splitlines():
        if line in LABELS:
            counts[line] += 1
        else:
            counts["INVALID"] += 1
    return counts


def parse_program_report(text: str) -> dict[str, int]:
    result: dict[str, int] = {}
    for line in text.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        if key not in (*LABELS, "INVALID") or key in result:
            raise ValueError(f"Unexpected/duplicate report key: {key!r}")
        if not value.strip().isdigit():
            raise ValueError(f"Non-numeric art-status count: {line!r}")
        result[key] = int(value.strip())
    if set(result) != {*LABELS, "INVALID"}:
        raise ValueError(f"Missing/extra report fields: {set(result)}")
    return result


def verify_program(executable: Path) -> None:
    if not executable.is_file():
        raise FileNotFoundError(f"Compiled binary not found: {executable}")
    for fixture, expected, expected_code in (
        (VALID, VALID_EXPECTED, 0),
        (INVALID, INVALID_EXPECTED, 1),
    ):
        process = subprocess.run(
            [str(executable.resolve()), str(fixture.resolve())],
            capture_output=True, text=True, check=False, timeout=20
        )
        if process.returncode != expected_code:
            raise AssertionError(
                f"{fixture.name}: exit {process.returncode}, expected {expected_code}. "
                f"output={process.stdout!r} stderr={process.stderr!r}"
            )
        actual = parse_program_report(process.stdout)
        if actual != expected:
            raise AssertionError(f"{fixture.name}: {actual!r} != {expected!r}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--program", type=Path,
        help="Optional compiled COBOL or FreeBASIC program to execute and verify"
    )
    args = parser.parse_args()
    if reference_counts(VALID) != VALID_EXPECTED:
        raise AssertionError("Valid synthetic fixture mismatch")
    if reference_counts(INVALID) != INVALID_EXPECTED:
        raise AssertionError("Invalid synthetic fixture mismatch")
    if args.program:
        verify_program(args.program)
        print("PASS: compiled standalone program handles both synthetic fixtures")
    else:
        print("PASS: synthetic fixture contract only; COBOL/BASIC compilation NOT VERIFIED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
