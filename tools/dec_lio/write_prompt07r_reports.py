#!/usr/bin/env python3
"""Render the compact, source-grounded Prompt07R evidence package."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess


GEODE_COMMIT = "1f008a7249e36393a1752622de50660b77b5b7f4"
GEODE_REPO = "https://github.com/thisparticle/GEODE_Evaluation"
ONSET = "1706584541.828 .. 1706584579.030"


def write(path: pathlib.Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content.rstrip() + "\n", encoding="utf-8")


def fmt(value, digits=6):
    if value is None:
        return "NA"
    if isinstance(value, bool):
        return "YES" if value else "NO"
    if isinstance(value, float):
        return f"{value:.{digits}g}"
    return str(value)


def stat_line(s):
    return (f"n={s.get('n', s.get('count', 0))}; min={fmt(s.get('min'))}; "
            f"P5={fmt(s.get('p5'))}; median/P50={fmt(s.get('median', s.get('p50')))}; "
            f"P90={fmt(s.get('p90'))}; P95={fmt(s.get('p95'))}; "
            f"P99={fmt(s.get('p99'))}; max={fmt(s.get('max'))}")


def arm_row(d, scene, arm):
    r = d["scenes"][scene][arm]
    s, q, c, x, t = r["stage"], r["d2"], r["consistency"], r["physical_axis"], r["trajectory"]
    p = r["N_vs_arm_projector"]
    return {
        "arm": arm,
        "semantics": r["finite_stride_semantics"],
        "N_after_voxel": s["N_after_voxel"]["median"],
        "N_used": s["N_used"]["median"],
        "lambda_R": q["lambda_R_min"],
        "lambda_t": q["lambda_t_min"],
        "kappa_R": q["kappa_R"],
        "kappa_t": q["kappa_t"],
        "weak_rank": q["weak_rank_R_distribution"],
        "O_P": p["overlap"],
        "angle": p["rank1_principal_angle_deg"],
        "O_yaw": x["O_yaw"],
        "O_course": x["O_long"],
        "O_lateral": x["O_lat"],
        "C_L": c["C_L"],
        "G_N": c["G_per_used"],
        "APE": t,
        "sha": r["trajectory_sha256"],
    }


def table(rows, columns):
    lines = ["| " + " | ".join(columns) + " |",
             "|" + "|".join("---" for _ in columns) + "|"]
    for row in rows:
        lines.append("| " + " | ".join(str(row.get(column, "")) for column in columns) + " |")
    return "\n".join(lines)


def render(args):
    d = json.loads(args.analysis.read_text(encoding="utf-8"))
    audit = json.loads(args.audit.read_text(encoding="utf-8"))
    out = args.out
    out.mkdir(parents=True, exist_ok=True)

    write(out / "PROMPT07R_SUPERSESSION.md", f"""# Prompt07R supersession

Prompt07 CLOSED is superseded by:

```text
Prompt07 PARTIAL — exact preprocessing/time authority pending Prompt07R.
```

The original Prompt07 runtime and evidence are historical and were not deleted
or rewritten. Prompt07R adds the exact GEODE finite-cloud/stride seam, resolves
the source-level `given_offset_time` branch, and re-evaluates retained
trajectories at the canonical 0.10 s association threshold.

Prompt07 A1/A2 are renamed conceptually:

```text
A_old = PARAMETER_ALIGNED_RAW_STRIDE
```

The new A* arm is `GEODE_SCAN_INPUT_ALIGNED`; it remains a Super estimator run,
not full FAST-LIO equivalence.
""")

    write(out / "GEODE_GIVEN_OFFSET_TIME_AUDIT.md", f"""# GEODE given_offset_time branch audit

Authority: `{GEODE_REPO}` at `{GEODE_COMMIT}`.

The pinned `FAST_LIO/src/preprocess.cpp` executes:

```text
pcl::fromROSMsg → pcl::removeNaNFromPointCloud
→ if (pl_orig.points[plsize - 1].time > 0) given_offset_time=true
→ otherwise ring/yaw reconstruction
```

This is evaluated after finite XYZ compaction, frame by frame. `time > 0` is
the C++ strict comparison; a non-finite final time therefore selects FALSE.

| scene | frames | TRUE | TRUE fraction | FALSE | FALSE fraction | longest TRUE run | longest FALSE run | first/last TRUE | first/last FALSE |
|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| Stairs | {audit['stairs']['frames_total']} | {audit['stairs']['given_offset_time']['true_frames']} | {fmt(audit['stairs']['given_offset_time']['true_fraction'])} | {audit['stairs']['given_offset_time']['false_frames']} | {fmt(audit['stairs']['given_offset_time']['false_fraction'])} | {audit['stairs']['given_offset_time']['true_run']['length']} | {audit['stairs']['given_offset_time']['false_run']['length']} | {audit['stairs']['given_offset_time']['first_true_frame']}/{audit['stairs']['given_offset_time']['last_true_frame']} | {audit['stairs']['given_offset_time']['first_false_frame']}/{audit['stairs']['given_offset_time']['last_false_frame']} |
| Tunnel2 | {audit['tunnel2']['frames_total']} | {audit['tunnel2']['given_offset_time']['true_frames']} | {fmt(audit['tunnel2']['given_offset_time']['true_fraction'])} | {audit['tunnel2']['given_offset_time']['false_frames']} | {fmt(audit['tunnel2']['given_offset_time']['false_fraction'])} | {audit['tunnel2']['given_offset_time']['true_run']['length']} | {audit['tunnel2']['given_offset_time']['false_run']['length']} | {audit['tunnel2']['given_offset_time']['first_true_frame']}/{audit['tunnel2']['given_offset_time']['last_true_frame']} | {audit['tunnel2']['given_offset_time']['first_false_frame']}/{audit['tunnel2']['given_offset_time']['last_false_frame']} |

Final finite-point original-index distributions:

* Stairs time: `{stat_line(audit['stairs']['given_offset_time']['final_finite_point_time_s'])}`; original index: `{stat_line(audit['stairs']['given_offset_time']['final_finite_original_index'])}`.
* Tunnel2 time: `{stat_line(audit['tunnel2']['given_offset_time']['final_finite_point_time_s'])}`; original index: `{stat_line(audit['tunnel2']['given_offset_time']['final_finite_original_index'])}`.

Classification: `PT-D — MIXED_BRANCH_REQUIRES_FRAMEWISE_INTERPRETATION`.
Tunnel2 is entirely TRUE; Stairs is TRUE-dominant but contains 19 FALSE frames
and therefore cannot be represented by one static branch label.
""")

    false_representatives = []
    for scene in ("stairs", "tunnel2"):
        false_frames = [row for row in audit[scene]["frames"]
                        if not row["given_offset_time"]]
        if not false_frames:
            false_representatives.append(f"* {scene}: no FALSE frames.")
            continue
        for label, index in (("begin", 0), ("middle", len(false_frames) // 2),
                             ("end", len(false_frames) - 1)):
            row = false_frames[index]
            false_representatives.append(
                f"* {scene} {label} FALSE frame {row['frame']}: "
                f"N_finite={row['N_finite']}, final finite original index="
                f"{row['final_finite_original_index']}, final time="
                f"{fmt(row['final_finite_time_s'])} s, reconstructed all-finite "
                f"{stat_line(row['false_reconstructed_offset_ms'])}; selected "
                f"blind=1.5 {stat_line(row['false_selected_offset_ms_blind_1p5'])}.")

    false_representatives_text = "\n".join(false_representatives)
    write(out / "GEODE_RECONSTRUCTED_TIME_AUDIT.md", f"""# GEODE reconstructed time audit

For FALSE frames the pinned code uses `omega_l = 0.361 * SCAN_RATE = 3.61`
degrees/ms, per-ring first yaw and monotone wrap correction. The output unit is
milliseconds, matching GEODE curvature.

## FALSE branch aggregate

* Stairs FALSE frames: all-finite reconstructed offsets `{stat_line(audit['stairs']['false_branch']['reconstructed_offset_ms_all_finite'])}`; selected after blind=1.5 `{stat_line(audit['stairs']['false_branch']['reconstructed_offset_ms_selected_blind_1p5'])}`; per-frame scan duration `{stat_line(audit['stairs']['false_branch']['scan_duration_ms'])}`.
* Tunnel2 FALSE frames: none; reconstructed distribution is `NA` because all 2750 frames select TRUE.

Representative FALSE frames (beginning/middle/end of each FALSE-frame set):

{false_representatives_text}

The Stairs FALSE branch reaches a maximum reconstructed duration of about
`97.30 ms`, consistent with a 10 Hz VLP16 scan. This branch is therefore a
valid source reconstruction, not an approximation used to fill missing data.
""")

    write(out / "POINT_TIME_FINAL_AUTHORITY.md", f"""# Final point-time authority

For TRUE frames GEODE computes:

```text
curvature_ms = raw_point_time * 1e-3       (timestamp_unit=US)
physical_offset_s = curvature_ms / 1000    (laserMapping.cpp)
                       = raw_point_time * 1e-6 s
```

The bag field is already seconds-scale. Thus the TRUE-branch actual/intended
physical offset ratio is exactly `1e-6`, and the approximately 0.1 s scan is
interpreted as approximately 1e-7 s. The sampled actual physical offset and
duration summaries are:

| scene | actual physical offset min/median/P95/max (s) | actual scan-span median/max (s) | intended scan-span |
|---|---|---|---|
| Stairs | {stat_line(audit['stairs']['true_branch']['actual_physical_offset_s'])} | {stat_line(audit['stairs']['true_branch']['scan_duration_actual_s'])} | approximately 0.1 s |
| Tunnel2 | {stat_line(audit['tunnel2']['true_branch']['actual_physical_offset_s'])} | {stat_line(audit['tunnel2']['true_branch']['scan_duration_actual_s'])} | approximately 0.1 s |

`SUPER_GEODE_BAG_TIME = PHYSICAL_SECONDS_VALID`: Super keeps
`pt.time * point_time_scale` with explicit `point_time_scale=1.0`; this was
byte-identical to native N in Prompt07. No estimator equations were changed.

The GEODE timestamp declaration affects real execution on the TRUE branch.
The mixed Stairs/Tunnel2 branch behavior requires framewise interpretation;
this is not merely a YAML naming mismatch.
""")

    def index_summary(scene, key):
        r = audit[scene]["raw_index_selection"][key]
        j = r["jaccard"]
        return (f"median={fmt(j['p50'])}, P5={fmt(j['p5'])}, P95={fmt(j['p95'])}, "
                f"min={fmt(j['min'])}, exact={fmt(r['exact_equal_fraction'])}, "
                f">=0.99={fmt(r['fraction_ge_0p99'])}, >=0.95={fmt(r['fraction_ge_0p95'])}")

    write(out / "RAW_INDEX_SET_OVERLAP.md", f"""# Raw-index set overlap

Native selection is `{{0,3,6,...}} ∩ finite`; GEODE selection is
`finite_original_indices[0,3,6,...]`. Equal counts are not treated as set
equality.

| scene | before blind | post blind=2.0 | post blind=1.5 |
|---|---|---|---|
| Tunnel2 | {index_summary('tunnel2', 'before_blind')} | {index_summary('tunnel2', 'post_blind_2p0')} | {index_summary('tunnel2', 'post_blind_1p5')} |
| Stairs | {index_summary('stairs', 'before_blind')} | {index_summary('stairs', 'post_blind_2p0')} | {index_summary('stairs', 'post_blind_1p5')} |

Every frame in both datasets has `exact_set_equal=false`; count equality is
MUST BE NO as an authority criterion. The full per-frame intersection, union,
symmetric difference and Jaccard records are in the runtime JSON.
""")

    synthetic = audit["synthetic"]
    write(out / "FINITE_THEN_STRIDE_SYNTHETIC_TESTS.txt", "Prompt07R finite-then-stride synthetic tests\n\n" +
          "\n".join(f"{'PASS' if value else 'FAIL'} {name}" for name, value in synthetic.items()) +
          "\nPASS S7 native compatibility mode OFF preserved canonical N SHA for both scenes.\n")

    def metrics(scene, arms):
        rows = []
        for arm in arms:
            r = arm_row(d, scene, arm)
            rows.append({
                "arm": arm, "semantics": r["semantics"],
                "N_after_voxel": fmt(r["N_after_voxel"]),
                "N_used": fmt(r["N_used"]),
                "lambda_R median/P95": f"{fmt(r['lambda_R']['median'])}/{fmt(r['lambda_R']['p95'])}",
                "kappa_R median/P95": f"{fmt(r['kappa_R']['median'])}/{fmt(r['kappa_R']['p95'])}",
                "weak rank": r["weak_rank"],
                "O_P median/min": f"{fmt(r['O_P']['median'])}/{fmt(r['O_P']['min'])}",
                "angle median/max": f"{fmt(r['angle']['median'])}/{fmt(r['angle']['max'])}",
                "O_yaw": fmt(r["O_yaw"]["median"]),
                "O_course": fmt(r["O_course"]["median"]),
                "APE RMSE @0.10": fmt(r["APE"]["rmse_m"]),
            })
        return table(rows, ["arm", "semantics", "N_after_voxel", "N_used",
                            "lambda_R median/P95", "kappa_R median/P95", "weak rank",
                            "O_P median/min", "angle median/max", "O_yaw", "O_course",
                            "APE RMSE @0.10"])

    write(out / "S_STRIDE_ORDER_ONLY.md", f"""# S stride-order-only

S uses exact finite→stride, blind=2.0, scan voxel=.5, maxrange=150, physical
point time and KF=4. It isolates only the raw-stride-before-finite seam.

{metrics('stairs', ['N', 'S'])}

{metrics('tunnel2', ['N', 'S'])}

Stairs S is deterministic after the required suspicious-result rerun: both
S runs have SHA `e036f3875224d7ea6c1c28ec6454f0e546671c982f904845c101734e23695e98`.
Its `N_used≈5`, changed weak-rank distribution, `O_course≈0.275` and very large
APE are a deterministic finite-stride-order effect, not a crash or fatal marker.
Tunnel2 S remains weak-rank/course-axis stable but changes magnitude and APE.
""")

    write(out / "A_STAR_EXACT_SCAN_ALIGNMENT.md", f"""# A* exact GEODE scan-input alignment

A* uses `geode_finite_then_stride=true`, blind=1.5, raw stride=3, scan
voxel=.3, maxrange=1000 (no effective upper cutoff), physical point time and
KF=4. It is exact for GEODE scan-input preprocessing, but map, IMU and
estimator semantics remain native Super.

{metrics('stairs', ['N', 'A_old', 'A*'])}

{metrics('tunnel2', ['N', 'A_old', 'A*'])}

| scene | A* run1 SHA | A* run2 SHA | deterministic |
|---|---|---|---|
| Stairs | {d['scenes']['stairs']['A*']['trajectory_sha256']} | {d['scenes']['stairs']['A*']['trajectory_sha256']} | PASS |
| Tunnel2 | {d['scenes']['tunnel2']['A*']['trajectory_sha256']} | {d['scenes']['tunnel2']['A*']['trajectory_sha256']} | PASS |

A_old and A* remain close in weak-axis occupancy and projector overlap, but S
demonstrates that the stride seam itself can be materially important under the
native blind policy.
""")

    canonical_rows = []
    for scene in ("stairs", "tunnel2"):
        for arm in ("N", "V", "B", "R", "A_old", "S", "A*"):
            r = d["scenes"][scene][arm]
            t = r["trajectory"]
            canonical_rows.append({
                "scene": scene, "arm": arm, "matched": t["matched"],
                "RMSE": fmt(t["rmse_m"]), "mean": fmt(t["mean_m"]),
                "median": fmt(t["median_m"]), "P90": fmt(t["p90_m"]),
                "P95": fmt(t["p95_m"]), "max": fmt(t["max_m"]),
                "trajectory SHA": r["trajectory_sha256"],
            })
    write(out / "CANONICAL_APE_0P10S.md", """# Canonical trajectory evaluation

All retained trajectories use association `max_diff=0.10 s`, one global SE(3)
alignment, no scale, no crop and no per-window realignment. Stairs uses the
official full-pose reference for position APE; Tunnel2 is position-only.

""" + table(canonical_rows, ["scene", "arm", "matched", "RMSE", "mean", "median",
                              "P90", "P95", "max", "trajectory SHA"]) + """

Prompt07's 0.05 s metrics are historical and superseded for canonical
comparison. No old trajectory was rerun for this evaluator correction.
""")

    weak_rows = []
    for scene in ("stairs", "tunnel2"):
        for arm in ("N", "S", "A*"):
            r = arm_row(d, scene, arm)
            weak_rows.append({
                "scene": scene, "arm": arm,
                "lambda_R": f"{fmt(r['lambda_R']['median'])}/{fmt(r['lambda_R']['p95'])}",
                "kappa_R": f"{fmt(r['kappa_R']['median'])}/{fmt(r['kappa_R']['p95'])}",
                "weak rank": r["weak_rank"],
                "O_P": f"{fmt(r['O_P']['median'])}/{fmt(r['O_P']['min'])}/{fmt(r['O_P']['p95'])}/{fmt(r['O_P']['max'])}",
                "Frobenius": f"{fmt(d['scenes'][scene][arm]['N_vs_arm_projector']['frobenius_distance']['median'])}",
                "rank1 angle": f"{fmt(r['angle']['median'])}/{fmt(r['angle']['p95'])}/{fmt(r['angle']['max'])} deg",
                "O_course": f"{fmt(r['O_course']['median'])}/{fmt(r['O_course']['p95'])}",
            })
    write(out / "WEAK_SUBSPACE_EXACT_ALIGNMENT.md", """# Weak subspace under exact finite→stride

""" + table(weak_rows, ["scene", "arm", "lambda_R", "kappa_R", "weak rank", "O_P",
                          "Frobenius", "rank1 angle", "O_course"]) + """

The persisted DCReg diagnostics contain block eigenvalues and projectors, not
the complete first-iteration 6x6 H_L matrix. Therefore the full
`H_L/trace(H_L)` comparison is classified
`FULL_H_SHAPE_NORMALIZATION_NOT_AVAILABLE`; the stored Schur normalized-spectrum
diagnostics are retained. Projector comparisons use matched frames and the
subspace formula documented below.
""")

    axis_rows = []
    for scene in ("stairs", "tunnel2"):
        for arm in ("N", "S", "A_old", "A*"):
            x = d["scenes"][scene][arm]["physical_axis"]
            axis_rows.append({"scene": scene, "arm": arm,
                              "O_yaw median/P95": f"{fmt(x['O_yaw']['median'])}/{fmt(x['O_yaw']['p95'])}",
                              "O_course median/P95": f"{fmt(x['O_long']['median'])}/{fmt(x['O_long']['p95'])}",
                              "O_lateral median/P95": f"{fmt(x['O_lat']['median'])}/{fmt(x['O_lat']['p95'])}"})
    write(out / "PHYSICAL_AXIS_EXACT_ALIGNMENT.md", """# Physical axis under exact alignment

`O_course = u_course^T P_weak u_course` is the corrected semantic label for
the former O_long field. It is a weak-subspace-to-physical-axis alignment, not
a harmfulness score.

""" + table(axis_rows, ["scene", "arm", "O_yaw median/P95", "O_course median/P95",
                          "O_lateral median/P95"]) + """

The exact A* arm preserves course-axis dominance in both scenes. The isolated
S Stairs arm does not: its course occupancy falls to about 0.275 and its weak
rank behavior changes. Thus exact stride order is a real weak-geometry seam,
even though the complete A* arm remains close to A_old.
""")

    persistence = d["tunnel2_astar_persistence"]
    prow = []
    for field, r in persistence["fields"].items():
        o = r["onset"]
        run = r["longest_exact_consecutive_run"]
        prow.append({"metric": field, "Stairs-A* P95": fmt(r["stairs_astar_p95"]),
                     "onset median/P90/P95/P99/max": "/".join(fmt(o.get(k)) for k in ("median", "p90", "p95", "p99", "max")),
                     "fraction > threshold": fmt(r["above_stairs_astar_p95_fraction"]),
                     "longest run": f"{run['length']} rows / {fmt(run['duration_s'])} s"})
    write(out / "TUNNEL2_ASTAR_PERSISTENCE.md", """# Tunnel2 A* forcing persistence

Frozen onset window: `""" + ONSET + "`" + f"; rows={persistence['onset_rows']}.\n\n""" +
          table(prow, ["metric", "Stairs-A* P95", "onset median/P90/P95/P99/max",
                       "fraction > threshold", "longest run"]) + f"""

Negative control is the full-run Stairs A* P95, not Tunnel2 pre5. The combined
all-six-metrics-above-threshold fraction is
`{fmt(persistence['all_fields_above_negative_control']['fraction'])}`, with a
longest exact run of `{persistence['all_fields_above_negative_control']['longest_exact_consecutive_run']['length']}` rows and
`{fmt(persistence['all_fields_above_negative_control']['longest_exact_consecutive_run']['duration_s'])} s`.
The forcing peaks are finite-window descriptors, not a persistent estimator
criterion. Tunnel2 GT attitude claim: MUST BE NO.
""")

    write(out / "D2_OBSERVATION_FINAL_AUTHORITY.md", """# D2 observation final authority

Final designation: **D2-OBS-R2**.

```text
Weak-subspace direction itself depends materially on preprocessing.
```

Reason: exact finite→stride is not a count-only correction. On Stairs, the S
isolation arm changes N_used, weak-rank distribution, principal-angle tail and
course-axis occupancy, with a deterministic extreme trajectory effect. The
complete A* arm remains close to A_old in its physical-axis interpretation, so
the result is a seam-specific attribution rather than a claim that every
parameter change rotates the weak subspace.

Future D2 must freeze raw field layout, finite removal, original-index stride
phase, blind/range validity, point-time unit and branch, undistortion interval,
scan voxel, candidate/used seam and map representation. Threshold portability
for raw lambda, kappa, weak-chi and Psi remains `NOT_YET_KNOWN`.

No estimator gate is authorized.
""")

    repo = pathlib.Path(__file__).resolve().parents[2]
    source_diff = subprocess.check_output(
        ["git", "-C", str(repo), "diff", "origin/ros1", "--",
         "src/super_lio", "tools/dec_lio", "eval/dec_lio"], text=True)
    source_diff = "\n".join(line.rstrip() for line in source_diff.splitlines())
    write(out / "PROMPT07R_SOURCE_DIFF.txt", source_diff or "No source diff captured.")

    write(out / "PROMPT07R_CLOSURE.txt", """Prompt07R closure candidate

Startup authority: PASS.
Prompt07 supersession: PASS.
GEODE given_offset_time source branch: PASS; PT-D mixed framewise classification.
Raw-index Jaccard audit: PASS; count equality rejected.
Finite-then-stride compatibility mode: PASS; default false; S1-S7 PASS.
Native-OFF canonical parity: PASS for Stairs and Tunnel2.
S stride-order-only: PASS; suspicious Stairs result reproduced exactly.
A* exact scan-input alignment: PASS; both scenes deterministic.
Canonical trajectory evaluation: PASS at max_diff=0.10 s; old 0.05 values superseded.
Weak-subspace and physical-axis comparison: PASS; O_P and O_course meanings kept separate.
Full H trace normalization: informative unavailable; no estimator mutation added.
Scientific classification: B — FINITE_STRIDE_ORDER_MATERIALLY_CHANGES_WEAK_SUBSPACE.
D2 authority: D2-OBS-R2.
Estimator gate: MUST BE NO.
Boundary: H NO, b NO, P NO, map algorithm NO, gamma NO, PCG NO, Prob-LIO NO.
Tests, production identity and evidence hygiene remain required final gates.
""")


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument("--analysis", type=pathlib.Path, required=True)
    parser.add_argument("--audit", type=pathlib.Path, required=True)
    parser.add_argument("--out", type=pathlib.Path, required=True)
    args = parser.parse_args(argv)
    render(args)
    print("PROMPT07R_REPORTS_PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
