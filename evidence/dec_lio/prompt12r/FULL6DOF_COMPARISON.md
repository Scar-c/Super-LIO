# Full-6DoF held-out comparison

Evaluation contract: 0.05 s greedy timestamp association, no scale, one
global SE(3) alignment, no GT time-offset optimization, no crop; N and P1
use the same overlap.

## Fyllingsdalen

```text
matches/overlap: 2335 / 237.3608 s

APE translation RMSE N/P1: 2307.2587 / 2671.3478 m  (improvement -15.7802%)
APE translation median N/P1: 1247.7927 / 1408.6670 m
APE translation P95 N/P1: 5647.1060 / 6576.9578 m

APE rotation RMSE N/P1: 158.1704 / 154.8289 deg (improvement +2.1126%)
APE rotation median N/P1: 156.0988 / 151.2147 deg
APE rotation P95 N/P1: 173.3696 / 173.9488 deg

RPE translation RMSE (N -> P1), 1/5/10 s:
  103.2082 -> 120.2598 m; 500.6502 -> 584.6718 m;
  944.3184 -> 1106.4882 m
RPE rotation RMSE (N -> P1), 1/5/10 s:
  4.8568 -> 3.9998 deg; 21.0625 -> 17.8330 deg;
  38.3722 -> 34.3733 deg
```

## Runehamar

```text
matches/overlap: 2305 / 266.0579 s

APE translation RMSE N/P1: 0.974776 / 0.977375 m (improvement -0.2666%)
APE translation median N/P1: 0.780621 / 0.781629 m
APE translation P95 N/P1: 1.6274227 / 1.6378347 m

APE rotation RMSE N/P1: 12.81747 / 12.83110 deg (improvement -0.1063%)
APE rotation median N/P1: 12.85308 / 12.86629 deg
APE rotation P95 N/P1: 13.12063 / 13.18574 deg

RPE translation RMSE (N -> P1), 1/5/10 s:
  0.179054 -> 0.179003 m; 0.502215 -> 0.502578 m;
  1.114429 -> 1.114999 m
RPE rotation RMSE (N -> P1), 1/5/10 s:
  0.320164 -> 0.320477 deg; 0.497294 -> 0.497607 deg;
  0.776991 -> 0.777648 deg
```

The full trajectory numbers are descriptive held-out results, not tuning
targets. Fyllingsdalen shows a small rotation benefit with a 15.8% translation
regression; Runehamar is roughly neutral.
