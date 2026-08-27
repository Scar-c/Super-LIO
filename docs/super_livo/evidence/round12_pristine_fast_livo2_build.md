# Round12 pristine FAST-LIVO2 build evidence

This record establishes build readiness only. It does **not** claim a dataset
reproduction.

## Pinned inputs

- FAST-LIVO2: `hku-mars/FAST-LIVO2@0d2c0346107b75b59934975adec9a6eeeb913c64`
- rpg_vikit: `xuankuzcr/rpg_vikit@6c886c8e5d83997806e00294826d528cea3581dd`
- Sophus: `strasdat/Sophus@a621ff2e56c56c839a6c40418d42c3c254424b5c`
- temporary workspace: `/tmp/round12-fastlivo2-ws.70kmbv`

The FAST-LIVO2 and rpg_vikit worktrees were clean. No production source or
configuration was patched.

## Diagnosis and resolution

The first rpg_vikit link failed with unresolved non-templated `Sophus::SE3`
symbols. A subsequent attempt to rebuild the old pinned Sophus source against
the host Eigen failed in Sophus itself. The machine already contained a clean
build of the exact README-pinned Sophus revision. Supplying
`/home/lc/3rdparty/Sophus/build` explicitly in `CMAKE_PREFIX_PATH` resolved the
dependency discovery and ABI issue. This is an environment/dependency-path
resolution, not an algorithm change.

Build command (after sourcing ROS Noetic):

```text
CMAKE_PREFIX_PATH=/home/lc/3rdparty/Sophus/build:$CMAKE_PREFIX_PATH \
catkin_make -C /tmp/round12-fastlivo2-ws.70kmbv \
  -DCMAKE_BUILD_TYPE=Release -DCATKIN_ENABLE_TESTING=OFF
```

Result: `fastlivo_mapping` built successfully (return code 0).

## Artifact hashes and runtime linkage

- `fastlivo_mapping`: `75d261d765b2b347ca4e48c553b1bf4a701017eeef064c4225b7a0eeaf27aa9e`
- `libvikit_common.so`: `07d1836e7130854c751649afe067f812c32eeaeaaf8518ce04ca976140fa566d`
- `libSophus.so`: `60db4bc2589aff0c4cddaabedd266bb752cd6b7d186f432157ce4c696ebdb943`

`ldd` resolves `libvikit_common.so` from the isolated workspace and
`libSophus.so` from the pinned local build; no dependency is reported missing.

## Experiment gate

No FAST-LIVO2 bag run was started. The mandatory Super-LIO MCD night08
reproduction remained RED after three identical runs, which triggers Prompt49
section 50 `STOP FOR OWNER`. Remaining FAST-LIVO2 rows therefore stay pending.
