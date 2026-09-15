# Native evaluator parity

Both contracts were run on the same byte-identical trajectory:

`runtime/prompt18/identity/tunnel2_alpha_native_post_identity/trajectory.tum`

## Prompt18/Prompt17 corrected extended evaluator

Command:

```text
python3 eval/dec_lio/prompt16_evaluate.py \
  --sequence tunneling_tunnel2_alpha \
  --ground-truth /home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt \
  --native runtime/prompt18/identity/tunnel2_alpha_native_post_identity/trajectory.tum \
  --asymmetric runtime/prompt20/tunnel2_alpha_native/trajectory.tum \
  --out /tmp/p21_prompt16_same_native.csv
```

Result for both supplied identical files:

```text
GT tolerance: 0.10 s (module GT_TOLERANCE)
matches: 591
APE RMSE: 6.450671654842644 m
alignment: independent SE(3) Umeyama, no scale/crop
```

## Prompt20 compact evaluator

Command:

```text
python3 eval/dec_lio/eval_tum_translation.py \
  runtime/prompt18/identity/tunnel2_alpha_native_post_identity/trajectory.tum \
  /home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt \
  --max-diff 0.05 --min-matches 3 --out /tmp/p21_current_eval.txt
```

Result:

```text
GT tolerance: 0.05 s (explicit command argument)
matches: 580
APE RMSE: 6.30079297 m
alignment: independent SE(3) Umeyama, no scale/crop
```

The supplied GEODE GT contains unique but non-monotonic records. Both
contracts use a stable timestamp sort for GT association. The estimate remains
in native output order. The only numerical discriminator here is the
association tolerance: `0.10 s` versus `0.05 s`; the 11 additional accepted
GT matches change the Umeyama fit and the reported RMSE.
