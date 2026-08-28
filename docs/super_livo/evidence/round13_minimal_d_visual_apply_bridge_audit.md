# Round13 minimal D Visual-Apply bridge audit

Starting HEAD: `99c3c88996d0191bb41b08251d2fdb0977f9ac80`.

## Result

```text
B-T1 = RED (reproduced)
CAMERA_PAYLOAD_CHANGE_REQUIRED = YES
ROUND13_MINIMAL_BRIDGE_NOT_POSSIBLE
```

No production bridge, instrumentation, full run, or evaluation was performed.

## Event-path evidence

The normalized D camera action is `IMU_ONLY`. In
`ROSWrapper::sync_fullscan_camera_epoch`, that action obtains `t_c`, drains IMU,
then calls `accountFullscanCamera(false)` before returning the measure group.
`accountFullscanCamera` immediately calls `camera_buffer_.popOldest()`.

`SuperLIO::process` then dispatches `IMU_ONLY` exclusively to
`statePropagateOnly`, which propagates to `measures_.epoch_ts` and commits the
propagation boundary. It does not call the Visual producer, lifecycle, query,
residual, or Apply primitive. The camera payload for that epoch has already
been removed before this function begins.

The accepted Prompt60 H/b is produced elsewhere: inside the `need_converge`
branch of the full LiDAR `Observe()` ESKF callback. This is a later full-scan
geometry-update context, not the camera `IMU_ONLY` event. Prompt60 runtime
accounting corroborates the distinction:

```text
camera epochs = 1966
frontend entered from full-scan convergence = 3260
non-null measurement frames = 823
H/b accumulation frames = 823
```

Thus “the existing D Visual measurement event” is not currently a co-located
camera-event seam capable of satisfying the new normative bridge contract.

## Why each candidate bridge violates Prompt62

| Candidate | Conflict |
|---|---|
| Change `PARTIAL` gate to accept `imu_fullscan` | Executes the legacy post-LiDAR block, which recomputes H/b at scan end rather than applying the accepted camera-epoch H/b |
| Capture callback H/b and apply after LiDAR `Observe()` | Explicitly stores H/b and applies later at scan end; forbidden by §§7–8 |
| Invoke `UpdateObserveFromPrior` inside the LiDAR ESKF observation callback | Nested mutation of the same ESKF while its LiDAR update is in progress; violates the sequential posterior boundary and cannot provide a committed LiDAR posterior prior |
| Apply from `statePropagateOnly()` | Requires retaining the camera payload and moving/adding producer/query/residual execution to that event; forbidden by §§12, 17, 18, and 46 |

The old quarantined commits attempted broader gate, payload, and placement
changes. None was restored or copied.

## Pre-fix RED feedback loop

A deterministic source-contract check verifies all five load-bearing facts:

```text
IMU_ONLY pops camera before returning: CONFIRMED
camera handler popOldest: CONFIRMED
IMU_ONLY has no visual measurement: CONFIRMED
accepted Shadow H/b is inside LiDAR Observe callback: CONFIRMED
Apply primitive is PARTIAL-only: CONFIRMED
B-T1 minimal same-camera-epoch bridge: RED
CAMERA_PAYLOAD_CHANGE_REQUIRED: YES
```

The command exits 2. Removing any one of the prohibited constraints would open
a design branch; with all Prompt62 constraints retained, no GREEN implementation
exists.

## Bridge TDD disposition

```text
B-T1  RED reproduced
B-T2  inherited Shadow non-apply remains PASS; no code changed
B-T3  BLOCKED — no permissible same-event Apply seam
B-T4  FAIL/BLOCKED — current H/b and camera prior are not co-located
B-T5  BLOCKED
B-T6  BLOCKED (ESKF primitive itself remains independently proven)
B-T7  BLOCKED
B-T8  BLOCKED
B-T9  BLOCKED
B-T10 BLOCKED — camera event has no Visual posterior to chain
B-T11 PRESERVED by no change; not a bridge proof
B-T12 PRESERVED by no change; not a bridge proof
B-T13 PASS by no-change canonical Shadow inheritance
B-T14 PASS by no-change legacy PARTIAL implementation
```

Prompt62 requires B-T2..B-T14 GREEN before playback, so the full-run gate is
closed.

## Production diff audit

```text
production files changed = NONE
camera payload changed = NO
producer changed = NO
measurement math changed = NO
LiDAR ownership changed = NO
33c1b3d restored = NO
7d9be50 restored = NO
ce3d1a9 restored = NO
```

## Owner decision required

A future prompt must relax exactly one incompatible boundary and explicitly
authorize its TDD scope. The smallest architecture-consistent candidate is a
bounded camera-event payload handoff into `statePropagateOnly`, followed by the
existing producer/measurement and `UpdateObserveFromPrior`; however, that is a
payload/lifecycle/placement change and is not authorized here.
