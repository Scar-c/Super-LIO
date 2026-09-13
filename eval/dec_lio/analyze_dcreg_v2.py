#!/usr/bin/env python3
"""Summarize Prompt02 D1 schema-v2 shadow output.

This module intentionally treats the raw-EVD projector and used-residual
counter as the authoritative diagnostics. The legacy axis-alignment columns
are retained for comparison but are not used for weak-subspace stability.
"""

import argparse
import csv
import json
import math
import pathlib
import statistics
import sys


MODES = ("rot", "trans")


def number(row, name):
    try:
        return float(row.get(name, ""))
    except (TypeError, ValueError):
        return math.nan


def finite(values):
    return [value for value in values if math.isfinite(value)]


def percentile(values, fraction):
    values = sorted(finite(values))
    if not values:
        return None
    if len(values) == 1:
        return values[0]
    position = (len(values) - 1) * fraction
    lower = math.floor(position)
    upper = math.ceil(position)
    if lower == upper:
        return values[lower]
    weight = position - lower
    return values[lower] * (1.0 - weight) + values[upper] * weight


def stats(values):
    values = finite(values)
    if not values:
        return {"count": 0, "median": None, "p95": None, "max": None}
    return {
        "count": len(values),
        "median": statistics.median(values),
        "p95": percentile(values, 0.95),
        "max": max(values),
    }


def rows_from_csv(path):
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        if reader.fieldnames is None:
            raise ValueError(f"{path}: missing CSV header")
        if "schema_version" not in reader.fieldnames:
            raise ValueError(f"{path}: missing schema_version")
        rows = list(reader)
    versions = {int(float(row["schema_version"])) for row in rows if row.get("frame")}
    if versions != {2}:
        raise ValueError(f"{path}: expected schema_version=2, got {sorted(versions)}")
    return [row for row in rows if row.get("frame")]


def weak_rank_distribution(rows, mode):
    values = [int(float(row[f"weak_rank_{mode}"])) for row in rows
              if row.get("valid") == "1"]
    result = {}
    for value in sorted(set(values)):
        result[str(value)] = values.count(value)
    return result


def longest_run(rows, predicate):
    longest = {"frames": 0, "duration_s": 0.0, "start_time": None, "end_time": None}
    current = []
    for row in rows:
        if predicate(row):
            current.append(row)
        elif current:
            candidate = _run_record(current)
            if candidate["frames"] > longest["frames"]:
                longest = candidate
            current = []
    if current:
        candidate = _run_record(current)
        if candidate["frames"] > longest["frames"]:
            longest = candidate
    return longest


def _run_record(rows):
    start = number(rows[0], "timestamp")
    end = number(rows[-1], "timestamp")
    return {
        "frames": len(rows),
        "duration_s": end - start if math.isfinite(start) and math.isfinite(end) else None,
        "start_time": start,
        "end_time": end,
    }


def gamma_study(rows, mode, reference):
    valid = [row for row in rows if row.get("valid") == "1"]
    gamma_by_mode = [[] for _ in range(3)]
    for row in valid:
        for index in range(3):
            normalized = number(row, f"normalized_lambda_{mode}_{index}")
            if not math.isfinite(normalized) or normalized <= 0.0:
                gamma = 0.0
            else:
                kappa = 1.0 / normalized
                gamma = min(1.0, math.sqrt(reference / kappa))
            gamma_by_mode[index].append(gamma)
    mode_result = {}
    for index, values in enumerate(gamma_by_mode):
        mode_result[f"mode_{index}"] = {
            "fraction_gamma_lt_1": sum(value < 1.0 for value in values) / len(values)
            if values else 0.0,
            "fraction_gamma_lt_075": sum(value < 0.75 for value in values) / len(values)
            if values else 0.0,
            "fraction_gamma_lt_05": sum(value < 0.5 for value in values) / len(values)
            if values else 0.0,
            "fraction_gamma_lt_025": sum(value < 0.25 for value in values) / len(values)
            if values else 0.0,
            "median": statistics.median(values) if values else None,
            "p10": percentile(values, 0.10),
            "minimum": min(values) if values else None,
            "longest_continuous_attenuation": longest_run(
                valid,
                lambda row, i=index: _gamma_for(row, mode, i, reference) < 1.0,
            ),
        }
    return mode_result


def _gamma_for(row, mode, index, reference):
    normalized = number(row, f"normalized_lambda_{mode}_{index}")
    if not math.isfinite(normalized) or normalized <= 0.0:
        return 0.0
    return min(1.0, math.sqrt(reference * normalized))


def summarize(path, references=(3.0, 5.0, 10.0, 20.0)):
    rows = rows_from_csv(path)
    valid = [row for row in rows if row.get("valid") == "1"]
    result = {
        "input": str(pathlib.Path(path).resolve()),
        "schema_version": 2,
        "total_rows": len(rows),
        "valid_rows": len(valid),
        "invalid_rows": len(rows) - len(valid),
        "factorization_failures": sum(row.get("factorization_ok") != "1" for row in rows),
        "candidate_count": stats([number(row, "candidate_count") for row in valid]),
        "used_residual_count": stats([number(row, "used_residual_count") for row in valid]),
        "used_residual_ratio": stats([number(row, "used_residual_ratio") for row in valid]),
        "condition_rot": stats([number(row, "cond_rot") for row in valid]),
        "condition_trans": stats([number(row, "cond_trans") for row in valid]),
        "weak_rank_distribution": {
            mode: weak_rank_distribution(valid, mode) for mode in MODES
        },
        "rank_change_rate": {
            mode: sum(row.get(f"rank_changed_{mode}") == "1" for row in valid) /
            len(valid) if valid else 0.0 for mode in MODES
        },
        "projector_distance": {
            mode: stats([number(row, f"projector_distance_{mode}") for row in valid])
            for mode in MODES
        },
        "principal_angle_max": {
            mode: stats([number(row, f"principal_angle_max_{mode}") for row in valid])
            for mode in MODES
        },
        "principal_angle_mean": {
            mode: stats([number(row, f"principal_angle_mean_{mode}") for row in valid])
            for mode in MODES
        },
        "eigenbasis_gauge_events": {
            mode: sum(
                number(row, f"projector_distance_{mode}") < 1e-8 and
                number(row, f"principal_angle_max_{mode}") > 1e-6
                for row in valid
            ) for mode in MODES
        },
        "gamma_study": {
            str(int(reference)): {
                mode: gamma_study(valid, mode, reference) for mode in MODES
            } for reference in references
        },
    }
    return result


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("raw_csv", type=pathlib.Path)
    parser.add_argument("--out", type=pathlib.Path)
    args = parser.parse_args(argv)
    try:
        report = json.dumps(summarize(args.raw_csv), indent=2, sort_keys=True) + "\n"
    except (OSError, ValueError, KeyError) as error:
        print(f"D1_ANALYSIS_FAIL: {error}", file=sys.stderr)
        return 2
    if args.out:
        args.out.write_text(report, encoding="utf-8")
    else:
        sys.stdout.write(report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
