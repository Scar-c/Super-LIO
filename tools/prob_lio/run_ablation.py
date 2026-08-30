#!/usr/bin/env python3
"""Run one clean, registered Prob-LIO ablation and emit a manifest.

The estimator remains the existing run_baseline.sh/offline production path.
This script only performs preflight, variant parameterization, identity
capture, evaluator dispatch, and result-manifest writing.
"""

import argparse
import datetime
import hashlib
import pathlib
import re
import subprocess
import sys
import time

import yaml


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
REGISTRY_PATH = REPO_ROOT / "eval/prob_lio/evaluator_registry.yaml"
RUNNER_PATH = REPO_ROOT / "tools/prob_lio/run_baseline.sh"

VARIANTS = {
    "B0": {
        "cov_enable": "false",
        "cov_validation_mode": "light",
        "map_pose_cov_model": "livo2_compat",
        "map_cov_storage_precision": "double",
        "qr_plane_cov_enable": "false",
        "p2p_weight_mode": "fixed_1000",
        "association_mode": "super_legacy",
        "prob_assoc_shadow_enable": "false",
        "association_pose_cov_model": "inherit_map",
        "association_sensor_cov_model": "extrinsic_consistent",
    },
    "P4-LC": {
        "cov_enable": "true",
        "cov_validation_mode": "light",
        "map_pose_cov_model": "livo2_compat",
        "map_cov_storage_precision": "double",
        "qr_plane_cov_enable": "true",
        "p2p_weight_mode": "prob_livo2",
        "association_mode": "super_legacy",
        "prob_assoc_shadow_enable": "false",
        "association_pose_cov_model": "inherit_map",
        "association_sensor_cov_model": "extrinsic_consistent",
    },
    "P4-RC": {
        "cov_enable": "true",
        "cov_validation_mode": "light",
        "map_pose_cov_model": "super_right_consistent",
        "map_cov_storage_precision": "double",
        "qr_plane_cov_enable": "true",
        "p2p_weight_mode": "prob_livo2",
        "association_mode": "super_legacy",
        "prob_assoc_shadow_enable": "false",
        "association_pose_cov_model": "inherit_map",
        "association_sensor_cov_model": "extrinsic_consistent",
    },
    "P5-ACTIVE": {
        "cov_enable": "true",
        "cov_validation_mode": "light",
        "map_pose_cov_model": "livo2_compat",
        "map_cov_storage_precision": "double",
        "qr_plane_cov_enable": "true",
        "p2p_weight_mode": "prob_livo2",
        "association_mode": "prob_livo2",
        "prob_assoc_shadow_enable": "false",
        "association_pose_cov_model": "livo2_compat",
        "association_sensor_cov_model": "livo2_active_compat",
    },
    "P5-SENSOR-CORR": {
        "cov_enable": "true",
        "cov_validation_mode": "light",
        "map_pose_cov_model": "livo2_compat",
        "map_cov_storage_precision": "double",
        "qr_plane_cov_enable": "true",
        "p2p_weight_mode": "prob_livo2",
        "association_mode": "prob_livo2",
        "prob_assoc_shadow_enable": "false",
        "association_pose_cov_model": "livo2_compat",
        "association_sensor_cov_model": "extrinsic_consistent",
    },
    "P5-BOTH-CORR": {
        "cov_enable": "true",
        "cov_validation_mode": "light",
        "map_pose_cov_model": "livo2_compat",
        "map_cov_storage_precision": "double",
        "qr_plane_cov_enable": "true",
        "p2p_weight_mode": "prob_livo2",
        "association_mode": "prob_livo2",
        "prob_assoc_shadow_enable": "false",
        "association_pose_cov_model": "super_right_consistent",
        "association_sensor_cov_model": "extrinsic_consistent",
    },
}

VARIANT_ORDER = (
    "B0", "P4-LC", "P4-RC", "P5-ACTIVE", "P5-SENSOR-CORR", "P5-BOTH-CORR"
)


def resolve(value):
    path = pathlib.Path(value)
    return path if path.is_absolute() else REPO_ROOT / path


def sha256_file(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def identity(path):
    path = pathlib.Path(path).resolve()
    return {
        "path": str(path),
        "exists": path.exists(),
        "size_bytes": path.stat().st_size if path.exists() else None,
        "sha256": sha256_file(path) if path.is_file() else None,
    }


def git_info():
    def git(*args):
        return subprocess.check_output(
            ["git", "-C", str(REPO_ROOT), *args],
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    status = git("status", "--short")
    return {
        "branch": git("branch", "--show-current"),
        "head": git("rev-parse", "HEAD"),
        "dirty": bool(status),
        "status_short": status,
        "production_tree_oid": git("rev-parse", "HEAD:src/super_lio"),
    }


def rosbag_topics(path):
    result = subprocess.run(
        ["rosbag", "info", "--yaml", str(path)],
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or "rosbag info failed")
    document = yaml.safe_load(result.stdout)
    topics = {}
    for item in (document or {}).get("topics", []):
        topics[item["topic"]] = {
            "type": item.get("type"),
            "messages": item.get("messages"),
            "frequency": item.get("frequency"),
        }
    return topics


def rosparam_overrides(variant):
    return {
        f"/lio/prob_lio/{key}": value
        for key, value in VARIANTS[variant].items()
    }


def write_yaml(path, document):
    pathlib.Path(path).write_text(
        yaml.safe_dump(document, sort_keys=False), encoding="utf-8"
    )


def parse_rows(path):
    return sum(
        1 for line in pathlib.Path(path).read_text(encoding="utf-8").splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    )


def run_command(command, cwd=REPO_ROOT):
    print("$ " + " ".join(subprocess.list2cmdline([arg]) for arg in command), flush=True)
    return subprocess.run(command, cwd=str(cwd), check=False).returncode


def evaluate(profile, trajectory, gt_path, run_dir):
    evaluator = resolve(profile["evaluator_path"])
    kind = profile["evaluator_kind"]
    result_path = run_dir / "evaluation.yaml"
    if kind == "ntu_official":
        command = [
            sys.executable, str(evaluator), str(trajectory), str(gt_path),
            "--out", str(result_path),
        ]
    elif kind == "m3dgr_aruco":
        command = [
            sys.executable, str(evaluator), str(gt_path), str(trajectory),
            "--out", str(result_path),
        ]
    elif kind == "tum_translation":
        text_path = run_dir / "evaluation.txt"
        command = [
            sys.executable, str(evaluator), str(trajectory), str(gt_path),
            "--out", str(text_path), "--max-diff", "0.05",
        ]
        result_code = run_command(command)
        if result_code != 0:
            return result_code, result_path, None
        text = text_path.read_text(encoding="utf-8")
        match = re.search(r"translation APE \(m\): RMSE=([0-9.eE+-]+)", text)
        if match is None:
            raise RuntimeError("generic evaluator output did not contain RMSE")
        return 0, text_path, {
            "primary_metric": profile["primary_metric"],
            "value": float(match.group(1)),
            "unit": profile["unit"],
            "evaluator_output": str(text_path.resolve()),
        }
    else:
        raise ValueError(f"unsupported evaluator kind: {kind}")
    result_code = run_command(command)
    if result_code != 0:
        return result_code, result_path, None
    document = yaml.safe_load(result_path.read_text(encoding="utf-8")) or {}
    metrics = document.get("result", {})
    if kind == "ntu_official":
        value = metrics["translation_ate_rmse_m"]
    else:
        value = metrics["translation_error_m"]
    return 0, result_path, {
        "primary_metric": profile["primary_metric"],
        "value": float(value),
        "unit": profile["unit"],
        "evaluator_output": str(result_path.resolve()),
        "raw_result": metrics,
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dataset", required=True)
    parser.add_argument("--sequence", required=True)
    parser.add_argument("--variant", choices=VARIANT_ORDER, required=True)
    parser.add_argument("--bag", required=True)
    parser.add_argument("--config", required=True)
    parser.add_argument("--evaluator-profile", required=True)
    parser.add_argument("--output-root", default="results/prob_lio")
    parser.add_argument("--gt")
    parser.add_argument("--gt-topic")
    parser.add_argument("--required-topic", action="append", default=[])
    parser.add_argument("--calibration", action="append", default=[])
    parser.add_argument("--canonical", action="store_true")
    parser.add_argument("--preflight-only", action="store_true")
    parser.add_argument("--run-id")
    args = parser.parse_args(argv)

    registry = yaml.safe_load(REGISTRY_PATH.read_text(encoding="utf-8"))
    profiles = registry.get("profiles", {})
    if args.evaluator_profile not in profiles:
        raise SystemExit(f"unknown evaluator profile: {args.evaluator_profile}")
    profile = profiles[args.evaluator_profile]
    if args.sequence not in profile.get("sequences", []):
        raise SystemExit(
            f"profile {args.evaluator_profile} is not registered for {args.sequence}"
        )
    profile_status = profile.get("status", "ACTIVE")

    bag = resolve(args.bag)
    config = resolve(args.config)
    evaluator = resolve(profile["evaluator_path"])
    if not bag.is_file():
        print(f"preflight bag missing: {bag}", file=sys.stderr)
        return 2
    if profile_status == "ACTIVE" and (
        not config.is_file() or not evaluator.is_file()
    ):
        print("preflight input missing", file=sys.stderr)
        return 2

    topics = rosbag_topics(bag)
    required_topics = args.required_topic or profile.get("required_topics", [])
    missing_topics = [topic for topic in required_topics if topic not in topics]
    calibration = [resolve(path) for path in args.calibration]
    gt_source = profile.get("gt_source")
    gt = None
    if gt_source in ("external_csv", "external_tum", "external_reference"):
        gt = resolve(args.gt or profile.get("gt_path"))
        if not gt.is_file():
            print(f"preflight GT missing: {gt}", file=sys.stderr)
            return 2

    preflight = {
        "schema_version": 1,
        "status": (
            profile_status
            if profile_status != "ACTIVE"
            else ("PREFLIGHT_OK" if not missing_topics else "INPUT_ADAPTER_BLOCKED")
        ),
        "dataset": args.dataset,
        "sequence": args.sequence,
        "variant": args.variant,
        "ground_truth_type": profile["ground_truth_type"],
        "primary_metric": profile["primary_metric"],
        "unit": profile["unit"],
        "bag": identity(bag),
        "topics": topics,
        "required_topics": required_topics,
        "missing_topics": missing_topics,
        "calibration": [identity(path) for path in calibration],
        "ground_truth": (
            {"source": "bag_topic", "topic": args.gt_topic or profile.get("gt_topic")}
            if gt_source == "bag_topic"
            else identity(gt)
        ),
        "config": identity(config),
        "evaluator": identity(evaluator),
        "evaluator_provenance": {
            "profile": args.evaluator_profile,
            "kind": profile["evaluator_kind"],
            "upstream": profile.get("upstream_provenance", "pinned in evaluator source"),
            "frame_transform": profile["frame_transform"],
            "timestamp_policy": profile["timestamp_policy"],
            "alignment_policy": profile["alignment_policy"],
        },
        "config_provenance": profile.get("config_provenance", {}),
        "algorithm": git_info(),
        "variant_overrides": rosparam_overrides(args.variant),
        "effective_config_snapshot": {
            "base_config": str(config.resolve()),
            "base_config_sha256": (
                sha256_file(config) if config.is_file() else None
            ),
            "requested_overrides": rosparam_overrides(args.variant),
        },
    }
    output_root = resolve(args.output_root)
    run_id = args.run_id or (
        f"{args.dataset}_{args.sequence}_{args.variant}_"
        f"{datetime.datetime.utcnow().strftime('%Y%m%dT%H%M%SZ')}"
    )
    if not re.fullmatch(r"[A-Za-z0-9_.-]+", run_id):
        print("run id may contain only letters, numbers, dot, underscore, hyphen", file=sys.stderr)
        return 2
    run_dir = output_root / run_id
    if run_dir.exists() and not args.preflight_only:
        print(f"refusing to overwrite existing run directory: {run_dir}", file=sys.stderr)
        return 2
    # A canonical run must enter run_baseline.sh with a clean worktree. Keep
    # the preflight/requested snapshot outside the repository until the shell
    # runner has performed its clean-source check and created the run dir.
    snapshot_dir = run_dir
    if not args.preflight_only and args.canonical:
        import tempfile
        snapshot_dir = pathlib.Path(tempfile.mkdtemp(prefix="prob-lio-preflight-"))
    else:
        run_dir.mkdir(parents=True, exist_ok=True)
    write_yaml(snapshot_dir / "preflight.yaml", preflight)
    if profile_status != "ACTIVE":
        print(
            f"BLOCKED({profile_status}): "
            f"{profile.get('block_reason', 'profile is blocked')}",
            file=sys.stderr,
        )
        return 4
    if missing_topics:
        return 4
    if args.preflight_only:
        print(f"preflight: PASS {run_dir / 'preflight.yaml'}")
        return 0

    expected_config = snapshot_dir / "requested_effective_config.yaml"
    write_yaml(expected_config, {
        "base_config": identity(config),
        "dataset": args.dataset,
        "sequence": args.sequence,
        "variant": args.variant,
        "rosparam_overrides": rosparam_overrides(args.variant),
    })
    command = [
        str(RUNNER_PATH), "--offline", "--bag", str(bag), "--config", str(config),
        "--out", str(output_root), "--run-id", run_id,
        "--algorithm-commit", git_info()["head"],
    ]
    if args.canonical:
        command.append("--canonical")
    for key, value in rosparam_overrides(args.variant).items():
        command.extend(["--set", f"{key}={value}"])
    started = time.monotonic()
    shell_rc = run_command(command)
    runtime_wall_s = time.monotonic() - started
    run_dir.mkdir(parents=True, exist_ok=True)
    if snapshot_dir != run_dir:
        write_yaml(run_dir / "preflight.yaml", preflight)
        write_yaml(run_dir / "requested_effective_config.yaml", {
            "base_config": identity(config),
            "dataset": args.dataset,
            "sequence": args.sequence,
            "variant": args.variant,
            "rosparam_overrides": rosparam_overrides(args.variant),
        })
    trajectory = run_dir / "trajectory.tum"
    gt_for_eval = gt
    adapter_rc = 0
    if shell_rc == 0 and gt_source == "bag_topic":
        gt_for_eval = run_dir / "ground_truth.tum"
        adapter_rc = run_command([
            sys.executable, str(resolve(profile["gt_adapter_path"])),
            "--bag", str(bag), "--topic", args.gt_topic or profile["gt_topic"],
            "--output", str(gt_for_eval),
        ])
    elif shell_rc == 0 and gt_source == "external_csv":
        gt_for_eval = run_dir / "ground_truth.tum"
        adapter_rc = run_command([
            sys.executable, str(resolve(profile["gt_adapter_path"])),
            "--csv", str(gt), "--out", str(gt_for_eval),
        ])
    eval_rc = 1
    metric = None
    eval_artifact = None
    if shell_rc == 0 and adapter_rc == 0 and trajectory.is_file():
        eval_rc, eval_artifact, metric = evaluate(
            profile, trajectory, gt_for_eval, run_dir
        )
    post_git = git_info()
    classification = "CANONICAL_VALID"
    if shell_rc != 0:
        classification = "EXECUTION_FAILED"
    elif not trajectory.is_file():
        classification = "EXECUTION_FAILED"
    elif adapter_rc != 0 or eval_rc != 0:
        classification = "EVALUATOR_BLOCKED"
    elif args.canonical and preflight["algorithm"]["dirty"]:
        classification = "EXECUTION_FAILED"
    manifest = {
        "schema_version": 1,
        "status": classification,
        "dataset": args.dataset,
        "sequence": args.sequence,
        "variant_id": args.variant,
        "ground_truth_type": profile["ground_truth_type"],
        "primary_metric_name": profile["primary_metric"],
        "primary_metric_value": metric["value"] if metric else None,
        "unit": profile["unit"],
        "evaluator_profile": args.evaluator_profile,
        "evaluator": identity(evaluator),
        "evaluator_provenance": preflight["evaluator_provenance"],
        "bag": identity(bag),
        "ground_truth": (
            {"source": "bag_topic", "topic": args.gt_topic or profile["gt_topic"],
             "derived_tum": (
                 identity(gt_for_eval)
                 if gt_for_eval is not None and pathlib.Path(gt_for_eval).is_file()
                 else None
             )}
            if gt_source == "bag_topic"
            else {"source": gt_source, "source_identity": identity(gt),
                  "derived_tum": (
                      identity(gt_for_eval)
                      if gt_for_eval is not None and pathlib.Path(gt_for_eval).is_file()
                      else None
                  )}
        ),
        "config": identity(config),
        "effective_config": identity(run_dir / "effective_rosparams.yaml"),
        "config_provenance": preflight["config_provenance"],
        # `run_git_dirty` is the source cleanliness at transaction start.
        # The output directory is intentionally created during the run and
        # may itself be untracked until evidence is committed, so retain the
        # post-run Git state only as a diagnostic field.
        "algorithm": preflight["algorithm"],
        "post_run_git": post_git,
        "run_id": run_id,
        "trajectory": identity(trajectory),
        "rows": parse_rows(trajectory) if trajectory.is_file() else 0,
        "completion": shell_rc == 0 and trajectory.is_file(),
        "return_codes": {"runner": shell_rc, "gt_adapter": adapter_rc, "evaluator": eval_rc},
        "runtime_wall_s": runtime_wall_s,
        "metric": metric,
        "artifacts": {
            "preflight": str((run_dir / "preflight.yaml").resolve()),
            "runner_meta": str((run_dir / "meta.txt").resolve()),
            "effective_config": str((run_dir / "effective_rosparams.yaml").resolve()),
            "trajectory": str(trajectory.resolve()),
            "evaluation": str(eval_artifact.resolve()) if eval_artifact else None,
        },
        "notes": "n=1 screening run; matrix is intentionally not edited by this runner",
    }
    write_yaml(run_dir / "run_manifest.yaml", manifest)
    print(f"manifest: {run_dir / 'run_manifest.yaml'}")
    print(f"status: {classification}")
    if metric:
        print(f"{metric['primary_metric']}={metric['value']:.9f} {metric['unit']}")
    return 0 if classification == "CANONICAL_VALID" else 1


if __name__ == "__main__":
    sys.exit(main())
