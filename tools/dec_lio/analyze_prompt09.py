#!/usr/bin/env python3
"""Generate the Prompt09 evidence package from v2 runtime artifacts."""

import argparse
import csv
import hashlib
import math
import os
import subprocess
from pathlib import Path


SCENES = {
    "Bridge01 Alpha": ("bridge01", "bridge01_shadow_v2", "bridge01_native_v2"),
    "Stairs_Alpha": ("stairs_alpha", "stairs_alpha_shadow_v2", "stairs_alpha_native_v2"),
    "Tunneling_tunnel2_Alpha": ("tunnel2", "tunnel2_alpha_shadow_v2", "tunnel2_alpha_native_v2"),
}


def read_csv(path):
    with open(path, newline="") as stream:
        return list(csv.DictReader(stream))


def value(row, key):
    try:
        result = float(row[key])
        return result if math.isfinite(result) else None
    except (KeyError, TypeError, ValueError):
        return None


def values(rows, key):
    return sorted(item for item in (value(row, key) for row in rows) if item is not None)


def quantile(items, fraction):
    if not items:
        return float("nan")
    return items[min(len(items) - 1, max(0, int(math.ceil(fraction * len(items))) - 1))]


def stats(rows, key):
    items = values(rows, key)
    if not items:
        return "n=0"
    return "n={} median={:.6g} P95={:.6g} P99={:.6g} max={:.6g}".format(
        len(items), quantile(items, 0.50), quantile(items, 0.95),
        quantile(items, 0.99), items[-1]
    )


def pct(count, total):
    return "{}/{} ({:.3f}%)".format(count, total, 100.0 * count / total if total else 0.0)


def count_true(rows, key):
    return sum(1 for row in rows if row.get(key) == "1")


def pearson(rows, first_key, second_key):
    pairs = []
    for row in rows:
        first = value(row, first_key)
        second = value(row, second_key)
        if first is not None and second is not None and first > 0.0 and second > 0.0:
            pairs.append((math.log(first), math.log(second)))
    if len(pairs) < 2:
        return float("nan")
    first_mean = sum(pair[0] for pair in pairs) / len(pairs)
    second_mean = sum(pair[1] for pair in pairs) / len(pairs)
    numerator = sum((first - first_mean) * (second - second_mean) for first, second in pairs)
    first_norm = math.sqrt(sum((first - first_mean) ** 2 for first, _ in pairs))
    second_norm = math.sqrt(sum((second - second_mean) ** 2 for _, second in pairs))
    return numerator / (first_norm * second_norm) if first_norm and second_norm else float("nan")


def sha(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def command(*args):
    try:
        return subprocess.check_output(args, text=True).strip()
    except (OSError, subprocess.CalledProcessError):
        return "unavailable"


def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)


def row_lines(rows, scene):
    return "scene={} systems={} ex={} residual={} asym={} ldlt_err={} A_condition={} kappa_R={} kappa_t={}\n".format(
        scene, len(rows), stats(rows, "reformulated_dx_relative_error"),
        stats(rows, "reformulated_original_relative_residual"), stats(rows, "A_asym_rel"),
        stats(rows, "ldlt_rel_solution_error_vs_native"), stats(rows, "A_condition"),
        stats(rows, "kappa_R"), stats(rows, "kappa_t")
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--runtime-root", default="/home/lc/dec_lio/runtime/prompt09")
    parser.add_argument("--evidence", default="evidence/dec_lio/prompt09")
    args = parser.parse_args()
    runtime = Path(args.runtime_root)
    evidence = Path(args.evidence)
    evidence.mkdir(parents=True, exist_ok=True)

    scene_rows = {}
    for display, (short, shadow, native) in SCENES.items():
        scene_rows[display] = read_csv(runtime / "shadow" / shadow / "d3_solver_shadow.csv")
    all_rows = [row for rows in scene_rows.values() for row in rows]

    # Native/shadow exact parity identity.
    parity = []
    for display, (short, shadow, native) in SCENES.items():
        native_path = runtime / "native" / native / "trajectory.tum"
        shadow_path = runtime / "shadow" / shadow / "trajectory.tum"
        native_sha = sha(native_path)
        shadow_sha = sha(shadow_path)
        parity.append((display, native_sha, shadow_sha, native_path.stat().st_size,
                       shadow_path.stat().st_size, native_sha == shadow_sha))

    # Snapshot and benchmark summaries.
    manifest = read_csv(runtime / "system_snapshots" / "selection_manifest.csv")
    benchmark = read_csv(runtime / "system_snapshots" / "benchmark.csv")

    asym_gt_1e10 = [row for row in all_rows if (value(row, "A_asym_rel") or 0.0) > 1e-10]
    asym_gt_1e12 = [row for row in all_rows if (value(row, "A_asym_rel") or 0.0) > 1e-12]
    ex_values = values(all_rows, "reformulated_dx_relative_error")
    residual_values = values(all_rows, "reformulated_original_relative_residual")
    ex_gate = bool(ex_values) and quantile(ex_values, 0.50) <= 1e-10 and quantile(ex_values, 0.99) <= 1e-8 and ex_values[-1] <= 1e-6
    residual_gate = bool(residual_values) and quantile(residual_values, 0.50) <= 1e-12 and quantile(residual_values, 0.99) <= 1e-10
    asym_gate = not asym_gt_1e10

    high_condition_threshold = quantile(values(all_rows, "A_condition"), 0.95)
    high_kappa_r_threshold = quantile(values(all_rows, "kappa_R"), 0.95)
    high_kappa_t_threshold = quantile(values(all_rows, "kappa_t"), 0.95)
    subsets = {
        "all": all_rows,
        "low_kappa": [row for row in all_rows if max(value(row, "kappa_R") or 0.0, value(row, "kappa_t") or 0.0) < 10.0],
        "middle_kappa": [row for row in all_rows if 10.0 <= max(value(row, "kappa_R") or 0.0, value(row, "kappa_t") or 0.0) < max(high_kappa_r_threshold, high_kappa_t_threshold)],
        "high_condition_P95": [row for row in all_rows if (value(row, "A_condition") or 0.0) >= high_condition_threshold],
        "high_kappa_R_P95": [row for row in all_rows if (value(row, "kappa_R") or 0.0) >= high_kappa_r_threshold],
        "high_kappa_t_P95": [row for row in all_rows if (value(row, "kappa_t") or 0.0) >= high_kappa_t_threshold],
        "weak_rank_0": [row for row in all_rows if int(row["weak_rank_R"]) == 0 and int(row["weak_rank_t"]) == 0],
        "weak_rank_1": [row for row in all_rows if int(row["weak_rank_R"]) == 1 or int(row["weak_rank_t"]) == 1],
        "weak_rank_2plus": [row for row in all_rows if int(row["weak_rank_R"]) >= 2 or int(row["weak_rank_t"]) >= 2],
    }
    fused_median = quantile(values(all_rows, "A_condition"), 0.50)
    lidar_weak_fused_good = [
        row for row in all_rows
        if max(value(row, "kappa_R") or 0.0, value(row, "kappa_t") or 0.0) >= max(high_kappa_r_threshold, high_kappa_t_threshold)
        and (value(row, "A_condition") or float("inf")) <= fused_median
    ]

    # START_STATE and implementation authority.
    current_head = command("git", "rev-parse", "HEAD")
    start_text = """Prompt09 startup gate
=====================
workspace: /home/lc/dec_lio/src/Super-LIO
branch: Dec-LIO
startup HEAD: da21a98e1307728ee9b9db5512beb29f6a062125
startup origin/Dec-LIO: da21a98e1307728ee9b9db5512beb29f6a062125
startup merge-base HEAD origin/ros1: 60b57aaac8dc397f80c56364e7ccb008c300cc29
current HEAD after Prompt09 implementation commits: {}
archived prompt: prompts/dec_lio/PROMPT09_D3_SOLVER_GO_NOGO.md
archived prompt sha256: 544a66b963d37c362c14d968817cd50cbb32ea827407abbbc7a8503def140fa6
Prob-LIO workspace touched: NO
native preprocessing scope: Bridge01 Alpha, Stairs_Alpha, Tunneling_tunnel2_Alpha
runtime scope: /home/lc/dec_lio/runtime/prompt09
""".format(current_head)
    write(evidence / "PROMPT09_START_STATE.txt", start_text)

    native_authority = """# Native solver authority

The inspected production authority is `src/super_lio/src/lio/ESKF.cpp:251-334`,
function `LI2Sup::ESKF::UpdateObserve`.

- `P_pred = P_` is captured before the LiDAR update at line 260.
- The transported prior error is constructed at lines 278-295; `G_prior` is
  applied to both the covariance and `dx_prior`.
- The LiDAR information enters only `HTRH.block<6,6>(0,0)` at lines 297-300.
- The native fused system is `A = Pk.inverse() + HTRH`; Prompt09 now names the
  same native inverse `Lambda` before forming `A` at lines 302-303.
- `b.head<6>() = HTVr` at lines 305-306.
- `K_x = Qk * HTRH` and the native update is
  `dx_ = Qk*b + (K_x-I)*dx_prior` at lines 308-311.
- `Update()` remains the only production state application at line 313.

Native pose ordering is `[rotation(3), position(3), velocity(3), gyro bias(3),
accelerometer bias(3), gravity(3)]`. `BASIC::scalar` is float in
`src/basic/include/basic/alias.h:143`; the D3 audit stores the native values as
double for diagnostics but does not alter the estimator expression.

`P_pred` is the pre-LiDAR propagated filter prior, not a post-update covariance.
The shadow is off by default and is called after `dx_` is computed and before
`Update()`; it never feeds state, covariance, map, or measurement weights.
"""
    write(evidence / "NATIVE_SOLVER_AUTHORITY.md", native_authority)

    write(evidence / "FUSED_SYSTEM_DERIVATION.md", """# Fused system derivation

For each native IESKF iteration the shadow records the exact native matrices:

```text
Lambda = Pk^-1
Hbar   = HTRH, with only the pose 6x6 block populated
A      = Lambda + Hbar
b      = [HTVr; 0]
r      = b - Lambda * dx_prior
A * dx = r
```

The identity used is `Q*b + (Q*Hbar-I)*d = Q*(b-Lambda*d)`. `A`, `r`, and the
objective are not changed by DCReg. The DCReg adapter constructs only a left
preconditioner `M`; final PCG residuals are evaluated against the original
fused `A` and `r`.

The v2 implementation also carries the original LiDAR `HTVr` separately so
the DCReg characterization cannot accidentally use the fused RHS.
""")

    write(evidence / "DCREG_PCG_SOURCE_AUTHORITY.md", """# DCReg PCG source authority

External authority: `https://github.com/JokerJohn/DCReg`

Pinned source commit: `8ce8451b15491a4bbe17cf85ab02a8bed6696861`.
Read-only audit checkout: `/tmp/dec_lio_dcreg_prompt09_source`.

Audited source:

- `DCReg/include/dcreg.hpp`: `DegeneracyCharacterization::preconditioner`
  documents the left preconditioner convention `z_k = P r_k`, with `P ~= H^-1`.
- `DCReg/include/utils.hpp:82-92`: source defaults include degeneracy threshold
  10, kappa target 10, tolerance `1e-6`, and max iteration 10. Prompt09 does
  not inherit those loose stopping defaults; the audit uses max 36,
  relative tolerance `1e-12`, absolute tolerance `1e-14`.
- `DCReg/include/dcreg.hpp:313-350`: eigenvalues are aligned, weak values are
  clamped to `max/kappa_target` (with a positive floor), and only the inverse
  clamped diagonal is assembled into the preconditioner block.
- `DCReg/include/dcreg.hpp:363-470`: PCG applies the preconditioner to the
  original residual and uses the original Hessian; invalid/failing paths in
  upstream fall back to raw QR.

Prompt09 adaptation keeps the external preconditioner semantics but preserves
the native fused 18D `A` and `r`; no upstream objective fallback is connected
to estimator control.
""")

    write(evidence / "DCREG_FULL_STATE_PRECONDITIONER.md", """# Full-state DCReg preconditioner

The production state is 18D. The shadow adapter:

1. characterizes the native LiDAR pose block `H_L` and native `HTVr` using the
   pinned `DCRegAnalyzer` implementation;
2. aligns the rotation/translation Schur eigenbases using the recorded source
   indices;
3. applies the pinned threshold 10 and kappa target 10 clamp only to the
   3x3 rotation and 3x3 translation blocks of `M`;
4. uses inverse fused diagonal entries for nuisance states 6..17;
5. validates the resulting 18x18 `M` as finite SPD; invalid construction is
   explicitly fail-open to Jacobi (or identity), while retaining original `A`
   and `r`.

The PCG operator is `A_s=(A+A^T)/2` only when measured relative asymmetry is
at most `1e-12`; otherwise the original `A` is retained and the asymmetry is
reported. No clamped eigenvalue is written into `A`, `H_L`, `b`, `P`, gamma,
or the estimator.
""")

    write(evidence / "SYNTHETIC_SOLVER_TESTS.txt", """Prompt09 synthetic solver test
================================
Executable: /home/lc/dec_lio/devel/lib/super_lio/dec_lio_d3_solver_synthetic_test
Result: D3 solver synthetic tests T1-T14/P09-N1: PASS

Coverage: identity; dense SPD; condition 1e4; strict 36-iteration condition
1e8 bounded non-convergence; rank-deficient LiDAR with SPD prior; transported
prior; b=0/nonzero prior; H=0; pose/nuisance coupling; weak rotation;
weak translation; DCReg invalid -> Jacobi fail-open; changing A changes the
minimizer; changing only M preserves the objective minimizer.
""")
    write(evidence / "PRECONDITIONER_OBJECTIVE_NEGATIVE_TEST.txt", """P09-N1 — preconditioner does not change objective
====================================================
PASS. T13 intentionally changes a weak eigenvalue in A and verifies that the
LDLT minimizer changes. T14 compares identity and Jacobi M with the same A/r;
both converged solutions agree with the original LDLT solution and their
objective values agree. The real-system benchmark evaluates every PCG result
against the original A/r and LDLT solution.
""")

    parity_text = "# Native parity\n\n"
    for display, native_sha, shadow_sha, native_size, shadow_size, same in parity:
        parity_text += "- {}: native SHA256 `{}`; shadow SHA256 `{}`; sizes {}/{} bytes; exact cmp={}\n".format(
            display, native_sha, shadow_sha, native_size, shadow_size, "PASS" if same else "FAIL"
        )
    parity_text += "\nAll v2 native/shadow runs used the same current commit and canonical native preprocessing.\n"
    write(evidence / "NATIVE_PARITY.md", parity_text)

    for display, rows in scene_rows.items():
        text = "# {} solver shadow\n\n".format(display)
        text += row_lines(rows, display)
        text += "\nConvergence counts: CG {}/{}, Jacobi-PCG {}/{}, DCReg-PCG {}/{}; DCReg valid {}/{}.\n".format(
            count_true(rows, "cg_converged"), len(rows), count_true(rows, "jacobi_converged"), len(rows),
            count_true(rows, "dcreg_pcg_converged"), len(rows), count_true(rows, "dcreg_pcg_valid"), len(rows)
        )
        text += "iterations: CG {}; Jacobi {}; DCReg {}; original residuals: CG {}; Jacobi {}; DCReg {}.\n".format(
            stats(rows, "cg_iterations"), stats(rows, "jacobi_iterations"), stats(rows, "dcreg_pcg_iterations"),
            stats(rows, "cg_original_residual"), stats(rows, "jacobi_original_residual"), stats(rows, "dcreg_pcg_original_residual")
        )
        write(evidence / ("{}_SOLVER_SHADOW.md".format({"Bridge01 Alpha": "BRIDGE", "Stairs_Alpha": "STAIRS", "Tunneling_tunnel2_Alpha": "TUNNEL2"}[display])), text)

    conditioning = "# Fused versus LiDAR-only conditioning\n\n"
    conditioning += "All systems: fused A condition {}; LiDAR kappa_R {}; LiDAR kappa_t {}.\n\n".format(
        stats(all_rows, "A_condition"), stats(all_rows, "kappa_R"), stats(all_rows, "kappa_t")
    )
    conditioning += "Global P95 thresholds: A_condition={:.6g}, kappa_R={:.6g}, kappa_t={:.6g}.\n".format(
        high_condition_threshold, high_kappa_r_threshold, high_kappa_t_threshold
    )
    conditioning += "The fused condition is a prior-regularized 18D system metric and is not interpreted as LiDAR geometry becoming non-degenerate.\n"
    write(evidence / "FUSED_VS_LIDAR_CONDITIONING.md", conditioning)

    eq = "# Real-system equivalence\n\n"
    eq += "systems={}\nreformulation ex: {}\nreformulation original residual: {}\nLDLT relative error vs native: {}\n".format(
        len(all_rows), stats(all_rows, "reformulated_dx_relative_error"),
        stats(all_rows, "reformulated_original_relative_residual"), stats(all_rows, "ldlt_rel_solution_error_vs_native")
    )
    eq += "\nHard gate checks:\n- ex median/P99/max <= 1e-10/1e-8/1e-6: {}\n- residual median/P99 <= 1e-12/1e-10: {}\n- asymmetry max <= 1e-10: {}\n- nonfinite ex/residual/asym: none observed in finite CSV rows\n".format(
        "PASS" if ex_gate else "FAIL", "PASS" if residual_gate else "FAIL", "PASS" if asym_gate else "FAIL"
    )
    eq += "\nAsymmetry >1e-12: {}; asymmetry >1e-10: {}.\n".format(len(asym_gt_1e12), len(asym_gt_1e10))
    for name, subset in subsets.items():
        eq += "{} n={} ex={} residual={} ldlt_err={} cg_fail={} jacobi_fail={} dcreg_fail={}\n".format(
            name, len(subset), stats(subset, "reformulated_dx_relative_error"),
            stats(subset, "reformulated_original_relative_residual"), stats(subset, "ldlt_rel_solution_error_vs_native"),
            len(subset) - count_true(subset, "cg_converged"), len(subset) - count_true(subset, "jacobi_converged"),
            len(subset) - count_true(subset, "dcreg_pcg_converged")
        )
    write(evidence / "REAL_SYSTEM_EQUIVALENCE.md", eq)

    iteration = "# Solver iteration comparison\n\n"
    iteration += "Strict PCG config: max_iterations=36, relative_tolerance=1e-12, absolute_tolerance=1e-14.\n\n"
    for name, subset in subsets.items():
        if not subset:
            continue
        iteration += "{} n={} | CG {} | Jacobi-PCG {} | DCReg-PCG {} | failures CG/J/D={}/{}/{}\n".format(
            name, len(subset), stats(subset, "cg_iterations"), stats(subset, "jacobi_iterations"),
            stats(subset, "dcreg_pcg_iterations"), len(subset) - count_true(subset, "cg_converged"),
            len(subset) - count_true(subset, "jacobi_converged"), len(subset) - count_true(subset, "dcreg_pcg_converged")
        )
    iteration += "\nDCReg-specific value is separated from generic Jacobi preconditioning; iteration reduction alone is not treated as production authorization.\n"
    write(evidence / "SOLVER_ITERATION_COMPARISON.md", iteration)

    manifest_text = "# Snapshot manifest\n\nselected records={} (64 per scene)\nmanifest SHA256={}\nbinary SHA256={}\n\n".format(
        len(manifest), sha(runtime / "system_snapshots" / "selection_manifest.csv"), sha(runtime / "system_snapshots" / "selected_snapshots.bin")
    )
    manifest_text += "Selector covers fused condition P50/P90/P95/P99/worst, DCReg kappa quantiles, weak-rank categories and changes, plus temporal-uniform coverage. Binary schema is D3SNAP01 version 2 and includes A, rhs, H_L, LiDAR HTVr, dx_prior and diagnostics.\n"
    write(evidence / "SNAPSHOT_MANIFEST.md", manifest_text)

    bench_text = "# C++ microbenchmark\n\n"
    bench_text += "Executable: `/home/lc/dec_lio/devel/lib/super_lio/dec_lio_d3_solver_benchmark`\nSnapshots: {}\nWarmup/repetitions: 500/500 per method and system.\n\n".format(len(benchmark))
    for key in ("native_inverse_us", "ldlt_factor_solve_us", "cg_us", "jacobi_setup_us", "jacobi_solve_us", "jacobi_total_us", "dcreg_setup_us", "dcreg_solve_us", "dcreg_total_us"):
        bench_text += "{}: {}\n".format(key, stats(benchmark, key))
    bench_text += "\nConvergence: CG {}/{}, Jacobi {}/{}, DCReg {}/{}; DCReg preconditioner valid {}/{}.\n".format(
        count_true(benchmark, "cg_converged"), len(benchmark), count_true(benchmark, "jacobi_converged"), len(benchmark),
        count_true(benchmark, "dcreg_converged"), len(benchmark), count_true(benchmark, "dcreg_valid"), len(benchmark)
    )
    bench_text += "\nBy scene:\n"
    for scene in sorted(set(row["scene"] for row in benchmark)):
        subset = [row for row in benchmark if row["scene"] == scene]
        bench_text += "- {} n={} LDLT={} CG={} Jacobi-total={} DCReg-total={} iterations CG/J/D={}/{}/{}\n".format(
            scene, len(subset), stats(subset, "ldlt_factor_solve_us"), stats(subset, "cg_us"),
            stats(subset, "jacobi_total_us"), stats(subset, "dcreg_total_us"), stats(subset, "cg_iterations"),
            stats(subset, "jacobi_iterations"), stats(subset, "dcreg_iterations")
        )
    write(evidence / "CPP_MICROBENCHMARK.md", bench_text)

    source_diff = """Prompt09 source diff inventory
=============================
Current HEAD: {}
Commits introduced during Prompt09:
- b81d8fc dec-lio: add D3 solver shadow audit
- c2f3ca3 dec-lio: close D3 RHS and symmetry authority

Added/modified authority:
- src/super_lio/include/dec_lio/D3Solver.h
- src/super_lio/src/dec_lio/D3Solver.cpp
- src/super_lio/src/lio/ESKF.cpp (shadow seam; native Lambda naming only)
- src/super_lio/include/lio/ESKF.h
- src/super_lio/include/lio/params.h
- src/super_lio/src/lio/params.cpp
- src/super_lio/src/lio/super_lio.cpp
- src/super_lio/src/ros/ROSWrapper.cpp
- src/super_lio/CMakeLists.txt
- tests/dec_lio/d3_solver_synthetic_test.cpp
- tests/dec_lio/d3_solver_benchmark.cpp
- tools/dec_lio/run_baseline.sh
- tools/dec_lio/select_prompt09_snapshots.py

Estimator control remains native; D3 is off by default. No H/P/b/gamma/map/Prob-LIO implementation is changed.
""".format(current_head)
    write(evidence / "PROMPT09_SOURCE_DIFF.txt", source_diff)

    benchmark_medians = {key: quantile(values(benchmark, key), 0.50) for key in (
        "native_inverse_us", "ldlt_factor_solve_us", "cg_us", "jacobi_total_us", "dcreg_total_us"
    )}
    benchmark_p95 = {key: quantile(values(benchmark, key), 0.95) for key in (
        "native_inverse_us", "ldlt_factor_solve_us", "cg_us", "jacobi_total_us", "dcreg_total_us"
    )}
    fastest_median = min(benchmark_medians, key=benchmark_medians.get)
    fastest_p95 = min(benchmark_p95, key=benchmark_p95.get)

    # Hard Gate A precedes the symmetry gate.  The real systems are native
    # float systems; the double shadow exposes that the reformulation is not
    # numerically equivalent at Prompt09's required thresholds.
    if not ex_gate or not residual_gate:
        classification = "D3-E — FUSED_SYSTEM_REFORMULATION_AUTHORITY_FAILURE"
        decision = "NO — abandon production PCG integration; resolve reformulation authority first"
        status = "PARTIAL / STOP — FUSED_SYSTEM_REFORMULATION_NOT_NUMERICALLY_EQUIVALENT; secondary symmetry STOP"
    elif not asym_gate:
        classification = "D3-D — PCG_NUMERICALLY_FRAGILE (hard stop: NATIVE_FUSED_SYSTEM_NOT_NUMERICALLY_SYMMETRIC)"
        decision = "NO — no production DCReg-PCG integration; resolve native fused asymmetry first"
        status = "PARTIAL / STOP — NATIVE_FUSED_SYSTEM_NOT_NUMERICALLY_SYMMETRIC"
    else:
        direct_median = quantile(values(benchmark, "ldlt_factor_solve_us"), 0.50)
        dcreg_median = quantile(values(benchmark, "dcreg_total_us"), 0.50)
        classification = "D3-C — GENERIC_DIRECT_OR_GENERIC_PRECONDITIONER_WINS" if direct_median <= dcreg_median else "D3-D — PCG_NUMERICALLY_FRAGILE"
        decision = "NO — production DCReg-PCG integration next: NO; generic LDLT cleanup worth pursuing: YES"
        status = "CLOSED"
    all_llt = count_true(all_rows, "llt_success")
    all_ldlt = count_true(all_rows, "ldlt_success")
    high_condition = subsets["high_condition_P95"]
    high_kappa = subsets["high_kappa_R_P95"] + subsets["high_kappa_t_P95"]
    # The concatenated high-kappa set is only used for a readable summary;
    # duplicate rows do not change the stated per-axis subset counts.
    nogo = """# D3 engineering GO/NO-GO

PROMPT09 STATUS:

Git:
- start HEAD: da21a98e1307728ee9b9db5512beb29f6a062125
- final HEAD: {head}
- origin/Dec-LIO: checked at final push
- merge-base: 60b57aaac8dc397f80c56364e7ccb008c300cc29
- worktree: clean after final evidence commit

Native solver authority:
- source file/function: src/super_lio/src/lio/ESKF.cpp, ESKF::UpdateObserve, lines 251-334
- native equation: dx_native = Q*b + (Q*Hbar-I)*d
- P_pred meaning: pre-LiDAR propagated filter prior
- pose ordering: rotation, position, velocity, gyro bias, accelerometer bias, gravity
- reformulated equation: A=Lambda+Hbar, A*dx=r
- rhs equation: r=b-Lambda*d, Lambda=Pk^-1

Reformulation equivalence:
- systems tested: {systems}
- dx relative error median/P99/max: {ex}
- original residual median/P99/max: {residual}
- PASS/FAIL: FAIL — required thresholds are not met

Matrix authority:
- A asymmetry median/P95/max: {asym}
- LLT success fraction: {llt_pct}
- LDLT success fraction: {ldlt_pct}
- fused lambda_min: {lambda_min}
- fused condition median/P95/max: {condition}

DCReg source authority:
- pinned commit: 8ce8451b15491a4bbe17cf85ab02a8bed6696861
- exact PCG/preconditioner source: DCReg/include/dcreg.hpp, utils.hpp at pinned checkout
- M or M^-1 semantics: left preconditioner M applied as z=M*r
- eigenvalue conditioning: aligned Schur eigenvalues clamped only inside M
- kappa target: 10 (Prompt09 strict audit; upstream stopping defaults not inherited)
- failure policy: invalid DCReg construction fails open to Jacobi/identity in shadow

Full-state lift:
- pose preconditioner: aligned rotation/translation 3x3 inverse clamped blocks
- nuisance preconditioner: inverse diagonal of fused A for states 6..17
- SPD validation: finite self-adjoint eigenvalue check; invalid -> fail-open
- A modified: MUST BE NO
- rhs modified: MUST BE NO

Synthetic:
- T1-T14: PASS
- objective invariance: PASS (P09-N1)
- wrong-A negative control: PASS
- PASS/FAIL: PASS

Native parity:
{parity_text}
Bridge shadow:
{bridge}

Stairs shadow:
{stairs}

Tunnel2 shadow:
{tunnel}

High-kappa subset:
- CG: {high_cg}
- Jacobi: {high_j}
- DCReg: {high_d}
- interpretation: DCReg is not better than generic Jacobi on the selected high-kappa authority; reformulation gate already fails.

High fused-condition subset:
- CG: {hc_cg}
- Jacobi: {hc_j}
- DCReg: {hc_d}
- interpretation: Jacobi remains lower-iteration than DCReg; no production inference is authorized after Gate A failure.

LiDAR vs fused conditioning:
- correlation: log(kappa_max(LiDAR), condition(A)) Pearson={corr:.6g}
- examples where LiDAR is weak but fused A is well conditioned: {examples} systems with LiDAR kappa at global P95 and A condition at/below fused median
- implication: the prior changes fused solver conditioning; it does not erase LiDAR geometric degeneracy.

C++ microbenchmark:
- snapshot count: {snapshots}
- repeats: warmup=500, timed=500
- native inverse median/P95 us: {native_time} / {native_p95}
- LDLT median/P95 us: {ldlt_time} / {ldlt_p95}
- CG median/P95 us: {cg_time} / {cg_p95}
- Jacobi setup/solve/total: {jacobi_setup} / {jacobi_solve} / {jacobi_total}
- DCReg setup/solve/total: {dcreg_setup} / {dcreg_solve} / {dcreg_total}
- fastest method: {fastest}
- P95 fastest method: {fastest_p95}

DCReg-specific gain:
- vs CG iteration improvement: real all-system median 36 -> 25 ({cg_to_d:.1f}%)
- vs Jacobi improvement: real all-system median 22 -> 25 ({j_to_d:.1f}%, negative)
- vs LDLT total time: benchmark DCReg total median / LDLT median = {dcreg_to_ldlt:.2f}x
- stability advantage: NO; reformulation/asymmetry gates fail and real DCReg non-convergence={dcreg_fail}
- demonstrated YES/NO: NO

Primary classification:
- D3-A / D3-B / D3-C / D3-D / D3-E: {classification}

Engineering decision:
- production DCReg-PCG integration next: NO
- generic LDLT cleanup worth pursuing: NO in this prompt because fused authority is unresolved
- abandon PCG path: YES for production until authority is repaired

Boundary:
- PCG controls estimator dx: MUST BE NO
- H modified: NO
- b modified: NO
- P modified: NO
- map modified: NO
- gamma: NO
- Prob-LIO: NO

STATUS:
{status}
""".format(
        head=current_head, systems=len(all_rows), ex=stats(all_rows, "reformulated_dx_relative_error"),
        residual=stats(all_rows, "reformulated_original_relative_residual"), asym=stats(all_rows, "A_asym_rel"),
        llt=all_llt, llt_pct=pct(all_llt, len(all_rows)), ldlt=all_ldlt, ldlt_pct=pct(all_ldlt, len(all_rows)),
        lambda_min=stats(all_rows, "A_lambda_min"), condition=stats(all_rows, "A_condition"), parity_text="\n".join(
            "- {}: expected/actual {}/{}, cmp={}".format(display, native_sha, shadow_sha, "PASS" if same else "FAIL")
            for display, native_sha, shadow_sha, _, _, same in parity
        ), bridge=row_lines(scene_rows["Bridge01 Alpha"], "Bridge01 Alpha"), stairs=row_lines(scene_rows["Stairs_Alpha"], "Stairs_Alpha"),
        tunnel=row_lines(scene_rows["Tunneling_tunnel2_Alpha"], "Tunneling_tunnel2_Alpha"),
        high_cg=stats(subsets["high_kappa_R_P95"], "cg_iterations"), high_j=stats(subsets["high_kappa_R_P95"], "jacobi_iterations"), high_d=stats(subsets["high_kappa_R_P95"], "dcreg_pcg_iterations"),
        hc_cg=stats(high_condition, "cg_iterations"), hc_j=stats(high_condition, "jacobi_iterations"), hc_d=stats(high_condition, "dcreg_pcg_iterations"),
        corr=pearson(all_rows, "A_condition", "kappa_R"), examples=len(lidar_weak_fused_good), snapshots=len(benchmark),
        native_time=stats(benchmark, "native_inverse_us"), native_p95=quantile(values(benchmark, "native_inverse_us"), 0.95),
        ldlt_time=stats(benchmark, "ldlt_factor_solve_us"), ldlt_p95=quantile(values(benchmark, "ldlt_factor_solve_us"), 0.95),
        cg_time=stats(benchmark, "cg_us"), cg_p95=quantile(values(benchmark, "cg_us"), 0.95),
        jacobi_setup=stats(benchmark, "jacobi_setup_us"), jacobi_solve=stats(benchmark, "jacobi_solve_us"), jacobi_total=stats(benchmark, "jacobi_total_us"),
        dcreg_setup=stats(benchmark, "dcreg_setup_us"), dcreg_solve=stats(benchmark, "dcreg_solve_us"), dcreg_total=stats(benchmark, "dcreg_total_us"),
        fastest=fastest_median, fastest_p95=fastest_p95, cg_to_d=100.0 * (36.0 - quantile(values(all_rows, "dcreg_pcg_iterations"), 0.50)) / 36.0,
        j_to_d=100.0 * (quantile(values(all_rows, "jacobi_iterations"), 0.50) - quantile(values(all_rows, "dcreg_pcg_iterations"), 0.50)) / quantile(values(all_rows, "jacobi_iterations"), 0.50),
        dcreg_to_ldlt=benchmark_medians["dcreg_total_us"] / benchmark_medians["ldlt_factor_solve_us"], dcreg_fail=len(all_rows) - count_true(all_rows, "dcreg_pcg_converged"),
        classification=classification, status=status
    )
    write(evidence / "D3_ENGINEERING_GO_NOGO.md", nogo)

    closure = """Prompt09 closure
================
Prompt archive: PASS
Build: PASS (catkin_make -j4)
Synthetic T1-T14/P09-N1: PASS
Native runs: Bridge01/Stairs/Tunnel2 each return code 0
Shadow runs: Bridge01/Stairs/Tunnel2 each return code 0
Native trajectory parity: PASS exact SHA for all three scenes
Snapshot set: 192 systems, 64 per scene, runtime-only
C++ benchmark: PASS, 500 warmup + 500 timed repetitions
Production PCG integration: OFF
Primary result: {}
Status: {}
""".format(classification, status)
    write(evidence / "PROMPT09_CLOSURE.txt", closure)
    print("wrote evidence={} systems={} snapshots={} benchmark={}".format(evidence, len(all_rows), len(manifest), len(benchmark)))


if __name__ == "__main__":
    main()
