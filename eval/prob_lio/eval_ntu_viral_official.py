# Provenance: recovered from the legacy `super-livo` branch of the Super-LIO
# repository (repo: /home/lc/super_livo_legacy/src/Super-LIO, branch super-livo).
#   eval_ntu_viral_official.py : commit 829b65b21dbe83cf73165096e581077b13b9c9df
#                                "feat(super-livo): recover official benchmark evaluators"
#   ntu_viral_official_ate.py   : same commit 829b65b2 (recovered alongside)
#   pose_bag_to_tum.py          : commit 17b493bc1b67bba3b0582fb631aae739cab4b0eb
#                                "bench(super-lio): reproduce pristine NTU baselines"
# Upstream evaluator provenance (pinned inside eval_ntu_viral_official.py):
#   ntu-aris/viral_eval @ 194dd4595b1fb5e8ae2a5a0c01255f816ab4082f (dataset author).
# Prob-LIO keeps this as the canonical NTU metric; do not replace with evo_ape.
#!/usr/bin/env python3
"""Python parity wrapper for the NTU VIRAL dataset-author MATLAB evaluator."""
import argparse
import hashlib
import pathlib
import sys
from typing import Optional, Sequence

import numpy as np
import yaml


UPSTREAM_REPOSITORY = "https://github.com/ntu-aris/viral_eval"
UPSTREAM_REVISION = "194dd4595b1fb5e8ae2a5a0c01255f816ab4082f"
UPSTREAM_FILES = {
    "evaluate_one.m": "20756b58d56dcaa66d9add0d41662034c01a329ca0fb4cda1215e5c853faec11",
    "traj_align.m": "f0ab16497ab35e709bf7c2c96a44aa50eec0f1bd78b595f7cd5ddd9fbb6bb319",
    "combteeth.m": "817bb3070c5d778e84e54e31a5d61094ce5c444f13e70809291a89541cba9e87",
    "vecitp.m": "730ccf7a973fba6dfaec2729d97c5861e6dc1a493df9a890c623ab21b37323d5",
    "quatconv.m": "5a28231a53b18ded9d4361e0d00738e5dbcb1cb1d0bf7ce8fd0a049dbb1294a8",
    "trans_B2prism.csv": "e4f50bcbbc3e670d268aab26ad80de77013819413833bb9642bea46db90232b9",
}
PRISM_B = np.asarray([-0.293656, -0.012288, -0.273095], dtype=float)


def sha256_file(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_tum(path):
    timestamps, positions, quaternions = [], [], []
    with pathlib.Path(path).open(encoding="utf-8") as stream:
        for line_number, line in enumerate(stream, 1):
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            fields = line.split()
            if len(fields) < 8:
                raise ValueError(f"{path}:{line_number}: expected at least 8 columns")
            values = np.asarray([float(value) for value in fields[:8]], dtype=float)
            if not np.all(np.isfinite(values)):
                raise ValueError(f"{path}:{line_number}: non-finite TUM row")
            timestamps.append(values[0])
            positions.append(values[1:4])
            quaternions.append(values[4:8])
    if not timestamps:
        raise ValueError(f"{path}: no TUM rows")
    timestamps = np.asarray(timestamps)
    if np.any(np.diff(timestamps) <= 0):
        raise ValueError(f"{path}: timestamps are not strictly increasing")
    return timestamps, np.asarray(positions), np.asarray(quaternions)


def quaternion_to_rotation(quaternion):
    norm = float(np.linalg.norm(quaternion))
    if not np.isfinite(norm) or norm <= 0:
        raise ValueError("invalid estimate quaternion")
    x, y, z, w = quaternion / norm
    return np.array(
        [
            [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
            [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
            [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
        ],
        dtype=float,
    )


def remove_dataset_position_duplicates(timestamps, positions):
    """Replicate union(unique-index(x), unique-index(y), unique-index(z))."""
    retained = set()
    for axis in range(3):
        _, first_indices = np.unique(positions[:, axis], return_index=True)
        retained.update(int(index) for index in first_indices)
    indices = np.asarray(sorted(retained), dtype=int)
    return timestamps[indices], positions[indices]


def interpolate_dataset_gt(est_times, gt_times, gt_positions, max_bracket=0.1):
    interpolated, estimate_indices = [], []
    last_gt_index = 0
    for estimate_index, timestamp in enumerate(est_times):
        match = None
        for gt_index in range(last_gt_index, len(gt_times) - 1):
            if (
                gt_times[gt_index] <= timestamp < gt_times[gt_index + 1]
                and abs(gt_times[gt_index + 1] - gt_times[gt_index]) < max_bracket
            ):
                match = gt_index
                last_gt_index = gt_index
                break
        if match is None:
            continue
        fraction = (timestamp - gt_times[match]) / (gt_times[match + 1] - gt_times[match])
        interpolated.append(
            gt_positions[match]
            + fraction * (gt_positions[match + 1] - gt_positions[match])
        )
        estimate_indices.append(estimate_index)
    return np.asarray(estimate_indices, dtype=int), np.asarray(interpolated, dtype=float)


def align_se3(estimate, ground_truth):
    if estimate.shape != ground_truth.shape or len(estimate) < 3:
        raise ValueError("SE(3) alignment requires at least three position pairs")
    estimate_mean = estimate.mean(axis=0)
    gt_mean = ground_truth.mean(axis=0)
    covariance = (ground_truth - gt_mean).T @ (estimate - estimate_mean) / len(estimate)
    u_matrix, _, vt_matrix = np.linalg.svd(covariance)
    correction = np.eye(3)
    if np.linalg.det(u_matrix) * np.linalg.det(vt_matrix.T) < 0:
        correction[2, 2] = -1
    rotation = u_matrix @ correction @ vt_matrix
    translation = gt_mean - rotation @ estimate_mean
    return rotation, translation


def evaluate(est_times, est_positions, est_quaternions, gt_times, gt_positions, prism_b=PRISM_B):
    gt_times, gt_positions = remove_dataset_position_duplicates(gt_times, gt_positions)
    prism_positions = np.empty_like(est_positions)
    for index, (position, quaternion) in enumerate(zip(est_positions, est_quaternions)):
        prism_positions[index] = position + quaternion_to_rotation(quaternion) @ prism_b
    indices, interpolated_gt = interpolate_dataset_gt(
        est_times, gt_times, gt_positions, max_bracket=0.1
    )
    if len(indices) < 3:
        raise ValueError(f"insufficient interpolated matches: {len(indices)} < 3")
    compared = prism_positions[indices]
    rotation, translation = align_se3(compared, interpolated_gt)
    aligned = (rotation @ compared.T).T + translation
    component_errors = interpolated_gt - aligned
    component_rms = np.sqrt(np.mean(component_errors ** 2, axis=0))
    ate = float(np.linalg.norm(component_rms))
    return {
        "primary_metric": "NTU_VIRAL_DATASET_TRANSLATION_ATE_RMSE_M",
        "translation_ate_rmse_m": ate,
        "component_rmse_m": [float(value) for value in component_rms],
        "matched": int(len(indices)),
        "lever_arm_applied": bool(np.linalg.norm(prism_b) > 0),
        "prism_translation_B_m": [float(value) for value in prism_b],
        "alignment": "SE3_UMEYAMA_NO_SCALE",
        "association": "LINEAR_GT_INTERPOLATION_AT_ESTIMATE_TIMESTAMPS",
        "interpolation_max_bracket_s": 0.1,
        "interpolation_bracket_comparison": "STRICT_LESS_THAN",
        "estimated_pose_frame": "IMU_BODY_W_T_B",
        "ground_truth_measurement_frame": "LEICA_PRISM_POSITION_IN_LEICA_WORLD",
    }


def identity(path):
    path = pathlib.Path(path).resolve()
    return {"path": str(path), "size_bytes": path.stat().st_size, "sha256": sha256_file(path)}


def parse_args(argv: Optional[Sequence[str]] = None):
    parser = argparse.ArgumentParser(description="NTU VIRAL dataset-author-compatible evaluator")
    parser.add_argument("estimate", type=pathlib.Path, help="estimated IMU/body W_T_B in TUM")
    parser.add_argument("leica", type=pathlib.Path, help="Leica prism positions in TUM")
    parser.add_argument("--out", type=pathlib.Path, required=True)
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(argv)
    try:
        est_times, est_positions, est_quaternions = load_tum(args.estimate)
        gt_times, gt_positions, _ = load_tum(args.leica)
        metrics = evaluate(
            est_times, est_positions, est_quaternions, gt_times, gt_positions, PRISM_B
        )
        result = {
            "schema_version": 1,
            "evaluator": {
                "name": metrics["primary_metric"],
                "provenance_tier": "DATASET_AUTHOR_BENCHMARK",
                "upstream_repository": UPSTREAM_REPOSITORY,
                "upstream_revision": UPSTREAM_REVISION,
                "upstream_files_sha256": UPSTREAM_FILES,
                "wrapper_path": str(pathlib.Path(__file__).resolve()),
                "wrapper_sha256": sha256_file(pathlib.Path(__file__)),
            },
            "inputs": {"estimate": identity(args.estimate), "leica": identity(args.leica)},
            "result": metrics,
        }
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(yaml.safe_dump(result, sort_keys=False), encoding="utf-8")
        print(f"primary_metric: {metrics['primary_metric']}")
        print(f"translation_ate_rmse_m: {metrics['translation_ate_rmse_m']:.9f}")
        print(f"matched: {metrics['matched']}")
        return 0
    except (OSError, ValueError, np.linalg.LinAlgError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
