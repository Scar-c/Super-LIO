#!/usr/bin/env python3
"""Replay the frozen Prompt23 G0/G1 gate over Prompt22 L1 diagnostics."""

import argparse
import csv
import math
from collections import deque
from pathlib import Path


def finite(value):
    try:
        value = float(value)
    except (TypeError, ValueError):
        return False
    return math.isfinite(value)


def median(values):
    if not values:
        return float("nan")
    values = sorted(values)
    middle = len(values) // 2
    if len(values) % 2:
        return values[middle]
    return 0.5 * (values[middle - 1] + values[middle])


def replay(path, gate_name):
    samples = deque(maxlen=20)
    loose = False
    rows = []
    switches = []
    with path.open(newline="") as stream:
        for row in csv.DictReader(stream):
            registration_success = row.get("registration_success") == "1"
            weak = registration_success and any(
                row.get(column) == "1"
                for column in ("dcreg_rot_weak_x", "dcreg_rot_weak_y", "dcreg_rot_weak_z")
            )
            q = row.get("q_weak", "nan")
            q = float(q) if finite(q) else float("nan")
            multiplier = row.get("weak_rot_multiplier", "nan")
            multiplier = float(multiplier) if finite(multiplier) else float("nan")
            before = loose
            if registration_success:
                samples.append((weak, q))
            weak_fraction = (
                sum(sample[0] for sample in samples) / len(samples)
                if samples else 0.0
            )
            q_values = [sample[1] for sample in samples if sample[0] and finite(sample[1])]
            q_median = median(q_values)
            if len(samples) < 10:
                loose = False
            elif gate_name == "g0":
                if not loose and weak_fraction >= 0.75:
                    loose = True
                elif loose and weak_fraction <= 0.60:
                    loose = False
            else:
                enter = weak_fraction >= 0.75 and finite(q_median) and q_median >= 0.30
                exit_ = weak_fraction <= 0.60 or (finite(q_median) and q_median <= 0.20)
                if not loose and enter:
                    loose = True
                elif loose and exit_:
                    loose = False
            changed = before != loose
            if changed:
                switches.append(int(row["frame"]))
            rows.append(
                {
                    "sequence_frame": row["frame"],
                    "timestamp": row["timestamp"],
                    "registration_success": int(registration_success),
                    "weak_flag": int(weak),
                    "weak_multiplier": multiplier,
                    "q_weak": q,
                    "rolling_weak_fraction": weak_fraction,
                    "rolling_q_median": q_median,
                    "rolling_successful_frames": len(samples),
                    "rolling_q_frames": len(q_values),
                    "gate_candidate": gate_name,
                    "state_before": "Loose" if before else "Native",
                    "state_after": "Loose" if loose else "Native",
                    "selected_estimator": "Loose_L1" if registration_success and loose else "Native",
                    "mode_switch": int(changed),
                }
            )
    loose_count = sum(row["selected_estimator"] == "Loose_L1" for row in rows)
    return rows, {
        "rows": len(rows),
        "successful_registration_rows": sum(row["registration_success"] for row in rows),
        "loose_rows": loose_count,
        "loose_duty_percent": 100.0 * loose_count / len(rows) if rows else 0.0,
        "switches": len(switches),
        "switch_frames": ";".join(str(frame) for frame in switches),
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--stairs", type=Path, required=True)
    parser.add_argument("--alpha", type=Path, required=True)
    parser.add_argument("--gamma", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    inputs = {"Stairs": args.stairs, "Alpha": args.alpha, "Gamma": args.gamma}
    summary_rows = []
    for sequence, path in inputs.items():
        for gate_name in ("g0", "g1"):
            rows, summary = replay(path, gate_name)
            output = args.out / f"{sequence.lower()}_{gate_name}.csv"
            with output.open("w", newline="") as stream:
                writer = csv.DictWriter(stream, fieldnames=list(rows[0].keys())) if rows else None
                if writer:
                    writer.writeheader()
                    writer.writerows(rows)
            summary_rows.append({"sequence": sequence, "gate": gate_name, **summary})
    with (args.out / "summary.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(summary_rows[0].keys()))
        writer.writeheader()
        writer.writerows(summary_rows)


if __name__ == "__main__":
    main()
