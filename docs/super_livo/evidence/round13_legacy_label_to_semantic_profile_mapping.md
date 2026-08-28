# Round13 legacy label to normalized semantic profile mapping

This mapping preserves historical names and classifies effective semantics from committed runners, snapshots, logs, and counters. It supersedes the naming inference in the earlier D-runner recovery prose; it does not rename or invalidate historical trajectory rows.

## Mechanical conclusions

- The shared D runner is `scripts/super_livo/experiments/run_offline_variant.sh`.
- Commit `4543347` removed `/lio/g0/shadow`, `/lio/g1/enabled`, and `/lio/g1/out_dir`. Constructor/runtime gates require all of them before `runVisualLifecycle` and the state-off `runVisualResidual` branch are reachable. This is `4543347_RUNNER_PRODUCER_GATE_REGRESSION`, not an algorithm-quality failure.
- Post-`4543347` D0/D-S1/D-S3 runs establish D scheduling, full-raw-scan ownership, one Observe, and their stride. They do not establish Visual producer/measurement activity and map to `D_SCHEDULER_BASE`.
- S1/S3 are mechanically the `/camera/temporal_stride` values 1/3, not Visual apply modes.
- No Oxford D execution artifact was found; the historical registry records it as stopped/not run.

| Legacy label / scope | Effective Visual semantics | Scheduler / stride | Normalized profile | Confidence |
|---|---|---|---|---|
| D0, NTU eee_01 and nya_01 | camera/epoch configured ON; v0/v2 configured ON; producer OFF; measurement NOT_ESTABLISHED; apply OFF | D corrected / 1 | `D_SCHEDULER_BASE` | PROVEN |
| D-S1, MCD Day10 | same producer regression | D corrected / 1 | `D_SCHEDULER_BASE` | PROVEN |
| D-S3, MCD Day10/night08 and M3 Corridor01 | same producer regression | D corrected / 3 | `D_SCHEDULER_BASE` | PROVEN |
| DV0, Round13 eee_01 `20260828T043621Z_ea80ba48` | snapshot lacks camera and camera-epoch enables and all producer gates; v0/v2 true; apply parameter true but apply_count=0; landmarks/samples/H/b=0 | D corrected / 1 | `OTHER` (armed apply label, no effective Visual lifecycle) | PROVEN |
| C0, eee_01 v0/v0c era | producer and measurement evidence exists; state apply OFF; scheduler is partial/C-family, not D | partial / 1 | `OTHER` | PARTIAL (legacy snapshot absent; counters present) |
| A0, eee_01 Round11N/O | producer/measurement and state apply active; scheduler is partial/C-family | partial / 1 | `OTHER` | PARTIAL |
| A1, eee_01 Round11P | A0 plus fixed outlier gate; state apply active | partial / 1 | `OTHER` | PARTIAL |

## Explicit DV0 resolution

```text
HISTORICAL_DV0_FOUND = YES
revision = Round13 transaction at the post-711a historical artifact frontier
runner = base_ws/tools/benchmark_adapters/launch_r13_dv0.sh -> run_superlivo_transaction.sh
dataset = NTU eee_01
frontend effective = NO (configured flags alone; lifecycle frames=0)
producer effective = NO
measurement effective = NO
state apply = NO effective application (parameter true; apply_count=0)
normalized semantic profile = OTHER
```

The name `DV0` therefore cannot be reused as proof of `D_VISUAL_SHADOW` or `D_VISUAL_APPLY`.

## Historical eee Visual evidence correction

The exact nonzero artifacts are:

- `results/super_livo/v0/eee/run_v0`, revision `d01344b3...`: `parents=263`, `landmarks=102`, `frames=3263`.
- `results/super_livo/v0c/eee/run_v0c`, revision `5f8a3da4...` with dirty-count 3: `frames=3263`, candidate coverage `3263`, `filled_new=61193`, `landmarks=61193`, photometric `accepted_landmarks=67`, `total_samples=17657887`.
- `docs/super_livo/evidence/hb0_production_numeric_audit.md`: eee H/b audit has nonzero epochs/landmarks/samples and bitwise state-off parity.

Earlier prose incorrectly attached the 263/102 counters to revision `5f8a3da4`; this document corrects that provenance without discarding either artifact. The conclusions are:

```text
HISTORICAL_EEE_LANDMARKS_NONZERO = YES
HISTORICAL_EEE_CANDIDATES_NONZERO = YES
HISTORICAL_EEE_RESIDUALS_NONZERO = YES
HISTORICAL_EEE_HB_NONZERO = YES
```

Historical D trajectories remain valid for their proven scheduler/ownership scope. Their measurement-active shadow claim is `NOT_ESTABLISHED`.
