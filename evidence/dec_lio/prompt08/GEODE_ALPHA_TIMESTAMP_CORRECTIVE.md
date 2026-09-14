# GEODE Alpha timestamp-unit corrective authority

External authority: `https://github.com/thisparticle/GEODE_Evaluation` at commit `1f008a7249e36393a1752622de50660b77b5b7f4`. No external
repository was modified.

The pinned `FAST_LIO/src/preprocess.h` declares:

```cpp
enum TIME_UNIT {SEC = 0, MS = 1, US = 2, NS = 3};
```

The pinned Alpha config sets `timestamp_unit: 2`, i.e. `US`. The released bag
field is seconds-scale point time. The pinned `preprocess.cpp` maps `US` to
`time_unit_scale = 1.e-3f` for curvature in milliseconds, while `SEC` maps to
`1.e3f`. The pinned `laserMapping.cpp` then divides the curvature by 1000 for
the physical query offset. Thus:

```text
official US: raw bag seconds × 1e-3 ms × 1e-3 s/ms = raw × 1e-6 s
corrected SEC: raw bag seconds × 1e3 ms ÷ 1e3 = raw seconds
```

Minimal corrected conceptual config:

```text
original:  timestamp_unit = US (2)
corrected: timestamp_unit = SEC (0)
```

Known source hashes from the pinned checkout:

```text
FAST_LIO/config/alpha.yaml       30a2ee38e95b5f4585c62791af54b30dae0bddad279e2d09026b89bf79d9a6c4
FAST_LIO/launch/alpha.launch      19b23549a0c6b25a8be928f97d2b2853247eb50235fed698514d69ca6c5bc749
FAST_LIO/src/preprocess.cpp       3024133a7887895364c02461ec3fb5213df44351a7e7665063b372b6479636cc
FAST_LIO/src/laserMapping.cpp     e1683fbe24988de262e645be8c16cfc2428c012e3205371bf9d0a39600dc0624
```

This establishes a corrected experimental authority, not full FAST-LIO
equivalence: map, estimator, and IMU semantics remain native Super-LIO.
