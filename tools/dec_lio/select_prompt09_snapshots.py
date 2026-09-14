#!/usr/bin/env python3
"""Select a deterministic, stratified Prompt09 runtime snapshot set."""

import argparse
import csv
import hashlib
import math
import os
import shutil
from collections import OrderedDict


SCENES = {
    "bridge01": "bridge01_shadow_v2",
    "stairs_alpha": "stairs_alpha_shadow_v2",
    "tunnel2": "tunnel2_alpha_shadow_v2",
}


def finite(value):
    try:
        return math.isfinite(float(value))
    except (TypeError, ValueError):
        return False


def numeric(row, key, default=float("nan")):
    try:
        value = float(row[key])
        return value if math.isfinite(value) else default
    except (KeyError, TypeError, ValueError):
        return default


def nearest(rows, key, quantile):
    values = sorted((numeric(row, key), row) for row in rows if finite(numeric(row, key)))
    if not values:
        return None
    index = min(len(values) - 1, max(0, int(math.ceil(quantile * len(values))) - 1))
    target = values[index][0]
    return min(values, key=lambda pair: abs(pair[0] - target))[1]


def add(candidate, row, reason):
    if row is None:
        return
    index = int(row["snapshot_index"])
    candidate.setdefault(index, []).append(reason)


def select_scene(rows, quota):
    rows = sorted(rows, key=lambda row: int(row["snapshot_index"]))
    candidate = OrderedDict()
    for key, label, quantiles in (
        ("A_condition", "fused_condition", (0.50, 0.90, 0.95, 0.99, 1.00)),
        ("kappa_R", "dcreg_kappa_R", (0.50, 0.95, 0.99, 1.00)),
        ("kappa_t", "dcreg_kappa_t", (0.50, 0.95, 0.99, 1.00)),
    ):
        for quantile in quantiles:
            add(candidate, nearest(rows, key, quantile), f"{label}_P{int(quantile * 100):02d}")

    rank_rows = {}
    for row in rows:
        rank = (int(row["weak_rank_R"]), int(row["weak_rank_t"]))
        rank_rows.setdefault(rank, row)
    for rank, row in sorted(rank_rows.items()):
        add(candidate, row, f"weak_rank_{rank[0]}_{rank[1]}")

    previous = None
    for row in rows:
        rank = (int(row["weak_rank_R"]), int(row["weak_rank_t"]))
        if previous is not None and rank != previous:
            add(candidate, row, "weak_rank_change")
        previous = rank

    # Keep the stress candidates first, then fill with evenly spaced temporal
    # coverage.  This makes the selected set stable across repeated runs.
    selected = OrderedDict()
    for index, reasons in candidate.items():
        if len(selected) >= quota:
            break
        selected[index] = reasons
    if len(selected) < quota:
        for position in range(quota):
            row = rows[min(len(rows) - 1, round(position * (len(rows) - 1) / max(quota - 1, 1)))]
            index = int(row["snapshot_index"])
            selected.setdefault(index, []).append("temporal_uniform")
            if len(selected) >= quota:
                break
    return selected


def read_rows(csv_path):
    with open(csv_path, newline="") as stream:
        return list(csv.DictReader(stream))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--runtime-root", default="/home/lc/dec_lio/runtime/prompt09")
    parser.add_argument("--out", default="/home/lc/dec_lio/runtime/prompt09/system_snapshots")
    parser.add_argument("--quota-per-scene", type=int, default=64)
    args = parser.parse_args()

    os.makedirs(args.out, exist_ok=True)
    selected_records = []
    source_record_cache = {}
    for scene, run_id in SCENES.items():
        run_dir = os.path.join(args.runtime_root, "shadow", run_id)
        csv_path = os.path.join(run_dir, "d3_solver_shadow.csv")
        binary_path = os.path.join(run_dir, "d3_solver_snapshots.bin")
        rows = read_rows(csv_path)
        selected = select_scene(rows, args.quota_per_scene)
        for index, reasons in selected.items():
            row = next(item for item in rows if int(item["snapshot_index"]) == index)
            selected_records.append((scene, binary_path, index, row, ";".join(reasons)))
        source_record_cache[binary_path] = None

    output_binary = os.path.join(args.out, "selected_snapshots.bin")
    manifest_path = os.path.join(args.out, "selection_manifest.csv")
    with open(output_binary, "wb") as output:
        header_written = False
        for scene, binary_path, index, row, reason in selected_records:
            if source_record_cache[binary_path] is None:
                with open(binary_path, "rb") as source:
                    data = source.read()
                if data[:8] != b"D3SNAP01" or int.from_bytes(data[8:12], "little") != 2:
                    raise RuntimeError(f"unexpected snapshot header: {binary_path}")
                count = sum(1 for item in read_rows(os.path.join(os.path.dirname(binary_path), "d3_solver_shadow.csv")))
                payload_size = len(data) - 12
                if payload_size % count:
                    raise RuntimeError(f"non-integral record size: {binary_path}")
                source_record_cache[binary_path] = (data[:12], payload_size // count, data)
            header, record_size, data = source_record_cache[binary_path]
            if not header_written:
                output.write(header)
                header_written = True
            offset = 12 + index * record_size
            output.write(data[offset : offset + record_size])

    with open(manifest_path, "w", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(
            [
                "selected_rank",
                "scene",
                "source_snapshot_path",
                "source_snapshot_index",
                "frame",
                "ieskf_iteration",
                "timestamp",
                "N_used",
                "A_condition",
                "kappa_R",
                "kappa_t",
                "weak_rank_R",
                "weak_rank_t",
                "selection_reason",
            ]
        )
        for rank, (scene, binary_path, index, row, reason) in enumerate(selected_records):
            writer.writerow(
                [
                    rank,
                    scene,
                    binary_path,
                    index,
                    row["frame"],
                    row["ieskf_iteration"],
                    row["timestamp"],
                    row["N_used"],
                    row["A_condition"],
                    row["kappa_R"],
                    row["kappa_t"],
                    row["weak_rank_R"],
                    row["weak_rank_t"],
                    reason,
                ]
            )

    sha = hashlib.sha256()
    with open(output_binary, "rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            sha.update(chunk)
    print(f"selected={len(selected_records)} binary={output_binary}")
    print(f"binary_sha256={sha.hexdigest()}")
    print(f"manifest={manifest_path}")


if __name__ == "__main__":
    main()
