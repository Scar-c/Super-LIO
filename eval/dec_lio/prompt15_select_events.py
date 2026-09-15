#!/usr/bin/env python3
"""Freeze Prompt15 event choices using Prompt14 estimator-only diagnostics."""

import argparse
import csv
import pathlib
import sys


def read_rows(path):
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def as_float(row, key):
    return float(row[key])


def is_candidate(row, suffix_end, min_suffix, separation):
    try:
        if int(row["basis_contract_ok"]) != 1 or int(row["matched_valid"]) != 1:
            return False
        if int(row["weak_rank_R"]) + int(row["weak_rank_t"]) <= 0:
            return False
        if as_float(row, "weak_norm_l") <= 1.0e-6:
            return False
        if not (as_float(row, "weak_ratio") < 0.5):
            return False
        if int(row["valid_init"]) != 1 or int(row["valid_lidar"]) != 1:
            return False
        # Only a converged nonlinear shadow is eligible for an intervention.
        if int(row["nonlinear_valid"]) != 1 or row["nonlinear_reason"] != "STEP_EPSILON":
            return False
        if as_float(row, "nonlinear_final_cost") >= as_float(row, "cost_init"):
            return False
        delta = sum(as_float(row, f"nonlinear_delta_{i}") ** 2 for i in range(6)) ** 0.5
        if not (1.0e-6 < delta <= 1.0):
            return False
        timestamp = as_float(row, "timestamp")
        if timestamp > suffix_end - min_suffix:
            return False
        return True
    except (KeyError, ValueError, OverflowError):
        return False


def select(sequence, frame_csv, trajectory, limit, min_suffix, separation):
    rows = read_rows(frame_csv)
    trajectory_times = []
    with pathlib.Path(trajectory).open(encoding="utf-8") as stream:
        for line in stream:
            if line.strip() and not line.lstrip().startswith("#"):
                trajectory_times.append(float(line.split()[0]))
    if not trajectory_times:
        raise ValueError(f"{trajectory}: empty trajectory")
    suffix_end = max(trajectory_times)
    candidates = [row for row in rows
                  if is_candidate(row, suffix_end, min_suffix, separation)]
    # Score is fixed before GT is read: relative local objective decrease.
    candidates.sort(key=lambda row: (
        -((as_float(row, "cost_init") - as_float(row, "nonlinear_final_cost")) /
          max(abs(as_float(row, "cost_init")), 1.0e-12)),
        int(row["frame"])))
    selected = []
    for row in candidates:
        timestamp = as_float(row, "timestamp")
        if all(abs(timestamp - as_float(other, "timestamp")) >= separation
               for other in selected):
            selected.append(row)
        if len(selected) >= limit:
            break
    selected.sort(key=lambda row: (as_float(row, "timestamp"), int(row["frame"])))
    result = []
    for rank, row in enumerate(selected, 1):
        objective_reduction = as_float(row, "cost_init") - as_float(
            row, "nonlinear_final_cost")
        relative_reduction = objective_reduction / max(
            abs(as_float(row, "cost_init")), 1.0e-12)
        result.append({
            "sequence": sequence,
            "selection_rank": rank,
            "frame": int(row["frame"]),
            "timestamp": as_float(row, "timestamp"),
            "weak_rank_R": int(row["weak_rank_R"]),
            "weak_rank_t": int(row["weak_rank_t"]),
            "weak_ratio": as_float(row, "weak_ratio"),
            "weak_norm_l": as_float(row, "weak_norm_l"),
            "delta_l_norm": as_float(row, "delta_l_norm"),
            "delta_tight_norm": as_float(row, "delta_tight_norm"),
            "nonlinear_delta_norm": sum(as_float(row, f"nonlinear_delta_{i}") ** 2
                                         for i in range(6)) ** 0.5,
            "cost_init": as_float(row, "cost_init"),
            "nonlinear_final_cost": as_float(row, "nonlinear_final_cost"),
            "objective_reduction": objective_reduction,
            "relative_objective_reduction": relative_reduction,
            "nonlinear_reason": row["nonlinear_reason"],
            "native_suffix_end": suffix_end,
        })
    return len(rows), len(candidates), result


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--sequence", action="append", nargs=3,
                        metavar=("NAME", "FRAME_CSV", "TRAJECTORY"),
                        required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--limit", type=int, default=5)
    parser.add_argument("--min-suffix-sec", type=float, default=20.0)
    parser.add_argument("--separation-sec", type=float, default=10.0)
    args = parser.parse_args(argv)
    if args.limit < 1 or args.min_suffix_sec < 0.0 or args.separation_sec < 0.0:
        parser.error("limit and time parameters must be non-negative; limit > 0")
    pathlib.Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    output_fields = [
        "sequence", "selection_rank", "frame", "timestamp", "weak_rank_R",
        "weak_rank_t", "weak_ratio", "weak_norm_l", "delta_l_norm",
        "delta_tight_norm", "nonlinear_delta_norm", "cost_init",
        "nonlinear_final_cost", "objective_reduction",
        "relative_objective_reduction", "nonlinear_reason", "native_suffix_end",
    ]
    total_rows = total_candidates = 0
    selections = []
    try:
        for sequence, frame_csv, trajectory in args.sequence:
            rows, candidates, selected = select(
                sequence, frame_csv, trajectory, args.limit,
                args.min_suffix_sec, args.separation_sec)
            total_rows += rows
            total_candidates += candidates
            selections.extend(selected)
            print(f"{sequence}: rows={rows} eligible={candidates} selected={len(selected)}")
        with pathlib.Path(args.output).open("w", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(stream, fieldnames=output_fields, lineterminator="\n")
            writer.writeheader()
            writer.writerows(selections)
        print(f"selection_rows={len(selections)} total_rows={total_rows} "
              f"total_eligible={total_candidates} output={args.output}")
    except (OSError, ValueError, csv.Error) as error:
        print(f"EVENT_SELECTION_FAIL: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
