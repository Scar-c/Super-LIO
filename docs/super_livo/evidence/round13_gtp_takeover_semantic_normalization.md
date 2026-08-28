# Round13 GTP takeover and semantic normalization

GTP accepted the actual DS frontier `711a6674`, registered Prompt59, and preserved Prompt58 as superseded provenance. The audit and forward-revert disposition are in `round13_prompt58_gtp_takeover_audit.md`. Production estimator files were restored exactly to accepted Round12 semantic frontier `8321586` by forward changes, not reset.

Normalized semantics are now resolved by `semantic_profiles.py`; aliases cannot select protected fields. The shared runner consumes the resolved projection, including the producer gates lost at `4543347`. The NTU adapter contains dataset facts only. The GTP supervisor holds the exclusive lock, verifies no active/stale transaction or conflicting estimator/rosbag, and uses a two-party pre-playback handshake: the runner persists manifest/readback, the supervisor validates them, and only then creates the authorization marker that lets the node open the bag.

Verification:

```text
N-T1..N-T8 = 8/8 PASS
S-T1..S-T6 = 6/6 PASS
offline node build = PASS
production files equal 8321586 before profile work = YES
```

The first sandboxed launch failed before playback because socket creation was denied. It is retained as `INVALID_NO_PLAYBACK`, terminal and cleanup verified. The sole full experiment was the escalated retry. Its six printed preflight lines all passed and the transaction ended `SUCCESS`, `cleanup_verified=true`.

Scientific result validation is deliberately separate from process/trajectory validity. The full run exposed that the existing aggregate log does not persist every Prompt59 hard-gate counter. `validate_d_visual_shadow_result.py` now fails closed on this condition for future transactions. No second full bag was run.
