# GEODE given_offset_time branch audit

Authority: `https://github.com/thisparticle/GEODE_Evaluation` at `1f008a7249e36393a1752622de50660b77b5b7f4`.

The pinned `FAST_LIO/src/preprocess.cpp` executes:

```text
pcl::fromROSMsg → pcl::removeNaNFromPointCloud
→ if (pl_orig.points[plsize - 1].time > 0) given_offset_time=true
→ otherwise ring/yaw reconstruction
```

This is evaluated after finite XYZ compaction, frame by frame. `time > 0` is
the C++ strict comparison; a non-finite final time therefore selects FALSE.

| scene | frames | TRUE | TRUE fraction | FALSE | FALSE fraction | longest TRUE run | longest FALSE run | first/last TRUE | first/last FALSE |
|---|---:|---:|---:|---:|---:|---:|---:|---|---|
| Stairs | 3456 | 3437 | 0.994502 | 19 | 0.00549769 | 1783 | 4 | 0/3455 | 588/3224 |
| Tunnel2 | 2750 | 2750 | 1 | 0 | 0 | 2750 | 0 | 0/2749 | None/None |

Final finite-point original-index distributions:

* Stairs time: `n=3456; min=-0.0196326; P5=0.00130176; median/P50=0.00130637; P90=0.00130637; P95=0.00130637; P99=0.00130637; max=0.00130637`; original index: `n=3456; min=23120; P5=28767; median/P50=28799; P90=29183; P95=29183; P99=29535; max=29567`.
* Tunnel2 time: `n=2750; min=0.00130176; P5=0.00130637; median/P50=0.00130637; P90=0.00130637; P95=0.00130637; P99=0.00130637; max=0.00130637`; original index: `n=2750; min=28798; P5=28799; median/P50=28799; P90=29183; P95=29183; P99=29183; max=29183`.

Classification: `PT-D — MIXED_BRANCH_REQUIRES_FRAMEWISE_INTERPRETATION`.
Tunnel2 is entirely TRUE; Stairs is TRUE-dominant but contains 19 FALSE frames
and therefore cannot be represented by one static branch label.
