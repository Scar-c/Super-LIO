# prob_lio — eval

Evaluation helpers for prob-lio baseline validation.

- `extract_and_compare.py` — trajectory fidelity check. Compares two
  trajectories (TUM file or rosbag with `--topic /lio/odom`) stamp-by-stamp;
  reports matched/unmatched stamps, max/mean position delta, max orientation
  delta, and `RESULT: IDENTICAL|DIFFERS`.

Usage:

```bash
python3 eval/prob_lio/extract_and_compare.py \
  results/prob_lio/run_<online>/result.bag \
  results/prob_lio/run_<offline>/trajectory.tum
```

P0 result: online (30 s window) vs offline (full bag) — 294/294 stamps
matched exactly, max position delta `0.000e+00 m`, IDENTICAL.
