# Dec-LIO roadmap authority

This file is the single roadmap authority for Dec-LIO. Prompt00R's
computational closure remains valid; Prompt01 corrects only its roadmap
semantics and extends D1 with shadow-only characterization.

| ID | Scope | Status | Authority / exit condition |
|---|---|---|---|
| D0 | Native ROS1 Super-LIO baseline: Bridge01 and Stairs Alpha offline baselines | CLOSED | Prompt00R Bridge authority plus Prompt01 Stairs repeatability |
| D1 | DCReg LiDAR-only 6DoF shadow degeneracy characterization | CLOSED | Prompt01 OFF/ON parity, runtime summaries, and synthetic/reference tests |
| D2 | DCReg + paired SA-style LiDAR information gate | PLANNED | Owner review after D1 spectra and persistence evidence |
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
complements, spectra, condition diagnostics, basis alignment, and weak-axis
reports. It is shadow-only, defaults OFF, and cannot modify `H_L`, `b_L`, the
native estimator, or its trajectory. Factorization failure is represented as
`valid=false` and fail-open, never as an authoritative all-degenerate command.
Prompt01 runtime evidence is in `evidence/dec_lio/d1_runtime_bridge.txt` and
`evidence/dec_lio/d1_runtime_stairs.txt`; the machine-readable summaries stay
under `/home/lc/dec_lio/runtime/prompt01/`.

## Explicitly deferred

D2 information gating, D3 PCG/preconditioning, D4 combined Dec-LIO, and D5
Prob-LIO remain unimplemented or unauthorized after Prompt01.
