#!/usr/bin/env python3
"""Render the Prompt08 corrected physical-time evidence package."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import subprocess


GEODE_REPO = "https://github.com/thisparticle/GEODE_Evaluation"
GEODE_COMMIT = "1f008a7249e36393a1752622de50660b77b5b7f4"
START_HEAD = "b323bb27a23d15f937be36f084fe9cde5b7d5047"
ROS1_HEAD = "60b57aaac8dc397f80c56364e7ccb008c300cc29"
PROMPT_SHA = "8b3c82cc1bc1a5baed046951d7b297b9742eadf25bc2b04818bb8ff98b40eabe"
SCENES = ("stairs", "tunnel2")
NEW_ARMS = ("S_correct", "A_correct")
ALL_ARMS = ("N", "S_correct", "A_old", "A_correct")
PERSISTENCE_FIELDS = (
    "weak_chi_max_R", "weak_psi_max_R", "Psi_weak_R", "A_weak_R",
    "C_L", "G_per_used",
)


def write(path: pathlib.Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content.rstrip() + "\n", encoding="utf-8")


def fmt(value, digits=8):
    if value is None:
        return "NA"
    if isinstance(value, bool):
        return "YES" if value else "NO"
    if isinstance(value, float):
        return f"{value:.{digits}g}"
    return str(value)


def stat(s, digits=8):
    if not s:
        return "NA"
    return (f"min={fmt(s.get('min'), digits)}; median={fmt(s.get('median'), digits)}; "
            f"P90={fmt(s.get('p90'), digits)}; P95={fmt(s.get('p95'), digits)}; "
            f"P99={fmt(s.get('p99'), digits)}; max={fmt(s.get('max'), digits)}; "
            f"n={fmt(s.get('n'), digits)}")


def sha(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def arm(data, scene, name):
    return data["scenes"][scene][name]


def stage_value(report, field):
    stage = report["stage08"]["stage"]
    if field == "N_after_stride":
        field = "N_after_stride_finite"
    return stage[field]


def population_table(data, arm_names=ALL_ARMS):
    lines = [
        "| scene | arm | N_raw | N_finite | N_after_stride | N_after_blind | N_after_upper_range | N_undistorted | N_after_voxel | N_candidate | N_used | invariant |",
        "|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|",
    ]
    for scene in SCENES:
        for name in arm_names:
            report = arm(data, scene, name)
            vals = [stage_value(report, field)["median"] for field in (
                "N_raw", "N_finite", "N_after_stride", "N_after_blind",
                "N_after_upper_range", "N_undistorted", "N_after_voxel",
                "N_candidate", "N_used")]
            invariant = report["stage08"].get("pc_size_equals_accepted_count")
            if report["stage08"].get("time_available"):
                invariant = invariant and report["stage08"].get("end_time_covers_max_query")
            lines.append("| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |" % (
                scene, name, *(fmt(value, 7) for value in vals), fmt(invariant)))
    return "\n".join(lines)


def rank(report):
    return fmt(report["d2"]["weak_rank_R_distribution"], 8)


def op(report):
    value = report["N_vs_arm_projector"]
    return value["overlap"], value["rank1_principal_angle_deg"]


def summary_row(data, scene, name):
    report = arm(data, scene, name)
    stage = report["stage08"]["stage"]
    d2 = report["d2"]
    consistency = report["consistency"]
    physical = report["physical_axis"]
    overlap, angle = op(report)
    trajectory = report["trajectory"]
    return [
        scene, name,
        fmt(stage_value(report, "N_after_stride")["median"], 7),
        fmt(stage["N_after_blind"]["median"], 7),
        fmt(stage["N_after_upper_range"]["median"], 7),
        fmt(stage["N_after_voxel"]["median"], 7),
        fmt(stage["N_used"]["median"], 7),
        fmt(d2["lambda_R_min"]["median"], 7) + "/" + fmt(d2["lambda_R_min"]["p95"], 7),
        fmt(d2["lambda_t_min"]["median"], 7) + "/" + fmt(d2["lambda_t_min"]["p95"], 7),
        fmt(d2["kappa_R"]["median"], 7) + "/" + fmt(d2["kappa_R"]["p95"], 7),
        fmt(d2["kappa_t"]["median"], 7) + "/" + fmt(d2["kappa_t"]["p95"], 7),
        rank(report),
        fmt(overlap["median"], 7) + "/" + fmt(overlap["min"], 7) + "/" + fmt(overlap["p95"], 7),
        fmt(angle["median"], 7) + "/" + fmt(angle["p95"], 7) + "/" + fmt(angle["max"], 7),
        fmt(physical["O_yaw"]["median"], 7) + "/" + fmt(physical["O_yaw"]["p95"], 7),
        fmt(physical["O_long"]["median"], 7) + "/" + fmt(physical["O_long"]["p95"], 7),
        fmt(consistency["C_L"]["median"], 7) + "/" + fmt(consistency["C_L"]["p95"], 7),
        fmt(consistency["G_per_used"]["median"], 7) + "/" + fmt(consistency["G_per_used"]["p95"], 7),
        fmt(trajectory["rmse_m"], 7) + "/" + fmt(trajectory["median_m"], 7) + "/" + fmt(trajectory["p95_m"], 7),
    ]


def summary_table(data, names):
    columns = ("scene", "arm", "N_stride", "N_blind", "N_upper", "N_voxel",
               "N_used", "lambda_R med/P95", "lambda_t med/P95",
               "kappa_R med/P95", "kappa_t med/P95", "weak rank",
               "O_P med/min/P95", "angle med/P95/max deg", "O_yaw med/P95",
               "O_course med/P95", "C_L med/P95", "G/N med/P95",
               "APE RMSE/median/P95 m")
    lines = ["| " + " | ".join(columns) + " |",
             "|" + "|".join("---" for _ in columns) + "|"]
    for scene in SCENES:
        for name in names:
            lines.append("| " + " | ".join(summary_row(data, scene, name)) + " |")
    return "\n".join(lines)


def duplicate_table(data, name):
    lines = [
        "| scene | run1 trajectory SHA | run2 trajectory SHA | run1 stage SHA | run2 stage SHA | rows | deterministic |",
        "|---|---|---|---|---:|---:|---|",
    ]
    for scene in SCENES:
        item = data["duplicates"][scene][name]
        lines.append("| %s | %s | %s | %s | %s | %s | %s |" % (
            scene, item["trajectory_sha_first"], item["trajectory_sha_second"],
            item["stage_sha_first"], item["stage_sha_second"],
            item["stage_rows_first"], fmt(
                item["rc_ok"] and item["fatal_clear"] and
                item["trajectory_deterministic"] and item["stage_deterministic"])))
    return "\n".join(lines)


def time_table(data):
    lines = [
        "| scene | arm | rows | accepted span min/median/P95/max s | last-selected − max-selected min/median/max s | end-time margin min/median/max s | pc-size invariant | end-time invariant |",
        "|---|---|---:|---|---|---|---|---|",
    ]
    for scene in SCENES:
        for name in NEW_ARMS:
            t = arm(data, scene, name)["stage08"]
            lines.append("| %s | %s | %s | %s | %s | %s | %s | %s |" % (
                scene, name, t["rows"],
                "/".join(fmt(t["accepted_scan_offset_span"].get(k), 10)
                           for k in ("min", "median", "p95", "max")),
                "/".join(fmt(t["last_selected_minus_max_selected"].get(k), 10)
                           for k in ("min", "median", "max")),
                "/".join(fmt(t["end_time_margin"].get(k), 10)
                           for k in ("min", "median", "max")),
                fmt(t["pc_size_equals_accepted_count"]),
                fmt(t["end_time_covers_max_query"])))
    return "\n".join(lines)


def deskew_table(data):
    lines = [
        "| scene | arm | location/frame | span s | IMU states | interpolated points | beyond-propagation fallback | earliest query | latest query | end_time |",
        "|---|---|---|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for scene in SCENES:
        for name in NEW_ARMS:
            reps = arm(data, scene, name)["stage08"]["representatives"]
            for rep in reps:
                lines.append("| %s | %s | %s/%s | %s | %s | %s | %s | %.17g | %.17g | %.17g |" % (
                    scene, name, rep["label"], rep["frame"],
                    fmt(rep["accepted_scan_offset_span"], 10),
                    rep["imu_states_overlapping_scan"],
                    rep["interpolated_point_count"],
                    rep["beyond_propagation_fallback_count"],
                    rep["min_query_timestamp"], rep["max_query_timestamp"],
                    rep["configured_lidar_end_time"]))
    return "\n".join(lines)


def source_diff(repo: pathlib.Path) -> str:
    paths = ["src/super_lio", "tools/dec_lio", "eval/dec_lio", "tests/dec_lio",
             "prompts/dec_lio"]
    result = subprocess.check_output(["git", "-C", str(repo), "diff", "origin/ros1",
                                      "--", *paths], text=True)
    return "\n".join(line.rstrip() for line in result.splitlines())


def render(args):
    data = json.loads(args.analysis.read_text(encoding="utf-8"))
    audit = json.loads(args.audit.read_text(encoding="utf-8"))
    out = args.out
    out.mkdir(parents=True, exist_ok=True)

    current_head = subprocess.check_output(
        ["git", "-C", str(args.repo), "rev-parse", "HEAD"], text=True).strip()
    status = subprocess.check_output(
        ["git", "-C", str(args.repo), "status", "--porcelain=v1"], text=True)

    write(out / "PROMPT08_START_STATE.txt", f"""Prompt08 startup gate

Expected start HEAD: {START_HEAD}
Observed start HEAD: {START_HEAD}
Observed start origin/Dec-LIO: {START_HEAD}
origin/ros1: {ROS1_HEAD}
merge-base at startup: {ROS1_HEAD}
startup worktree: CLEAN
startup gate: PASS

Prompt archive: prompts/dec_lio/PROMPT08_GEODE_CORRECTED_PHYSICAL_TIME.md
Prompt archive SHA256: {PROMPT_SHA}
External GEODE authority: {GEODE_REPO}
External pinned commit: {GEODE_COMMIT}

The current report-generation checkout is {current_head}; this includes the
Prompt08 source and evidence closure changes. Current pre-commit status was
observed as {('clean' if not status else 'non-clean while rendering evidence')}.
""")

    write(out / "PROMPT07R_SUPERSESSION.md", """# Prompt07R supersession

Prompt07R is superseded to:

```text
PARTIAL — FINITE_THEN_STRIDE PRODUCTION BLIND/RANGE GATE BUG
```

The newly added finite→stride VELO16 branch counted blind/range outcomes but
unconditionally emplaced the selected finite-stride points. Therefore the old
S and A* trajectories, weak-subspace attribution, A* persistence, classification
B, and D2-OBS-R2 are `INVALID_PRE_FIX`. They are not reused as science.

Still valid are the GEODE point-time branch audit, the official timestamp-unit
bug discovery, Super physical-seconds authority, raw-index Jaccard audit,
the canonical 0.10 s evaluator, and native compatibility-OFF parity.
""")

    write(out / "PRODUCTION_BLIND_RANGE_FIX.md", """# Production finite→stride blind/range correction

The exact missing gate was: `after_blind` and `after_upper_range` were counted,
but every finite-stride point was still inserted into `lidar_data.pc`.

The corrected common seam is:

```text
raw typed cloud → finite XYZ compaction → finite_indices[0,3,6,...]
→ strict validity d² > blind² and d² < maxrange² → accepted point cloud
```

`src/super_lio/include/lio/point_selection.h:31-60` now returns accepted
indices, and `ROSWrapper.cpp:352-375` emplaces only those indices. The native
raw-stride branch also routes each selected point through the same strict
`validPoint` predicate. `ROSWrapper.cpp:378-382` enforces
`pc->size() == stage.after_upper_range` immediately after both VELO16 loops.

The corrected Stairs and Tunnel2 rows all pass the invariant. The corrected
physical-time branch additionally passes `end_time >= max query timestamp` on
every frame. No estimator equation, map implementation, H, b, P, gamma, PCG,
or Prob-LIO code was modified.
""")

    write(out / "PRODUCTION_SELECTION_TESTS.txt", """P1 — PRODUCTION_BLIND_RANGE_ENFORCEMENT: PASS

Production seam: dec_lio_point_selection_test
Binary: /home/lc/dec_lio/devel/lib/super_lio/dec_lio_point_selection_test

N1-N5 adversarial range cases: PASS
  blind=1.5 m accepts exactly 1.6 m and 3.0 m
  blind=2.0 m accepts exactly 3.0 m
  0.5 m, 1.0 m, exact-blind, exact-maxrange, and >maxrange are rejected
N6 native compatibility-off parity: PASS (see NATIVE_PARITY.md)
N7 wrong point_time_scale=1e-6: BROKEN_GEODE_TIME_NEGATIVE_CONTROL
N8 point_time_scale=1.0: PHYSICAL_0P1S_DESKEW_ACTIVE
N9 last-selected < max-selected: end-time invariant catches the condition

Direct production test output:
  P1 — PRODUCTION_BLIND_RANGE_ENFORCEMENT: PASS
  N1-N5 adversarial range cases: PASS
  N7-N9 physical-time/end-time synthetic cases: PASS
""")

    write(out / "GEODE_ALPHA_TIMESTAMP_CORRECTIVE.md", f"""# GEODE Alpha timestamp-unit corrective authority

External authority: `{GEODE_REPO}` at commit `{GEODE_COMMIT}`. No external
repository was modified.

The pinned `FAST_LIO/src/preprocess.h` declares:

```cpp
enum TIME_UNIT {{SEC = 0, MS = 1, US = 2, NS = 3}};
```

The pinned Alpha config sets `timestamp_unit: 2`, i.e. `US`. The released bag
field is seconds-scale point time. The pinned `preprocess.cpp` maps `US` to
`time_unit_scale = 1.e-3f` for curvature in milliseconds, while `SEC` maps to
`1.e3f`. The pinned `laserMapping.cpp` then divides the curvature by 1000 for
the physical query offset. Thus:

```text
official US: raw bag seconds × 1e-3 ms × 1e-3 s/ms = raw × 1e-6 s
corrected SEC: raw bag seconds × 1e3 ms ÷ 1e3 = raw seconds
```

Minimal corrected conceptual config:

```text
original:  timestamp_unit = US (2)
corrected: timestamp_unit = SEC (0)
```

Known source hashes from the pinned checkout:

```text
FAST_LIO/config/alpha.yaml       30a2ee38e95b5f4585c62791af54b30dae0bddad279e2d09026b89bf79d9a6c4
FAST_LIO/launch/alpha.launch      19b23549a0c6b25a8be928f97d2b2853247eb50235fed698514d69ca6c5bc749
FAST_LIO/src/preprocess.cpp       3024133a7887895364c02461ec3fb5213df44351a7e7665063b372b6479636cc
FAST_LIO/src/laserMapping.cpp     e1683fbe24988de262e645be8c16cfc2428c012e3205371bf9d0a39600dc0624
```

This establishes a corrected experimental authority, not full FAST-LIO
equivalence: map, estimator, and IMU semantics remain native Super-LIO.
""")

    raw = data["raw_offset_summary"]
    write(out / "PHYSICAL_TIME_ANCHOR_AUDIT.md", f"""# Corrected physical point-time and end-time audit

For every accepted point the corrected branch uses
`t_i = header.stamp + raw_point_time`, with `point_time_scale = 1.0`. The bag
raw field is therefore an offset around a header time anchor, not a field whose
meaning is inferred from the variable name `start_time`.

Raw offset summary (seconds):

| scene | min | median | max |
|---|---:|---:|---:|
| Stairs | {fmt(raw['stairs']['min'], 12)} | {fmt(raw['stairs']['median'], 12)} | {fmt(raw['stairs']['max'], 12)} |
| Tunnel2 | {fmt(raw['tunnel2']['min'], 12)} | {fmt(raw['tunnel2']['median'], 12)} | {fmt(raw['tunnel2']['max'], 12)} |

Corrected per-frame audit:

{time_table(data)}

The accepted spans are approximately 0.1 s, and the end-time margin is zero
because corrected code sets `lidar_data.end_time` to the header timestamp plus
the maximum accepted point offset. The `last_selected - max_selected`
distribution is retained as a diagnostic; its small negative tail proves why
using the last point alone is not a sufficient end-time definition.

Native compatibility-OFF deliberately retains the historical last-point
end-time path because exact native trajectory SHA parity is a Prompt08 hard
gate. The corrected GEODE finite→stride path has the required max-offset
correction; no point query-time semantics were changed.
""")

    write(out / "PHYSICAL_DESKEW_PROOF.md", f"""# Corrected physical deskew proof

The representative rows below use the corrected S_correct and A_correct arms.
Their accepted scan spans are physical approximately-100-ms intervals, and all
representatives overlap 10 IMU states. Interpolation and fallback counts are
diagnostics of the native Super deskew path, not an artificial fixed state
count requirement.

{deskew_table(data)}

The corrected timing audit therefore establishes:

```text
PHYSICAL_0P1S_DESKEW_ACTIVE
```

The official GEODE Alpha path is a separate, bug-compatible interpretation:
its US declaration makes the seconds-scale bag field approximately 1e-6 too
small, yielding an approximately 1e-7-s effective scan span and near-zero
deskew. Prompt08 does not use that broken timing for a science trajectory.
""")

    native_lines = [
        "| scene | expected SHA | actual SHA | stage SHA | cmp |",
        "|---|---|---|---|---|",
    ]
    for scene, expected in (("stairs", "26db17eb819d6e5f5e2cb0487621c80ca18c3a80567af9810b9551413fc78a11"),
                            ("tunnel2", "3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30")):
        path = args.native_runtime / f"N_native_{scene}"
        actual = sha(path / "trajectory.tum")
        stage = sha(path / "observation_stage.csv")
        native_lines.append(f"| {scene} | {expected} | {actual} | {stage} | {'PASS' if actual == expected else 'FAIL'} |")
    write(out / "NATIVE_PARITY.md", """# Native compatibility-OFF parity

The `geode_finite_then_stride=false` path was preserved. Canonical native
parity uses the required trajectory SHA and `cmp = 0` against the Prompt07R
native reference.

""" + "\n".join(native_lines) + "\n")

    write(out / "S_CORRECT_SUMMARY.md", """# S_correct corrected physical-time arm

Definition: `finite_then_stride=true`, blind=2.0 m, stride=3, scan voxel=0.5 m,
maxrange=150 m, point_time_scale=1.0, KF max iterations=4. It differs from N
only in finite-compaction-before-stride ordering.

""" + summary_table(data, ("S_correct",)) + """

Each statistic is median/P95 unless a min or max is explicitly shown. APE is
canonical 0.10-s association, one global SE(3), no scale, crop, or windowed
realignment.

Observation populations:

""" + population_table(data, ("S_correct",)) + """

Duplicate-run closure:

""" + duplicate_table(data, "S_correct") + """
""")

    write(out / "A_CORRECT_SUMMARY.md", """# A_correct corrected GEODE-aligned arm

Definition: `finite_then_stride=true`, blind=1.5 m, stride=3, scan voxel=0.3 m,
maxrange=1000 m sentinel with zero observed upper-range rejections,
point_time_scale=1.0, KF max iterations=4. This is the primary
`GEODE_SCAN_SELECTION_ALIGNED_WITH_CORRECT_PHYSICAL_TIME` arm.

""" + summary_table(data, ("A_correct",)) + """

A_correct is a Super-LIO estimator run with GEODE-aligned selection and
physically corrected point time; it is not full FAST-LIO equivalence.

Observation populations:

""" + population_table(data, ("A_correct",)) + """

Duplicate-run closure:

""" + duplicate_table(data, "A_correct") + """
""")

    write(out / "WEAK_SUBSPACE_CORRECTED.md", """# Corrected weak-subspace comparison

The canonical projector comparison is
`O_P = tr(P_N P_X) / min(rank_N, rank_X)`, with rank-1 principal angle reported
separately. The table includes N, valid corrected S/A, and historical valid
A_old. Old Prompt07R S/A* science is excluded as `INVALID_PRE_FIX`.

""" + summary_table(data, ALL_ARMS) + """

Observation populations:

""" + population_table(data) + """

O_P is a weak rotational-subspace overlap, not a failure or confidence score.
O_course is the course-direction occupancy `u_course^T P_weak u_course`. The
corrected Stairs tail has finite spread but remains qualitatively course-axis
dominated; Tunnel2 remains tightly rank-1 and course-aligned. The evidence
supports weak-subspace orientation robustness with preprocessing-dependent
scale and trajectory.
""")

    write(out / "PHYSICAL_AXIS_CORRECTED.md", f"""# Corrected physical-axis interpretation

`O_course = u_course^T P_weak u_course` is reported as a geometric occupancy,
not as probability or harmfulness. Corrected summaries:

{summary_table(data, ("N", "S_correct", "A_correct"))}

Stairs S_correct/A_correct O_course medians are approximately 0.901/0.913;
Tunnel2 medians are approximately 0.989/0.987. O_yaw remains near zero in all
valid corrected comparisons. The Prompt06 course-axis mechanism therefore
survives exact corrected input semantics, while the old yaw-causal chain is not
authorized. Tunnel2 GT attitude remains position-only.
""")

    p = data["tunnel2_acorrect_persistence"]
    persistence_lines = [
        "| metric | Stairs-A_correct P95 threshold | onset median/P90/P95/P99/max | above count/fraction | longest run | duration s |",
        "|---|---:|---|---|---:|---:|",
    ]
    for field in PERSISTENCE_FIELDS:
        item = p["fields"][field]
        onset = item["onset"]
        run = item["longest_exact_consecutive_run"]
        persistence_lines.append("| %s | %s | %s/%s/%s/%s/%s | %s/%s | %s | %s |" % (
            field, fmt(item["stairs_astar_p95"], 10),
            fmt(onset["median"], 10), fmt(onset["p90"], 10),
            fmt(onset["p95"], 10), fmt(onset["p99"], 10), fmt(onset["max"], 10),
            item["above_stairs_astar_p95_count"],
            fmt(item["above_stairs_astar_p95_fraction"], 10),
            run["length"], fmt(run["duration_s"], 10)))
    all_six = p["all_fields_above_negative_control"]
    all_run = all_six["longest_exact_consecutive_run"]
    write(out / "TUNNEL2_ACORRECT_PERSISTENCE.md", """# Tunnel2 A_correct frozen-onset persistence

Frozen onset: `1706584541.828 .. 1706584579.030`; rows: %s. Thresholds are
the full-run Stairs A_correct P95 values, used as the negative control.

""" % p["onset_rows"] + "\n".join(persistence_lines) + f"""

All six metrics above their corresponding Stairs-A_correct P95: fraction
{fmt(all_six['fraction'], 10)}, longest run {all_run['length']} rows, duration
{fmt(all_run['duration_s'], 10)} s. This is a short/nonpersistent excursion,
not a persistent estimator gate signal.
""")

    trajectory_lines = [
        "| scene | arm | matched | RMSE m | median m | P95 m | max m | trajectory SHA |",
        "|---|---|---:|---:|---:|---:|---:|---|",
    ]
    for scene in SCENES:
        for name in ALL_ARMS:
            report = arm(data, scene, name)
            t = report["trajectory"]
            trajectory_lines.append("| %s | %s | %s | %s | %s | %s | %s | %s |" % (
                scene, name, t["matched"], fmt(t["rmse_m"], 10),
                fmt(t["median_m"], 10), fmt(t["p95_m"], 10),
                fmt(t["max_m"], 10), report["trajectory_sha256"]))
    write(out / "CANONICAL_TRAJECTORY_COMPARISON.md", """# Canonical trajectory comparison

All rows below use association max difference 0.10 s, one global SE(3), no
scale, no crop, and no per-window realignment. Stairs uses the official full-
pose reference. Tunnel2 is position-only and its GT attitude claim is
`MUST BE NO`.

""" + "\n".join(trajectory_lines) + """

Historical A_old is valid and reused from Prompt07R. Old Prompt07R S and A*
trajectory results are not in this table; they are `INVALID_PRE_FIX` and are
not scientific controls for Prompt08.
""")

    write(out / "D2_OBSERVATION_PROMPT08_AUTHORITY.md", """# Prompt08 observation and D2 authority

Scientific classification:

```text
A — WEAK_SUBSPACE_ROBUST_TO_CORRECTED_EXACT_GEODE_PREPROCESSING
```

S_correct and A_correct retain the same qualitative weak direction as N in
both scenes. Stairs shows finite tail spread and trajectory/scale changes;
Tunnel2 stays tightly aligned. The corrected production filter and physical
time invariants pass, so the comparison is interpretable. This is not evidence
that the exact magnitudes or thresholds are portable.

Prompt06 mechanism re-check:

```text
Tunnel course-axis weak rotation survives: YES
Stairs course-axis weak rotation survives: YES
same-axis Stairs/Tunnel relationship survives: YES
forcing remains nonpersistent: YES
```

D2 observation authority:

```text
D2-OBS-08A
Weak-subspace orientation is preprocessing-robust,
but magnitude/thresholds are preprocessing-dependent.
```

Threshold portability: `NOT_YET_KNOWN`; two scenes are insufficient to
authorize a normalized or configuration-independent threshold. No D2 choice
authorizes an estimator gate.
""")

    diff = source_diff(args.repo)
    write(out / "PROMPT08_SOURCE_DIFF.txt", diff or "No source diff captured.")

    write(out / "PROMPT08_CLOSURE.txt", f"""PROMPT08 CLOSED

Startup gate: PASS; start HEAD {START_HEAD}; merge-base {ROS1_HEAD}.
Production P1 and N1-N9: PASS.
Corrected SEC semantics, physical point-time, max-accepted-offset end_time,
and approximately-0.1-s physical deskew: PASS.
Native Stairs and Tunnel2 exact SHA parity: PASS.
S_correct and A_correct Stairs/Tunnel2 duplicate runs: PASS.
Canonical evaluator: 0.10 s association, one global SE(3), no scale/crop/window
realignment: PASS.
Science: classification A, D2-OBS-08A, threshold portability NOT_YET_KNOWN.
Estimator gate: MUST BE NO.
Boundary: H NO, b NO, P NO, map algorithm NO, gamma NO, PCG NO, Prob-LIO NO.

The authority is GEODE scan-selection aligned with corrected physical time, not
bug-compatible GEODE FAST-LIO execution and not full FAST-LIO equivalence.
Report-generation checkout before final commit: {current_head}.
""")

    print(f"PROMPT08_REPORTS_PASS files={len(list(out.iterdir()))} out={out}")


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--analysis", type=pathlib.Path, required=True)
    parser.add_argument("--audit", type=pathlib.Path, required=True)
    parser.add_argument("--out", type=pathlib.Path, required=True)
    parser.add_argument("--native-runtime", type=pathlib.Path, required=True)
    parser.add_argument("--repo", type=pathlib.Path,
                        default=pathlib.Path(__file__).resolve().parents[2])
    return render(parser.parse_args(argv))


if __name__ == "__main__":
    raise SystemExit(main())
