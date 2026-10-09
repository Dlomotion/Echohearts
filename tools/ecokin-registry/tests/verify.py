#!/usr/bin/env python3
"""Golden fixture and optional compiled-program parity tests."""
from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "tests" / "fixtures"
EXPECTED = ROOT / "tests" / "expected" / "reports.json"
EXPECTED_STATS = ROOT / "tests" / "expected" / "stats.json"
REPORT_KEYS = (
    "SCHEMA", "ROWS", "NUMBERED", "UNNUMBERED", "PENDING_REVIEW",
    "LEGACY_RETIRED", "PROPOSED_UNAPPLIED", "DUPLICATE_IDS", "INVALID_IDS",
    "DUPLICATE_NAMES", "INVALID_NAMES", "INVALID_ATTRIBUTES", "INVALID_STATES",
    "CSV_ERRORS", "SCHEMA_ERRORS", "ERRORS",
)
ERROR_KEYS = (
    "DUPLICATE_IDS", "INVALID_IDS", "DUPLICATE_NAMES", "INVALID_NAMES",
    "INVALID_ATTRIBUTES", "INVALID_STATES", "CSV_ERRORS", "SCHEMA_ERRORS",
)


def load_expected() -> dict[str, dict[str, int | str]]:
    reports = json.loads(EXPECTED.read_text(encoding="utf-8"))
    fixture_names = {path.name for path in FIXTURES.glob("*.csv")}
    if set(reports) != fixture_names:
        raise AssertionError("Golden reports and fixture CSVs do not match")
    for filename, report in reports.items():
        if tuple(report) != REPORT_KEYS:
            raise AssertionError(f"{filename}: report fields/order do not match contract")
        if report["SCHEMA"] != "ecokin-registry-v1":
            raise AssertionError(f"{filename}: incorrect report schema")
        if report["ERRORS"] != sum(report[key] for key in ERROR_KEYS):
            raise AssertionError(f"{filename}: inconsistent error total")
        if report["ROWS"] != report["NUMBERED"] + report["UNNUMBERED"]:
            raise AssertionError(f"{filename}: row identity counts do not reconcile")
    return reports


def parse_report(
    text: str, filename: str
) -> tuple[dict[str, int | str], list[str]]:
    report: dict[str, int | str] = {}
    stats_lines: list[str] = []
    for line in text.splitlines():
        if line.startswith(("ORIGINAL_STATS_", "PROPOSED_STATS_")):
            stats_lines.append(line)
            continue
        if "=" not in line:
            raise AssertionError(f"{filename}: malformed report line: {line!r}")
        key, value = line.split("=", 1)
        if key in report or key not in REPORT_KEYS:
            raise AssertionError(f"{filename}: unexpected or duplicate report key {key!r}")
        report[key] = value if key == "SCHEMA" else int(value)
    if tuple(report) != REPORT_KEYS:
        raise AssertionError(f"{filename}: report keys/order do not match contract")
    return report, stats_lines


def verify_program(
    program: Path,
    expected: dict[str, dict[str, int | str]],
    expected_stats: dict[str, list[str]],
) -> None:
    executable = program.resolve()
    if not executable.is_file():
        raise FileNotFoundError(f"Program not found: {executable}")
    for fixture_name, report in expected.items():
        result = subprocess.run(
            [str(executable), str((FIXTURES / fixture_name).resolve())],
            capture_output=True,
            text=True,
            check=False,
            timeout=20,
        )
        expected_code = 1 if report["ERRORS"] else 0
        if result.returncode != expected_code:
            raise AssertionError(
                f"{program}: {fixture_name}: exit={result.returncode}, "
                f"expected={expected_code}; stderr={result.stderr!r}"
            )
        actual, actual_stats = parse_report(result.stdout, fixture_name)
        if actual != report:
            raise AssertionError(f"{program}: {fixture_name}: {actual!r} != {report!r}")
        if actual_stats != expected_stats[fixture_name]:
            raise AssertionError(
                f"{program}: {fixture_name}: stat columns {actual_stats!r} "
                f"!= {expected_stats[fixture_name]!r}"
            )

    usage = subprocess.run(
        [str(executable)], capture_output=True, text=True, check=False, timeout=20
    )
    if usage.returncode != 2:
        raise AssertionError(f"{program}: no-argument invocation must return 2")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--program",
        action="append",
        type=Path,
        default=[],
        help="Compiled CLI to compare with the shared golden reports; repeat as needed",
    )
    args = parser.parse_args()
    expected = load_expected()
    expected_stats = json.loads(EXPECTED_STATS.read_text(encoding="utf-8"))
    if set(expected_stats) != set(expected):
        raise AssertionError("Stat reports and fixture CSVs do not match")

    if not args.program:
        print(
            "PASS: shared golden fixture contract only; "
            "COBOL/FreeBASIC compilation NOT VERIFIED"
        )
        return 0

    for program in args.program:
        verify_program(program, expected, expected_stats)
    print(f"PASS: {len(args.program)} program(s) match all {len(expected)} golden fixtures")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
