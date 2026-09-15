#!/usr/bin/env python3
"""Summarize Prompt14 matched and nonlinear LiDAR-only shadow runs.

The first Prompt14 runs were made with a binary built before the CSV schema
field was added.  Their data rows therefore have 85 values for an 86-column
schema.  This reader detects that exact condition and restores the constant
schema value; it does not reinterpret or drop any measurement value.
"""

import argparse
import csv
import math
import pathlib
import statistics
from collections import Counter, defaultdict

import numpy as np


FRAME_FIELDS = """schema_version,frame,timestamp,basis_contract_ok,p1_active,matched_valid,matched_points,matched_rank,matched_pinv_threshold,delta_l_0,delta_l_1,delta_l_2,delta_l_3,delta_l_4,delta_l_5,delta_tight_0,delta_tight_1,delta_tight_2,delta_tight_3,delta_tight_4,delta_tight_5,delta_diff_0,delta_diff_1,delta_diff_2,delta_diff_3,delta_diff_4,delta_diff_5,delta_l_norm,delta_tight_norm,delta_diff_norm,angle_deg,cond_R,cond_t,weak_rank_R,weak_rank_t,weak_norm_l,weak_norm_tight,strong_norm_l,strong_norm_tight,weak_ratio,strong_ratio,raw_effective_h_diff,raw_effective_b_diff,cost_init,valid_init,cost_lidar,valid_lidar,cost_tight,valid_tight,mean_abs_init,mean_abs_lidar,mean_abs_tight,rmse_init,rmse_lidar,rmse_tight,nonlinear_valid,nonlinear_iterations,nonlinear_reason,nonlinear_delta_0,nonlinear_delta_1,nonlinear_delta_2,nonlinear_delta_3,nonlinear_delta_4,nonlinear_delta_5,nonlinear_final_cost,nonlinear_final_valid,nonlinear_final_points,nonlinear_final_mean_abs,nonlinear_final_rmse,nonlinear_final_rank,nonlinear_final_pinv_threshold,t_init_x,t_init_y,t_init_z,t_init_qx,t_init_qy,t_init_qz,t_init_qw,t_shadow_x,t_shadow_y,t_shadow_z,t_shadow_qx,t_shadow_qy,t_shadow_qz,t_shadow_qw,nonlinear_cost_trajectory""".split(",")


SEQUENCES = {
    "bridge01": {
        "sensor_config": "VELO16 / geode_alpha.yaml",
        "run": "bridge01",
        "gt": "/home/lc/dec_lio/bag/GEODE/bridge01.txt",
    },
    "stairs_alpha": {
        "sensor_config": "VELO16 / geode_stairs_alpha.yaml",
        "run": "stairs_alpha",
        "gt": "/home/lc/dec_lio/bag/GEODE/stairs_alpha.txt",
    },
    "tunnel1_gamma": {
        "sensor_config": "LIVOX6 / geode_gamma.yaml",
        "run": "tunnel1_gamma",
        "gt": "/home/lc/dec_lio/bag/GEODE/Tunneling_tunnel1.txt",
    },
    "tunnel2_alpha": {
        "sensor_config": "VELO16 / geode_tunneling2_alpha.yaml",
        "run": "tunnel2_alpha",
        "gt": "/home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt",
    },
    "tunnel2_gamma": {
        "sensor_config": "LIVOX6 / geode_gamma.yaml",
        "run": "tunnel2_gamma",
        "gt": "/home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt",
    },
}

EVENT_FIELDS = [
    "sequence", "event_id", "start_timestamp", "end_timestamp", "duration_s",
    "frames", "dominant_weak_mode", "median_c_L_norm", "median_c_tight_norm",
    "median_suppression", "median_cost_improvement", "nonlinear_valid_fraction",
    "native_local_behavior",
]


def number(row, key, default=float("nan")):
    try:
        return float(row[key])
    except (KeyError, TypeError, ValueError):
        return default


def integer(row, key, default=0):
    value = number(row, key, float(default))
    return int(value) if math.isfinite(value) else default


def finite(value):
    return math.isfinite(value)


def values(rows, key, predicate=None):
    result = []
    for row in rows:
        value = number(row, key)
        if finite(value) and (predicate is None or predicate(row)):
            result.append(value)
    return result


def percentile(items, quantile):
    return float(np.percentile(np.asarray(items, dtype=float), quantile * 100.0)) if items else float("nan")


def median(items):
    return float(statistics.median(items)) if items else float("nan")


def fraction(items, predicate):
    return sum(bool(predicate(item)) for item in items) / len(items) if items else float("nan")


def load_frame_csv(path):
    repaired = 0
    rows = []
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        reader = csv.reader(stream)
        next(reader)
        for row in reader:
            if not row:
                continue
            if len(row) == len(FRAME_FIELDS) - 1:
                row = ["1"] + row
                repaired += 1
            if len(row) != len(FRAME_FIELDS):
                raise ValueError(f"{path}: row has {len(row)} fields, expected {len(FRAME_FIELDS)}")
            rows.append(dict(zip(FRAME_FIELDS, row)))
    return rows, repaired


def load_modes(path):
    with pathlib.Path(path).open(newline="", encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def load_tum(path):
    rows = []
    with pathlib.Path(path).open(encoding="utf-8") as stream:
        for line in stream:
            fields = line.split()
            if not fields or fields[0].startswith("#"):
                continue
            if len(fields) < 8:
                continue
            rows.append((float(fields[0]), np.asarray([float(x) for x in fields[1:4]])))
    if not rows:
        raise ValueError(f"{path}: no TUM rows")
    return np.asarray([x[0] for x in rows]), np.asarray([x[1] for x in rows])


def associate(est_times, gt_times, max_diff=0.05):
    candidates = []
    for ei, stamp in enumerate(est_times):
        lo = int(np.searchsorted(gt_times, stamp - max_diff, side="left"))
        hi = int(np.searchsorted(gt_times, stamp + max_diff, side="right"))
        candidates.extend((abs(float(stamp - gt_times[gi])), ei, gi) for gi in range(lo, hi))
    used_est, used_gt, pairs = set(), set(), []
    for delta, ei, gi in sorted(candidates):
        if ei in used_est or gi in used_gt:
            continue
        used_est.add(ei)
        used_gt.add(gi)
        pairs.append((ei, gi, delta))
    return sorted(pairs)


def umeyama(source, destination):
    source_mean = source.mean(axis=0)
    destination_mean = destination.mean(axis=0)
    covariance = (source - source_mean).T @ (destination - destination_mean) / len(source)
    u, _, vt = np.linalg.svd(covariance)
    correction = np.diag([1.0, 1.0, np.sign(np.linalg.det(vt.T @ u.T))])
    rotation = vt.T @ correction @ u.T
    translation = destination_mean - rotation @ source_mean
    return rotation, translation


def trajectory_metrics(estimate_path, gt_path):
    et, ep = load_tum(estimate_path)
    gt, gp = load_tum(gt_path)
    gt_order = np.argsort(gt, kind="stable")
    gt, gp = gt[gt_order], gp[gt_order]
    pairs = associate(et, gt)
    result = {"gt_valid": 0, "gt_matches": len(pairs), "ape_rmse_m": float("nan"),
              "ape_median_m": float("nan"), "ape_p95_m": float("nan")}
    if len(pairs) < 3:
        return result
    source = np.asarray([ep[ei] for ei, _, _ in pairs])
    destination = np.asarray([gp[gi] for _, gi, _ in pairs])
    rotation, translation = umeyama(source, destination)
    errors = np.linalg.norm((rotation @ source.T).T + translation - destination, axis=1)
    result.update(gt_valid=1, ape_rmse_m=float(np.sqrt(np.mean(errors ** 2))),
                  ape_median_m=float(np.median(errors)), ape_p95_m=float(np.percentile(errors, 95)))

    matched_times = np.asarray([et[ei] for ei, _, _ in pairs])
    matched_est = source
    matched_gt = destination
    for dt in (1.0, 5.0, 10.0):
        errors_rpe = []
        for index, stamp in enumerate(matched_times):
            target = stamp + dt
            position = int(np.searchsorted(matched_times, target))
            candidates = [x for x in (position - 1, position) if 0 <= x < len(matched_times)]
            if not candidates:
                continue
            other = min(candidates, key=lambda x: abs(float(matched_times[x] - target)))
            if abs(float(matched_times[other] - target)) > 0.05 or other <= index:
                continue
            estimate_delta = rotation @ (matched_est[other] - matched_est[index])
            gt_delta = matched_gt[other] - matched_gt[index]
            errors_rpe.append(float(np.linalg.norm(estimate_delta - gt_delta)))
        result[f"rpe_{int(dt)}s_m"] = float(np.sqrt(np.mean(np.asarray(errors_rpe) ** 2))) if errors_rpe else float("nan")
    return result


def rank_distribution(rows, key):
    counts = Counter(integer(row, key) for row in rows)
    return ";".join(f"{rank}:{counts[rank]}" for rank in sorted(counts))


def weak_rows(rows):
    return [row for row in rows if integer(row, "weak_rank_R") + integer(row, "weak_rank_t") > 0]


def valid_cost(row, name):
    return integer(row, f"valid_{name}") == 1 and finite(number(row, f"cost_{name}"))


def event_candidate(row):
    weak = integer(row, "weak_rank_R") + integer(row, "weak_rank_t") > 0
    weak_norm = number(row, "weak_norm_l")
    ratio = number(row, "weak_ratio")
    return (weak and finite(weak_norm) and weak_norm > 1.0e-6 and finite(ratio) and ratio < 0.5
            and valid_cost(row, "init") and valid_cost(row, "lidar")
            and number(row, "cost_lidar") < number(row, "cost_init"))


def sustained_events(sequence, rows, mode_rows):
    candidates = [row for row in rows if event_candidate(row)]
    by_frame = {integer(row, "frame"): row for row in rows}
    groups = []
    current = []
    for row in candidates:
        frame = integer(row, "frame")
        if current and frame != integer(current[-1], "frame") + 1:
            if len(current) >= 3:
                groups.append(current)
            current = []
        current.append(row)
    if len(current) >= 3:
        groups.append(current)

    output = []
    for event_id, group in enumerate(groups, 1):
        frames = {integer(row, "frame") for row in group}
        modes = [row for row in mode_rows if integer(row, "frame") in frames]
        mode_counts = Counter(f"{row.get('mode_type','?')}{row.get('mode_index','?')}" for row in modes)
        dominant = mode_counts.most_common(1)[0][0] if mode_counts else "weak-projector"
        suppressions = [number(row, "weak_ratio") for row in group if finite(number(row, "weak_ratio"))]
        improvements = [1.0 - number(row, "cost_lidar") / number(row, "cost_init")
                        for row in group if number(row, "cost_init") > 0]
        nonlinear = [row for row in group if integer(row, "nonlinear_valid") == 1]
        output.append({
            "sequence": sequence,
            "event_id": event_id,
            "start_timestamp": number(group[0], "timestamp"),
            "end_timestamp": number(group[-1], "timestamp"),
            "duration_s": number(group[-1], "timestamp") - number(group[0], "timestamp"),
            "frames": len(group),
            "dominant_weak_mode": dominant,
            "median_c_L_norm": median([number(row, "weak_norm_l") for row in group]),
            "median_c_tight_norm": median([number(row, "weak_norm_tight") for row in group]),
            "median_suppression": median(suppressions),
            "median_cost_improvement": median(improvements),
            "nonlinear_valid_fraction": len(nonlinear) / len(group),
            "native_local_behavior": "nonlinear-shadow-valid" if nonlinear else "nonlinear-shadow-invalid",
        })
    return output


def sequence_report(name, spec, run_root, native_root, output_dir):
    run = spec["run"]
    shadow_dir = pathlib.Path(run_root) / f"{run}_shadow"
    native_dir = pathlib.Path(native_root) / f"{run}_native"
    rows, repaired = load_frame_csv(shadow_dir / "prompt14_shadow.csv")
    mode_rows = load_modes(shadow_dir / "prompt14_modes.csv")
    trajectory = trajectory_metrics(native_dir / "trajectory.tum", spec["gt"])
    valid = [row for row in rows if integer(row, "matched_valid") == 1]
    basis = [row for row in rows if integer(row, "basis_contract_ok") == 1]
    weak = weak_rows(valid)
    ratios = [number(row, "weak_ratio") for row in weak
              if finite(number(row, "weak_ratio")) and number(row, "weak_norm_l") > 1.0e-9]
    cost_weak = [row for row in weak if valid_cost(row, "init") and valid_cost(row, "lidar")]
    candidates = [row for row in valid if event_candidate(row)]
    agree = [row for row in valid if finite(number(row, "delta_l_norm")) and
             number(row, "delta_difference_norm") <= 0.25 * max(number(row, "delta_l_norm"), 1.0e-6)]
    strong_disagreement = [row for row in valid if integer(row, "weak_rank_R") + integer(row, "weak_rank_t") == 0 and
                           number(row, "delta_difference_norm") > 0.5 * max(number(row, "delta_l_norm"), 1.0e-6)]
    nonlinear_effect = [row for row in valid if integer(row, "nonlinear_valid") == 1 and
                        number(row, "delta_l_norm") > 1.0e-6 and
                        np.linalg.norm(np.asarray([number(row, f"nonlinear_delta_{i}") for i in range(6)]) -
                                       np.asarray([number(row, f"delta_l_{i}") for i in range(6)])) >
                        0.25 * number(row, "delta_l_norm")]
    result = {
        "sequence": name,
        "sensor_config": spec["sensor_config"],
        "frames_processed": len(rows),
        "matched_valid_frames": len(valid),
        "basis_contract_fraction": len(basis) / len(rows) if rows else float("nan"),
        "csv_schema_rows_repaired": repaired,
        **trajectory,
        "median_cond_R": median(values(valid, "cond_R")),
        "p95_cond_R": percentile(values(valid, "cond_R"), 0.95),
        "median_cond_t": median(values(valid, "cond_t")),
        "p95_cond_t": percentile(values(valid, "cond_t"), 0.95),
        "fraction_weak_R": fraction(valid, lambda row: integer(row, "weak_rank_R") > 0),
        "fraction_weak_t": fraction(valid, lambda row: integer(row, "weak_rank_t") > 0),
        "weak_rank_distribution": f"R[{rank_distribution(valid, 'weak_rank_R')}] T[{rank_distribution(valid, 'weak_rank_t')}]",
        "median_delta_L": median(values(valid, "delta_l_norm")),
        "median_delta_tight": median(values(valid, "delta_tight_norm")),
        "median_weak_delta_L": median(values(weak, "weak_norm_l")),
        "median_weak_delta_tight": median(values(weak, "weak_norm_tight")),
        "median_strong_delta_L": median(values(valid, "strong_norm_l")),
        "median_strong_delta_tight": median(values(valid, "strong_norm_tight")),
        "median_weak_suppression": median(ratios),
        "p10_weak_suppression": percentile(ratios, 0.10),
        "p50_weak_suppression": percentile(ratios, 0.50),
        "p90_weak_suppression": percentile(ratios, 0.90),
        "fraction_weak_suppression_lt_0_5": fraction(ratios, lambda value: value < 0.5),
        "fraction_weak_suppression_lt_0_25": fraction(ratios, lambda value: value < 0.25),
        "fraction_weak_suppression_lt_0_1": fraction(ratios, lambda value: value < 0.1),
        "fraction_weak_cost_lidar_better": fraction(cost_weak, lambda row: number(row, "cost_lidar") < number(row, "cost_tight")),
        "prior_suppression_candidates": len(candidates),
        "prior_suppression_candidate_fraction": len(candidates) / len(valid) if valid else float("nan"),
        "agree_fraction": len(agree) / len(valid) if valid else float("nan"),
        "strong_disagreement_fraction": len(strong_disagreement) / len(valid) if valid else float("nan"),
        "nonlinear_effect_fraction": len(nonlinear_effect) / len(valid) if valid else float("nan"),
    }
    events = sustained_events(name, valid, mode_rows)
    result["sustained_event_count"] = len(events)
    result["sustained_event_max_frames"] = max((event["frames"] for event in events), default=0)

    markdown = output_dir / {
        "bridge01": "BRIDGE01_SUMMARY.md",
        "stairs_alpha": "STAIRS_ALPHA_SUMMARY.md",
        "tunnel1_gamma": "TUNNEL1_GAMMA_SUMMARY.md",
        "tunnel2_alpha": "TUNNEL2_ALPHA_SUMMARY.md",
        "tunnel2_gamma": "TUNNEL2_GAMMA_SUMMARY.md",
    }[name]
    lines = [f"# {name} Prompt14 summary", "", f"Sensor/config: `{spec['sensor_config']}`", "",
             "| metric | value |", "|---|---:|"]
    for key, value in result.items():
        if key in ("sequence", "sensor_config", "weak_rank_distribution"):
            continue
        if isinstance(value, float):
            rendered = "NA" if not finite(value) else f"{value:.9g}"
        else:
            rendered = str(value)
        lines.append(f"| `{key}` | `{rendered}` |")
    lines.extend(["", f"Weak rank distribution: `{result['weak_rank_distribution']}`", "",
                  "Primary candidate rule: weak DCReg mode, meaningful weak LiDAR correction, "
                  "weak ratio < 0.5, and strict cost_lidar < cost_init.",
                  "APE/RPE are contextual trajectory metrics; they are not the Prompt14 decision gate.",
                  ""])
    markdown.write_text("\n".join(lines), encoding="utf-8")
    return result, events


def write_csv(path, rows, fields=None):
    if not rows:
        if fields:
            path.write_text(",".join(fields) + "\n", encoding="utf-8")
        else:
            path.write_text("", encoding="utf-8")
        return
    fields = fields or list(rows[0])
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--run-root", required=True, type=pathlib.Path)
    parser.add_argument("--native-root", type=pathlib.Path)
    parser.add_argument("--output-dir", required=True, type=pathlib.Path)
    args = parser.parse_args(argv)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    native_root = args.native_root or args.run_root
    summaries, events = [], []
    for name, spec in SEQUENCES.items():
        summary, sequence_events = sequence_report(
            name, spec, args.run_root, native_root, args.output_dir)
        summaries.append(summary)
        events.extend(sequence_events)
        print(f"{name}: frames={summary['frames_processed']} weak_candidates={summary['prior_suppression_candidates']} "
              f"sustained_events={summary['sustained_event_count']} ape_rmse={summary['ape_rmse_m']:.6g}")
    write_csv(args.output_dir / "GEODE_CROSS_SEQUENCE_SUMMARY.csv", summaries)
    write_csv(args.output_dir / "SUSTAINED_EVENTS.csv", events, EVENT_FIELDS)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
