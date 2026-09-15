# Evaluator correction

Prompt16's RPE and distance-segment code compared raw world-frame displacement
vectors. That is not invariant when two trajectories differ by one constant
world rotation, so the old values are superseded.

Prompt17 now evaluates, for each start/end pair,

```text
E_ij = (T_i^est^-1 T_j^est)^-1 (T_i^gt^-1 T_j^gt)
```

and reports the norm of the translation component of `E_ij`. The 5/10/20 m
segments choose the horizon from the ground-truth cumulative distance, then
match estimate poses by time. The 1/5/10/20 s RPE uses the same relative SE(3)
translation error.

APE is now reported in two explicitly labeled frames:

- `ape_*`: an independent Umeyama fit for each branch;
- `common_frame_ape_*`: the native branch's fit applied to both branches.

`tests/dec_lio/test_prompt17_evaluator.py` verifies global-rotation invariance
of RPE and segments and checks that both APE frames are present and labeled.
