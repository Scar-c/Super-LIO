# Round13 canonical semantic-profile standard

Every experiment has two independent identities: an immutable `legacy_alias` and a normalized `semantic_profile`. A legacy name never selects protected algorithm semantics.

The frozen execution layering is:

```text
GTP transaction supervisor -> canonical semantic profile -> dataset adapter -> production estimator
```

The profile owns scheduler family, camera/epoch enablement, Visual frontend/producer/measurement/apply, raw-LiDAR policy, Observe count, and camera-stride policy. A dataset adapter owns only paths/topics, calibration, transport, time offset, GT/evaluator, and same-semantic sensor values. Protected-field override is a pre-playback `SEMANTIC_PROFILE_FAIL`.

| Profile | Scheduler/raw LiDAR | Camera/epoch | Frontend/producer/measurement | Apply |
|---|---|---|---|---|
| `D_SCHEDULER_BASE` | D corrected; full raw scan at scan end; one Observe | no claim | no claim | no claim |
| `D_VISUAL_SHADOW` | same | ON/ON | ON/ON/ON | OFF |
| `D_VISUAL_APPLY` | identical to Shadow | ON/ON | ON/ON/ON | ON |

`D_VISUAL_APPLY` differs from Shadow in exactly one protected field and is not authorized by Prompt59. `D_SCHEDULER_BASE` is descriptive and cannot be executed by the resolver.

The public implementation is `scripts/super_livo/experiments/semantic_profiles.py`. It resolves and validates `resolved_experiment_semantics.yaml`; `run_offline_variant.sh` consumes its ROS-parameter projection. Producer activation includes `/lio/g0/shadow=true`, `/lio/g1/enabled=true`, and a nonempty `/lio/g1/out_dir`. The manifest must exist and validate before the offline node opens the bag.

Required provenance is split among Super-LIO LIO authority, FAST-LIVO2 Visual authority, and dataset-author calibration. Production/profile/adapter/transaction revisions are mandatory. N-T1..N-T8 enforce this fail-closed boundary.
