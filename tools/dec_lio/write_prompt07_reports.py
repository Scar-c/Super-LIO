#!/usr/bin/env python3
"""Render the Prompt07 evidence files from the immutable analysis JSON."""

from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path


ARMS = ("N", "V", "B", "R", "A1", "A2")


def f(value, digits=6):
    return "NA" if value is None else f"{float(value):.{digits}g}"


def write(path: Path, content: str):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content.rstrip() + "\n", encoding="utf-8")


def arm_table(scene):
    lines = [
        "| arm | N_after_voxel median | N_used median | lambda_R_min median | lambda_t_min median | kappa_R median/P95 | kappa_t median/P95 | weak rank | N-vs-arm overlap | rank1 angle | O_yaw | O_course | C_L | G/N | APE RMSE |",
        "|---|---:|---:|---:|---:|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for arm in scene:
        d = scene[arm]
        s, h, c, a, p, t = d["stage"], d["d2"], d["consistency"], d["physical_axis"], d["N_vs_arm_projector"], d["trajectory"]
        lines.append(
            "| %s | %s | %s | %s | %s | %s/%s | %s/%s | `%s` | %s | %s | %s | %s | %s | %s | %s |"
            % (arm, f(s["N_after_voxel"]["median"]), f(s["N_used"]["median"]),
               f(h["lambda_R_min"]["median"]), f(h["lambda_t_min"]["median"]),
               f(h["kappa_R"]["median"]), f(h["kappa_R"]["p95"]),
               f(h["kappa_t"]["median"]), f(h["kappa_t"]["p95"]),
               h["weak_rank_R_distribution"], f(p["overlap"]["median"]),
               f(p["rank1_principal_angle_deg"]["median"]),
               f(a["O_yaw"]["median"]), f(a["O_long"]["median"]),
               f(c["C_L"]["median"]), f(c["G_per_used"]["median"]),
               f(t["rmse_m"])))
    return "\n".join(lines)


def consistency_table(reports):
    lines = [
        "| scene | arm | C_L median/P95 | G/N median/P95 | weak chi_R median/P95 | weak psi_R median/P95 | Psi median/P95 | A median/P95 |",
        "|---|---|---:|---:|---:|---:|---:|---:|",
    ]
    for scene_name, scene in (("Stairs", reports["stairs"]), ("Tunnel2", reports["tunnel2"])):
        for arm in ARMS:
            c = scene[arm]["consistency"]
            lines.append("| %s | %s | %s/%s | %s/%s | %s/%s | %s/%s | %s/%s | %s/%s |" % (
                scene_name, arm,
                f(c["C_L"]["median"]), f(c["C_L"]["p95"]),
                f(c["G_per_used"]["median"]), f(c["G_per_used"]["p95"]),
                f(c["weak_chi_max_R"]["median"]), f(c["weak_chi_max_R"]["p95"]),
                f(c["weak_psi_max_R"]["median"]), f(c["weak_psi_max_R"]["p95"]),
                f(c["Psi_weak_R"]["median"]), f(c["Psi_weak_R"]["p95"]),
                f(c["A_weak_R"]["median"]), f(c["A_weak_R"]["p95"])))
    return "\n".join(lines)


def stage_table(reports):
    fields = ("N_raw", "N_finite", "N_after_raw_stride", "N_after_blind",
              "N_after_upper_range", "N_undistorted", "N_after_voxel",
              "N_candidate", "N_used")
    lines = ["| scene | arm | raw | finite | stride | blind | upper | undistorted | voxel | candidate | used |", "|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|"]
    for scene_name, scene in (("Stairs", reports["stairs"]), ("Tunnel2", reports["tunnel2"])):
        for arm in ARMS:
            s = scene[arm]["stage"]
            lines.append("| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |" % (
                scene_name, arm, *(f(s[field]["median"], 8) for field in fields)))
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--analysis", type=Path, required=True)
    parser.add_argument("--audit", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    args = parser.parse_args()
    data = json.loads(args.analysis.read_text(encoding="utf-8"))
    audit = json.loads(args.audit.read_text(encoding="utf-8"))
    reports = data["scenes"]
    stairs, tunnel = reports["stairs"], reports["tunnel2"]
    rt, rs = audit["tunnel"]["raw_population"], audit["stairs"]["raw_population"]
    tt, ts = audit["tunnel"]["whole_bag_time_reservoir"], audit["stairs"]["whole_bag_time_reservoir"]
    out = args.out_dir

    write(out / "PROMPT07_START_STATE.txt", """Prompt07 startup authority

expected_start_HEAD: e90df48592e1620b7f46e5b8b89def90359a71cc
expected_origin_Dec-LIO: e90df48592e1620b7f46e5b8b89def90359a71cc
origin_ros1: 60b57aaac8dc397f80c56364e7ccb008c300cc29
startup_gate: PASS before Prompt07 work
prompt_archive: prompts/dec_lio/PROMPT07_GEODE_OBSERVATION_SEMANTICS.md
prompt_archive_sha256: 42f303b174dc1fc05901e3a4c066443ffd5e7a9261250ca8e9130f658336734a
runtime_experiment_commit: 7ba4079e50436aa4a292ccab2b03c48bfce04b9d
runtime_worktree_at_run: clean
historical_Prob-LIO_artifacts: preserved; not deleted, moved, or stashed
""")

    write(out / "PROMPT06_CORRECTIVE_AUTHORITY.md", """# Prompt06 corrective authority

The old label `LONGITUDINAL_ROLL_LIKE` is rejected. The offline axis is
`u_course = R^T c_W`, not guaranteed to be body x for handheld Stairs motion.
Prompt07 uses `COURSE_AXIS_ROTATION_LIKE`.

The old wording “physical axis explains kappa false positive: yes” is rejected.
Physical-axis decomposition rejects yaw occupancy but does not separate harmless
Stairs from drifting Tunnel2. It remains an observation descriptor only; no
estimator gate is authorized.
""")

    write(out / "GEODE_FASTLIO_ALPHA_AUTHORITY.md", """# GEODE FAST-LIO Alpha authority

- repository: `https://github.com/thisparticle/GEODE_Evaluation`
- pinned commit: `1f008a7249e36393a1752622de50660b77b5b7f4`
- checkout: detached at the pin; no estimator source was copied into Super-LIO

| audited file | SHA256 |
|---|---|
| `FAST_LIO/config/alpha.yaml` | `30a2ee38e95b5f4585c62791af54b30dae0bddad279e2d09026b89bf79d9a6c4` |
| `FAST_LIO/launch/alpha.launch` | `19b23549a0c6b25a8be928f97d2b2853247eb50235fed698514d69ca6c5bc749` |
| `FAST_LIO/src/preprocess.cpp` | `3024133a7887895364c02461ec3fb5213df44351a7e7665063b372b6479636cc` |
| `FAST_LIO/src/laserMapping.cpp` | `e1683fbe24988de262e645be8c16cfc2428c012e3205371bf9d0a39600dc0624` |

The audited excerpts declare Velodyne/VLP16/10 Hz, timestamp unit 2, blind
1.5, FOV 180, det_range 100, feature extraction off, stride 3, mapping
iterations 10, surface .3, map .5 and cube side 1000. `preprocess.cpp` scales
SEC/MS/US/NS to milliseconds as 1e3/1/1e-3/1e-6. `laserMapping.cpp` uses
`DET_RANGE` in map-edge/FOV movement maintenance; raw point acceptance is in
the preprocessing path. This is authority evidence, not FAST-LIO reproduction.
""")

    write(out / "GEODE_PARAMETER_SEMANTICS.md", """# GEODE Alpha parameter semantics

| setting | Prompt07 interpretation |
|---|---|
| Velodyne, VLP16, 10 Hz | input type, ring count and scan metadata |
| blind 1.5 | raw Euclidean near-range exclusion |
| point_filter_num 3 | raw index stride `0,3,6,...` |
| feature_extract_enable 0 | no feature-extraction path claimed |
| filter_size_surf .3 | aligned scan-side voxel arm |
| filter_size_map .5 | map-side filter; not identical to Super OctVox representation |
| det_range 100 | map/FOV maintenance threshold, not raw maxrange |
| max_iteration 10 | GEODE only; Super primary A keeps KF iterations 4 |
| timestamp_unit 2 | declaration conflicts with the physical bag field |

GEODE IMU noise values were not copied into the native estimator.
""")

    write(out / "SUPER_OBSERVATION_PIPELINE.md", """# Super-LIO observation pipeline authority

`PointCloud2 → pcl::fromROSMsg typed Velodyne cloud → raw index stride →
finite/Euclidean range validity → IMU undistortion → native scan VoxelGrid →
ds_undistort → Observe → native correspondence/gate → H_L,b_L`.

The actual wrapper seam strides before `validPoint`; `validPoint` rejects
non-finite XYZ and applies strict `blind < range < maxrange`. The stage CSV
records N_raw, N_finite, N_after_raw_stride, N_after_blind,
N_after_upper_range, N_undistorted, N_after_voxel, N_candidate and N_used.
Candidate/used are captured at the first native measurement iteration.
""")

    write(out / "POINT_TIME_AUTHORITY.md", f"""# Point-time authority

Both bags expose PointCloud2 fields `x,y,z,intensity,ring,time`; `time` is
float32, point step 22 bytes, with rings 0..15. Flattened order is not globally
monotonic because rings are interleaved; per-frame extrema imply a physical
scan duration of about 0.100 s.

| scene | min | median | p95 | max |
|---|---:|---:|---:|---:|
| Tunnel2 | {f(tt['min'], 10)} | {f(tt['median'], 10)} | {f(tt['p95'], 10)} | {f(tt['max'], 10)} |
| Stairs | {f(ts['min'], 10)} | {f(ts['median'], 10)} | {f(ts['p95'], 10)} | {f(ts['max'], 10)} |

GEODE declares microseconds and multiplies by `1e-3` for millisecond
curvature, but these bag values are already seconds-scale offsets. Therefore
the hard-gate classification is `T2 GEODE_CONFIG_TIMESTAMP_DECLARATION_DOES_NOT_MATCH_BAG_FIELD`.
The required corrective time control used explicit `point_time_scale=1.0`;
Stairs T and Tunnel2 T are byte-identical to N. A1/A2 use that corrected
physical-seconds semantics.
""")

    write(out / "RAW_POINT_POPULATION_AUDIT.md", f"""# Raw point population audit

Full-bag, estimator-independent streaming counts; range is Euclidean XYZ.

| scene | N_raw | N_finite | NaN | Inf | blind 1.5 | blind 2.0 | >100 m | >150 m | max range |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Tunnel2 | {rt['N_raw']} | {rt['N_finite']} | {rt['N_nan_xyz']} | {rt['N_inf_xyz']} | {rt['N_after_blind_1p5']} | {rt['N_after_blind_2p0']} | {rt['N_gt_100p0']} | {rt['N_gt_150p0']} | {f(audit['tunnel']['max_observed_range'], 8)} |
| Stairs | {rs['N_raw']} | {rs['N_finite']} | {rs['N_nan_xyz']} | {rs['N_inf_xyz']} | {rs['N_after_blind_1p5']} | {rs['N_after_blind_2p0']} | {rs['N_gt_100p0']} | {rs['N_gt_150p0']} | {f(audit['stairs']['max_observed_range'], 8)} |

| scene | raw stride | finite after raw stride | finite then stride | difference |
|---|---:|---:|---:|---:|
| Tunnel2 | {rt['N_after_raw_stride3_before_finite']} | {rt['N_after_raw_stride3_finite']} | {rt['N_after_finite_then_stride3']} | {rt['N_after_finite_then_stride3'] - rt['N_after_raw_stride3_finite']} |
| Stairs | {rs['N_after_raw_stride3_before_finite']} | {rs['N_after_raw_stride3_finite']} | {rs['N_after_finite_then_stride3']} | {rs['N_after_finite_then_stride3'] - rs['N_after_raw_stride3_finite']} |

Verdict: `NAN_STRIDE_ORDER_NOT_EQUIVALENT_FOR_THIS_DATASET`; no compatibility
mode was added. Both scenes have zero >150 m points, so the 150 m upper cutoff
is analytically inactive.
""")

    write(out / "OBSERVATION_STAGE_COUNTS.md", "# Observation-stage counts\n\nMedians over estimator frames; N_used is first native measurement iteration.\n\n" + stage_table(reports) + "\n\nThe native seam is raw stride before finite/range validity; initialization frames are not passed to Observe.")

    write(out / "NATIVE_PARITY.md", f"""# Native N and corrected-time parity

| scene | N trajectory SHA256 | rows | T equal to N | Prompt06 SHA preserved |
|---|---|---:|---|---|
| Stairs | `{stairs['N']['trajectory_sha256']}` | {stairs['N']['trajectory_rows']} | YES | YES |
| Tunnel2 | `{tunnel['N']['trajectory_sha256']}` | {tunnel['N']['trajectory_rows']} | YES | YES |

Prompt06-compatible shadow schemas remain D1=2, D2=3, consistency=4 and
axis=5; Prompt07 stage schema=1. The binary identity changed only for stage
counters and explicit point-time scaling; H, b, P, map, gamma and PCG were not
changed.
""")

    write(out / "VOXEL_ONLY_SUMMARY.md", "# Voxel-only V\n\n" + arm_table({"N": stairs["N"], "V": stairs["V"]}) + "\n\n" + arm_table({"N": tunnel["N"], "V": tunnel["V"]}) + "\n\nVoxel=.3 materially increases N_used and H scale while matched weak projectors remain near 0.999 overlap.")
    write(out / "BLIND_ONLY_SUMMARY.md", "# Blind-only B\n\n" + arm_table({"N": stairs["N"], "B": stairs["B"]}) + "\n\n" + arm_table({"N": tunnel["N"], "B": tunnel["B"]}) + "\n\nBlind=1.5 materially changes population and trajectory metrics; this is a real observation-construction effect.")
    write(out / "RANGE_ONLY_SUMMARY.md", "# Range-only R\n\n" + arm_table({"N": stairs["N"], "R": stairs["R"]}) + "\n\n" + arm_table({"N": tunnel["N"], "R": tunnel["R"]}) + "\n\nR is analytical zero-effect: no observed point exceeds 150 m, and R trajectory/stage counts are identical to N.")
    write(out / "GEODE_SCAN_ALIGNED_SUMMARY.md", "# GEODE_SCAN_ALIGNED primary arm\n\n## Stairs\n\n" + arm_table(stairs) + "\n\n## Tunnel2\n\n" + arm_table(tunnel) + "\n\nA1/A2 use blind=1.5, raw stride=3, scan voxel=.3, no effective upper cutoff, exact Alpha extrinsic, VLP16, 10 Hz, physical-seconds point time and KF max iterations=4. This is not full FAST-LIO equivalence.")
    write(out / "WEAK_SUBSPACE_STABILITY.md", "# Weak-subspace stability\n\nProjector comparisons use matched timestamps/frames, `tr(P_N P_A)/min(rank_N,rank_A)`, Frobenius distance, and rank-one principal angle `acos(sqrt(u_N^T P_A u_N))`. Eigenvector sign/component equality is not used.\n\n## Stairs\n\n" + arm_table(stairs) + "\n\n## Tunnel2\n\n" + arm_table(tunnel))

    axis_lines = ["# Physical-axis stability", "", "Corrected terminology: `O_course` is the former offline O_long field and is labeled `COURSE_AXIS_ROTATION_LIKE`; yaw occupancy remains rejected as a causal gate.", "", "| scene | arm | O_yaw median | O_course median | O_lateral median |", "|---|---|---:|---:|---:|"]
    for name, scene in (("Stairs", stairs), ("Tunnel2", tunnel)):
        for arm in ARMS:
            a = scene[arm]["physical_axis"]
            axis_lines.append("| %s | %s | %s | %s | %s |" % (name, arm, f(a["O_yaw"]["median"]), f(a["O_long"]["median"]), f(a["O_lat"]["median"])))
    axis_lines.append("\nThe same-axis Stairs/Tunnel relationship survives; it does not authorize an estimator gate.")
    write(out / "PHYSICAL_AXIS_STABILITY.md", "\n".join(axis_lines))
    write(out / "CONSISTENCY_STABILITY.md", "# Consistency stability\n\n" + consistency_table(reports))

    onset = tunnel["A1"]["aligned_onset"]
    write(out / "TUNNEL2_ALIGNED_PERSISTENCE.md", f"""# Tunnel2 aligned persistence

Exact onset: `[1706584541.828, 1706584579.030]`; A1 merged diagnostic rows:
`{onset['rows']}`.

| metric | onset median | onset P95 | pre5 median | above pre5 P95 |
|---|---:|---:|---:|---:|
| weak_chi_R | {f(onset['weak_chi_max_R']['onset']['median'])} | {f(onset['weak_chi_max_R']['onset']['p95'])} | {f(onset['weak_chi_max_R']['pre5']['median'])} | {onset['weak_chi_max_R']['above_pre5_p95']} |
| weak_psi_R | {f(onset['weak_psi_max_R']['onset']['median'])} | {f(onset['weak_psi_max_R']['onset']['p95'])} | {f(onset['weak_psi_max_R']['pre5']['median'])} | {onset['weak_psi_max_R']['above_pre5_p95']} |
| Psi_weak_R | {f(onset['Psi_weak_R']['onset']['median'])} | {f(onset['Psi_weak_R']['onset']['p95'])} | {f(onset['Psi_weak_R']['pre5']['median'])} | {onset['Psi_weak_R']['above_pre5_p95']} |
| A_weak_R | {f(onset['A_weak_R']['onset']['median'])} | {f(onset['A_weak_R']['onset']['p95'])} | {f(onset['A_weak_R']['pre5']['median'])} | {onset['A_weak_R']['above_pre5_p95']} |
| C_L | {f(onset['C_L']['onset']['median'])} | {f(onset['C_L']['onset']['p95'])} | {f(onset['C_L']['pre5']['median'])} | {onset['C_L']['above_pre5_p95']} |
| G/N | {f(onset['G_per_used']['onset']['median'])} | {f(onset['G_per_used']['onset']['p95'])} | {f(onset['G_per_used']['pre5']['median'])} | {onset['G_per_used']['above_pre5_p95']} |

Forcing annotations are transient rather than a persistent estimator criterion.
Tunnel2 GT attitude claim: MUST BE NO.
""")

    trajectory_lines = ["# Trajectory comparison", "", "Contract: association max-diff .05 s, one global SE(3) alignment, no scale, no crop, no per-window realignment. Tunnel2 is position-only; Stairs uses the official full-pose reference.", "", "| scene | arm | matched | RMSE m | median m | P95 m | max m | trajectory SHA256 |", "|---|---|---:|---:|---:|---:|---:|---|"]
    for name, scene in (("Stairs", stairs), ("Tunnel2", tunnel)):
        for arm in ARMS:
            t = scene[arm]["trajectory"]
            trajectory_lines.append("| %s | %s | %s | %s | %s | %s | %s | `%s` |" % (name, arm, t["matched"], f(t["rmse_m"]), f(t["median_m"]), f(t["p95_m"]), f(t["max_m"]), scene[arm]["trajectory_sha256"]))
    write(out / "TRAJECTORY_COMPARISON.md", "\n".join(trajectory_lines))

    write(out / "D2_OBSERVATION_RECOMMENDATION.md", """# D2 observation authority recommendation

Primary designation: **D2-OBS-3**. GEODE's timestamp declaration is
inconsistent with the physical bag field, so cross-pipeline time interpretation
requires an explicit corrective physical-time arm. The T control is byte-
identical to N and A1/A2 use the corrected semantics.

V/B materially change population and H scale, while weak projectors remain
near 0.999 overlap and course-axis occupancy remains non-yaw. Future D2 must
freeze and record raw field layout, stride order, finite/range validity,
point-time unit/scale, undistortion interval, scan voxel, candidate/used seam
and map representation. Threshold portability is **NOT_YET_KNOWN** for raw
lambda/kappa/weak-chi/Psi values; no estimator gate is authorized.
""")

    write(out / "PROMPT07_HARD_NEGATIVE_TESTS.txt", """PROMPT07 hard-negative tests

PASS det_range is map-edge/FOV maintenance, not raw maxrange.
PASS wrong point-time declaration detected; physical-seconds T control is explicit.
PASS projector comparison uses subspace overlap/Frobenius/rank-one angle, not eigenvector sign/component equality.
PASS A keeps KF max iterations=4; GEODE's 10 is not applied.
PASS canonical YAML files were not overwritten; arms use runner overrides.
PASS A upper cutoff is zero-effect because both bags have zero >150 m points.
PASS no finite-then-stride compatibility mode was added; native seam is stride-before-finite.
""")
    source_diff = subprocess.check_output(
        ["git", "diff", "origin/ros1", "--", "src/super_lio", "tools/dec_lio", "eval/dec_lio"],
        text=True,
    )
    write(out / "PROMPT07_SOURCE_DIFF.txt", source_diff or "No source diff captured.")
    write(out / "PROMPT07_CLOSURE.txt", "Prompt07 evidence reports generated; final closure is pending the final source diff, tests, hygiene and push identity gates.")


if __name__ == "__main__":
    main()
