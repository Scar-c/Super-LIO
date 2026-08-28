# Round13 D_VISUAL_APPLY production-path audit

Executor handoff: GTP → agent-ds. Takeover HEAD:
`19eeefebb07463004e1d9bd6cfd9cdc83dfd8ad0`.

## Decision

The existing ESKF update primitive implements the required state and covariance
posterior semantics, but the production Apply call is unreachable under the
normalized D-family profile. Prompt61 therefore stops before instrumentation,
full playback, or ATE evaluation:

```text
ROUND13_STOPPED_FOR_OWNER
first failed gate = A-T3 production Apply-path reachability
```

Changing the policy predicate or moving the call would be a production
scheduler/placement change. Prompt61 explicitly prohibits silently making that
change and prohibits resurrecting the reverted Round13 placement commits.

## Existing Apply path

| Question | Source-grounded answer |
|---|---|
| Entry point | `SuperLIO::stateProcess`, V-4A/B block in `super_lio.cpp` |
| Effective gate | `g_lio_v4_apply && g_lio_camera_epoch && policy == PARTIAL && g_lio_v2_enabled && g_lio_v0_enabled` |
| H/b input | Existing `runVisualResidual`; accumulated 6x6/6x1 photometric normal equations |
| Solver | `ESKF::UpdateObserveFromPrior` → iterative information-form `UpdateObserveImpl` |
| Prior state | `kf_->GetSysState()` captured immediately after the preceding `Observe()` |
| Prior covariance | `kf_->GetCov()` captured at the same boundary |
| State update | `Qk*b + (K_x-I)*dx_prior`, committed by `ESKF::Update()` |
| Covariance update | `P_=Qk`, reset Jacobian, then explicit symmetrization |
| Commit semantics | The ESKF object receives both posterior x and P; a `PosteriorSnapshot` is also returned |
| Cadence | Located after `stateProcess()` LiDAR Observe, but only for the legacy PARTIAL policy |

The latest-posterior construction and x/P math are internally coherent when
the block executes. Existing `v4c_test` and `test_s1_prior` prove zero-information
identity, informative state/covariance update, covariance contraction, and
finite posterior behavior at the ESKF public seam.

## Normalized-profile conflict

`D_VISUAL_APPLY` inherits `D_VISUAL_SHADOW`: D_CORRECTED scheduling, full raw
scan at scan end, one Observe per raw scan, and Visual apply ON. The runner
resolves that policy to `imu_fullscan`; the only production Apply block requires
`LidarUpdatePolicy::PARTIAL`.

The accepted full-scan Shadow measurement call is also guarded by
`!g_lio_v4_apply`. Enabling Apply therefore disables that local H/b call without
enabling the PARTIAL-only solver. The normalized profile would have solver,
state commit, and covariance commit counts all equal to zero. This is
`VISUAL_APPLY_SOLVER_INACTIVE_FAIL`, not a useful full-bag experiment.

Historical evidence independently records this exact gap: Round11AB says Apply
is PARTIAL-only and is not connected to the active D-family `imu_fullscan`
owner. The earlier predicate extension in `33c1b3d` and subsequent placement
changes were forward-reverted and are frozen by Prompt61.

## TDD and diagnostic gate

```text
A-T1 profile inheritance: PASS
A-T2 protected semantic delta: PASS (visual_state_apply only)
A-T3 nonzero H/b reaches real production Apply path: FAIL
A-T4 production state change: NOT REACHED (A-T3 hard stop)
A-T5 production covariance update: NOT REACHED (A-T3 hard stop)
A-T6 zero-information ESKF identity: PASS (existing v4c/S-1 tests)
A-T7 production nonfinite rejection: NOT REACHED
A-T8 LiDAR ownership: NOT EXECUTED; no source change made
A-T9 one Observe/raw scan: NOT EXECUTED; no source change made
A-T10 Apply-OFF Shadow parity: PASS from canonical Prompt60 control
```

The deterministic minimal feedback loop resolves both normalized profiles and
checks the production gate. It exits 2 with A-T3 FAIL: PARTIAL-only Apply versus
D `imu_fullscan`.

Ranked hypotheses were: (1) production Apply gate excludes the D policy —
confirmed; (2) the runner overrides the normalized full-scan policy — rejected;
(3) the Apply flag is overwritten before execution — rejected by the resolver
and rosparam contract, but moot because the policy gate blocks entry first.

## Owner decision required

The next step requires explicit architecture authority for where the
full-scan/camera-event Visual posterior update belongs and how it interacts
with exact-once camera payload ownership. No production code, experiment,
trajectory, evaluator output, or canonical ledger row was created here.
