# Baseline authority

Prompt00R is the authority for the Dec-LIO native parity baseline. The exact
operator copy is kept at
`prompts/dec_lio/PROMPT00R_NATIVE_PARITY_BASELINE.md`.

The implementation source of truth is the clean ROS1 branch:

```text
origin/ros1 = 60b57aaac8dc397f80c56364e7ccb008c300cc29
HEAD       = 60b57aaac8dc397f80c56364e7ccb008c300cc29 (bootstrap)
```

Only algorithm-neutral offline transport, evaluation, tests, evidence, and
documentation may be added. The historical Prob-LIO workspace at
`/home/lc/prob_lio` is not an input tree for this checkout and must not be
deleted, moved, stashed, or modified.

The authoritative input files are external runtime inputs:

```text
/home/lc/dec_lio/bag/GEODE/bridge01.bag
/home/lc/dec_lio/bag/GEODE/alpha_config.yaml
/home/lc/dec_lio/bag/GEODE/bridge01.txt
```

Their identities are recorded under `evidence/dec_lio/` after inspection.
