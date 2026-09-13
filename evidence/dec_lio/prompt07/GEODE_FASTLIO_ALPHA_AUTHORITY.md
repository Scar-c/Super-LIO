# GEODE FAST-LIO Alpha authority

- repository: `https://github.com/thisparticle/GEODE_Evaluation`
- pinned commit: `1f008a7249e36393a1752622de50660b77b5b7f4`
- checkout: detached at the pin; no estimator source was copied into Super-LIO

| audited file | SHA256 |
|---|---|
| `FAST_LIO/config/alpha.yaml` | `30a2ee38e95b5f4585c62791af54b30dae0bddad279e2d09026b89bf79d9a6c4` |
| `FAST_LIO/launch/alpha.launch` | `19b23549a0c6b25a8be928f97d2b2853247eb50235fed698514d69ca6c5bc749` |
| `FAST_LIO/src/preprocess.cpp` | `3024133a7887895364c02461ec3fb5213df44351a7e7665063b372b6479636cc` |
| `FAST_LIO/src/laserMapping.cpp` | `e1683fbe24988de262e645be8c16cfc2428c012e3205371bf9d0a39600dc0624` |

The audited excerpts declare Velodyne/VLP16/10 Hz, timestamp unit 2, blind
1.5, FOV 180, det_range 100, feature extraction off, stride 3, mapping
iterations 10, surface .3, map .5 and cube side 1000. `preprocess.cpp` scales
SEC/MS/US/NS to milliseconds as 1e3/1/1e-3/1e-6. `laserMapping.cpp` uses
`DET_RANGE` in map-edge/FOV movement maintenance; raw point acceptance is in
the preprocessing path. This is authority evidence, not FAST-LIO reproduction.
