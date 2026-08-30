# Prob-LIO Prompt Index

Trace which Owner instruction produced which commit on `prob-lio`.

| Prompt | Round | Topic | Producing commit |
|---|---|---|---|
| `prob_lio/prompt1_P0_baseline_freeze.md` | P0-1 | Baseline freeze / project bootstrap | P0 closure `63f97ea` + docs follow-up `bb8596f` |
| `prob_lio/prompt2_P0_eval_closure_P1_point_probability.md` | P0-2 / P1-1 | P0 evaluator/parity closure → P1 current point probability | Commit A (P0 corrective closure) + Commit B (P1) — see `git log` |
