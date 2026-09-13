# Dec-LIO roadmap authority

This file is the single roadmap authority for Dec-LIO. Prompt00R's
computational closure remains valid; Prompt01 corrects only its roadmap
semantics and extends D1 with shadow-only characterization.

| ID | Scope | Status | Authority / exit condition |
|---|---|---|---|
| D0 | Native ROS1 Super-LIO baseline: Bridge01 and Stairs Alpha offline baselines | CLOSED | Prompt00R Bridge authority plus Prompt01 Stairs repeatability |
| D1 | DCReg LiDAR-only 6DoF shadow degeneracy characterization | CLOSED — CORRECTED | Prompt01 OFF/ON parity plus Prompt02 used-residual correction, raw-EVD projector stability, and local-error evidence |
| D2 | DCReg + paired SA-style LiDAR information gate | PLANNED — DESIGN EVIDENCE AVAILABLE | Prompt02 recommends gate inputs and frame-freeze experiment; no estimator gate is implemented |
| D3 | PCG solver-equivalence and DCReg numerical preconditioner | PLANNED | Exact native solve-equivalence evidence |
| D4 | Combined Dec-LIO (D1 characterization + D2 gate + D3 PCG) | PLANNED | Separate approved implementation authority |
| D5 | Prob-LIO extension | NOT AUTHORIZED | Requires explicit future Owner authority |

## D0 authority

Bridge01 Alpha's native trajectory SHA is immutable:

```text
6b5dc117b86a1ff908261a2e3f0627c0e49a96d1fa2bfc0f45f95545bc5e2203
```

No DCReg, SA gate, gamma scaling, PCG, preconditioner, or Prob-LIO estimator
is part of D0.

## D1 authority

D1 observes only the native LiDAR-only 6x6 `H_L,b_L` reduction after the
thread-local reduction and before native delivery. It computes Schur
complements, spectra, condition diagnostics, raw-EVD weak projectors, and
secondary axis-alignment reports. It is shadow-only, defaults OFF, and cannot
modify `H_L`, `b_L`, the native estimator, or its trajectory. Factorization
failure is represented as `valid=false` and fail-open, never as an
authoritative all-degenerate command. The first valid non-converged iteration
is the frame authority. Prompt01 evidence is in
`evidence/dec_lio/prompt01/`; Prompt02 evidence is in
`evidence/dec_lio/prompt02/`, with machine-readable runtime outputs under
`/home/lc/dec_lio/runtime/prompt02/`.

## D2 design boundary

D1 describes LiDAR geometric conditioning. D2 would decide how much LiDAR
information should influence the tightly coupled estimator. These are related
but non-identical problems: geometric weakness is not estimator failure. The
Prompt02 evidence does not authorize H/b scaling, gamma application, PCG,
preconditioning, or frame freezing in the estimator.

## Explicitly deferred

D2 information gating, D3 PCG/preconditioning, D4 combined Dec-LIO, and D5
Prob-LIO remain unimplemented or unauthorized after Prompt01.
