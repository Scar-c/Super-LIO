# Baseline authority

Prompt00R is the authority for the Dec-LIO native parity baseline. The exact
operator copy is kept at
`prompts/dec_lio/PROMPT00R_NATIVE_PARITY_BASELINE.md`.

The implementation source of truth descends directly from the clean ROS1 branch:

```text
origin/ros1 = 60b57aaac8dc397f80c56364e7ccb008c300cc29
HEAD       = 3916769cdcb1569bbf267a56447054c07ac29b91 (Prompt03 D2 shadow
           calibration implementation; canonical runtime gates recorded under
           evidence/dec_lio/prompt03/)
```

The native estimator math remains the `origin/ros1` implementation. Prompt00R
added only the algorithm-neutral offline transport; Prompt01 adds a read-only
D1 diagnostic hook at the post-reduction/pre-delivery seam; Prompt03 adds a
read-only D2 shadow at the native update boundary. No update, gating, scaling,
solver, or preconditioner is authorized here. The historical
Prob-LIO workspace at
`/home/lc/prob_lio` is not an input tree for this checkout and must not be
deleted, moved, stashed, or modified.

The authoritative input files are external runtime inputs:

```text
/home/lc/dec_lio/bag/GEODE/bridge01.bag
/home/lc/dec_lio/bag/GEODE/alpha_config.yaml
/home/lc/dec_lio/bag/GEODE/bridge01.txt
/home/lc/dec_lio/bag/GEODE/stairs_alpha.bag
/home/lc/dec_lio/bag/GEODE/stairs_alpha.txt
```

`stairs_alpha.txt` is the official Stairs Alpha trajectory reference produced
by the GEODE PALoc-to-GT-map procedure and is used for ATE. Their identities
are recorded under `evidence/dec_lio/`.
