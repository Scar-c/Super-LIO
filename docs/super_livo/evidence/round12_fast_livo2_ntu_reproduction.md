# Round12 — FAST-LIVO2 NTU reproduction (eee_01 / nya_01)

## Evaluation (official NTU VIRAL semantics)

Reference: https://github.com/ntu-aris/ntu_viral_dataset/blob/gh-pages/ntuviral_evaluate.ipynb
and https://ntu-aris.github.io/ntu_viral_dataset/evaluation_tutorial.html
(Official: prism compensation T_B_prism = (-0.293656,-0.012288,-0.273095);
GT resample at estimate times with 0.1 s association window; Umeyama SE(3)
alignment; ATE = norm(per-axis RMS).)

Implemented in scripts/super_livo/evaluation/ntu_viral_official_ate.py (committed).

## F1 — eee_01

- method-author config NTU_VIRAL + camera_NTU_VIRAL; online rosbag play
- trajectory: Log/result/eee_01.txt (TUM) -> captured
  results/upstream_reproduction/fast_livo2/ntu/eee_01/trajectory.tum
  (sha256 e121adf2...)
- leica GT: /leica/pose/relative extracted (6616 rows)
- **ATE = 0.0303 m** vs published reference ~0.0271 m
- |R-P| = 0.0032 <= max(0.02, 0.20*P) = 0.02 -> GREEN

## F2 — nya_01

- same config family; seq_name override note: node wrote to the config's
  default seq_name file (eee_01.txt); the nya output preserved as
  results/upstream_reproduction/fast_livo2/ntu/nya_01/trajectory.tum
  (sha256 c7276eff...)
- leica GT: /leica/pose/relative extracted (7769 rows)
- **ATE = 0.0398 m** vs published reference ~0.0356 m
- |R-P| = 0.0042 <= 0.02 -> GREEN

## User-provided references recorded

- https://github.com/ntu-aris/ntu_viral_dataset/blob/gh-pages/ntuviral_evaluate.ipynb
- https://ntu-aris.github.io/ntu_viral_dataset/evaluation_tutorial.html
