# Raw DCReg held-out characterization

Statistics use raw pre-attenuation `dcreg_shadow.csv`, not attenuated H and
not the P1 control output. The main values are median/P95/max.

```text
Fyllingsdalen N:
 cond_R = 32.9096 / 49.0161 / 3.3727e13
 cond_t = 7.2116 / 17.3580 / 1.2811e7
 weak_rank_R counts = {0:353, 1:8593, 2:128, 3:3}
 weak_rank_t counts = {0:7102, 1:1835, 2:117, 3:23}
 principal-angle-R median/P95/max = 0.0003789 / 0.0051825 / 1.52134 deg

Runehamar N:
 cond_R = 18.9097 / 28.3786 / 34.4736
 cond_t = 2.7078 / 4.3137 / 5.34535
 weak_rank_R counts = {0:476, 1:9163}
 weak_rank_t counts = {0:9639}
 principal-angle-R median/P95/max = 0.0005051 / 0.0026602 / 0.029014 deg
```

P1 exposure:

```text
Fyllingsdalen active fraction=0.94785, valid fraction=0.98710,
 gamma median/P95=0.55049/0.85689, trace-ratio median/P95=0.998275/0.9999999
Runehamar active fraction=0.94971, valid fraction=1.0,
 gamma median/P95=0.72671/1.0, trace-ratio median/P95=0.997398/1.0
```

The singular maxima are retained as observations; no clipping or threshold
retuning was applied.
