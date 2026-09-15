# Native authority inputs

## Tunnel2 Alpha Native side-by-side

| field | Prompt18 canonical artifact | Prompt20 current artifact |
|---|---|---|
| repository HEAD | `b60408e1f52c027c8347adea390a9fd38f232d46` | `6b0abd90681138d39277e756d63e447193f7074d` |
| runtime artifact | `runtime/prompt18/identity/tunnel2_alpha_native_post_identity/trajectory.tum` | `runtime/prompt20/tunnel2_alpha_native/trajectory.tum` |
| bag | `/home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2_alpha.bag` | same |
| bag SHA256 | `08a4a32f660b3d2df3d2adb053fe86d310505a1689e9d37b3f87a6a20768498e` | same |
| config | `src/super_lio/config/geode_tunneling2_alpha.yaml` | same |
| config SHA256 | `1e081c1e414e251a1284dbef9688bc74955d86a7c029687f779ba5b9f0540c04` | same |
| GT | `/home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt` | same |
| GT SHA256 | `b83ca5db4102838cd260fc78b0a20cca90da9db855ab9b762f9d7f2e5c856c5a` | same |
| mode | `native` | `native` |
| requested threads | `4` | `4` |
| offline duration/rate | whole-bag / `1.0` | whole-bag / `1.0` |
| trajectory rows | `2739` | `2739` |
| trajectory bytes | `449428` | `449428` |
| trajectory SHA256 | `3668c6a5ca49471560d69469506f6ebf0a20bc5bd61c7fa715547145bd936e30` | same |
| historical ATE RMSE | `6.450671654842644` m | `6.30079297` m |
| historical matched GT rows | `591` | `580` |

Prompt18 provenance is recovered from
`runtime/prompt18/identity/tunnel2_alpha_native_post_identity/meta.txt` and
`runtime/prompt18/evaluation/tunnel2_alpha_a0.csv`. Prompt20 provenance is
recovered from `runtime/prompt20/tunnel2_alpha_native/meta.txt` and
`evidence/dec_lio/prompt20/THREE_SEQUENCE_SUMMARY.csv`.

## Runner/evaluator commands

Prompt18's stored Alpha Native evaluation row is the `native` row in
`runtime/prompt18/evaluation/tunnel2_alpha_a0.csv`. Its contract is the
Prompt17-corrected `eval/dec_lio/prompt16_evaluate.py` contract, with
`GT_TOLERANCE=0.10`, one independent SE(3) Umeyama alignment, and no crop or
scale. Re-running that contract on the recovered Prompt18 trajectory gives
the stored `6.450671654842644` value.

Prompt20's canonical Native run command was:

```text
tools/dec_lio/run_baseline.sh --mode offline \
  --sequence tunneling_tunnel2_alpha --estimator-mode native \
  --bag /home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2_alpha.bag \
  --config src/super_lio/config/geode_tunneling2_alpha.yaml \
  --out /home/lc/dec_lio/runtime/prompt20 \
  --run-id tunnel2_alpha_native --threads 4
```

Prompt20 evaluated with:

```text
python3 eval/dec_lio/eval_tum_translation.py \
  <trajectory.tum> /home/lc/dec_lio/bag/GEODE/Tunneling_tunnel2.txt \
  --max-diff 0.05 --min-matches 3 --out <evaluation.txt>
```

The two runner provenance records differ only in output paths and the later
Prompt20-added empty loose-pose diagnostics namespace; the Alpha input,
preprocessing, Native selector, and 4-thread policy are identical.
