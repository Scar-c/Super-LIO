# Normal versus weak segments

Geometry-only strata use the Native-N raw `weak_rank_R` at interval start;
there is no GT-dependent runtime logic.

```text
Fyllingsdalen 1 s rotation RMSE, rank=0:    N 3.9273 -> P1 3.5720 deg
Fyllingsdalen 1 s rotation RMSE, rank>0:    N 4.8990 -> P1 4.0202 deg
Fyllingsdalen 5 s rotation RMSE, rank=0:    N 1.3826 -> P1 1.8496 deg
Fyllingsdalen 5 s rotation RMSE, rank>0:    N 21.4993 -> P1 18.2004 deg

Runehamar 1 s rotation RMSE, rank=0:        N 1.0005 -> P1 1.0008 deg
Runehamar 1 s rotation RMSE, rank>0:        N 0.2354 -> P1 0.2357 deg
Runehamar 5 s rotation RMSE, rank=0:        N 0.9478 -> P1 0.9470 deg
Runehamar 5 s rotation RMSE, rank>0:        N 0.4608 -> P1 0.4612 deg
```

The normal-segment result is not a material regression on Runehamar; the
Fyllingsdalen 5 s rank=0 sample is small and mixed, while the rank>0 stratum
shows the directional rotation effect but not the translation benefit.
