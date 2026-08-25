# V-2 Photometric Jacobian — Round 11E DC-Bundle Differentiability

Status: corrective evidence（Round 11E；bundle-level smoothness 实现）
Related commits: Round 11E（DC bundle corrective）
Last updated: 2026-08-25

## Why sample-level smoothness was insufficient

DC residual r_k = (I_k - mean_c) - (I_k^ref - mean_r); D r_k = D I_k - D mean_c;
D mean_c = (1/M) sum D I_j -> every sample's derivative depends on the whole
bundle. Sample-k smoothness does not imply bundle smoothness.

## Bundle smoothness implementation

For each landmark x epoch x direction x eps: frozen DC bundle of M samples;
bundle smooth only if ALL production-valid samples keep support and bilinear
cell across -eps/base/+eps. Otherwise BUNDLE_NON_SMOOTH (support/cell counted
separately); non-smooth directions are not relative-gated.

## Single-source DC mean (E2)

mean_cur = sum(ic_vals)/M using the exact stored sample values (no resample).

## Trial counter semantics (E3)

attempted / structurally_complete (all 6 directions evaluated/classified) /
all6_smooth / with_nonsmooth. complete != PASS.

## eee 30s (13 epochs, 225 landmark_ids, 1236 trials)

```text
bundle: rx smooth 1059/1236, ry 1048, rz 1028, tx 959, ty 1015, tz 989
        nonsmooth_support 0 all; nonsmooth_cell 177-277
all6_smooth=897, with_nonsmooth=339

double oracle (smooth bundles only):
  rx 5.2e-3 PASS, tx 1.5e-5 PASS, ty 3.8e-3 PASS, tz 2.5e-4 PASS
  ry 3.5e-2 (worst z=0.73, fd=-0.074 an=-0.0765), rz 3.3e-2 (z=0.065)
  strong_med_rel < 3.1e-6 all directions
```

## Five-level decomposition (true smooth worst)

```text
L1 Xc : an ~ fd (1e-7)       PASS
L2 uv : an ~ fd (1e-3)       PASS
L3 raw: an ~ fd (0.01)       PASS
L4 mean: an ~ fd (0.01)      PASS
L5 DC : an=-0.27487 vs fd=-0.265962 (diff 0.0089); an=-0.0765 vs fd=-0.0740
       (diff 0.0025) -> float an DC large-number cancellation precision
```

## Conclusion

- Implementation correct: L1-L4 exact; bundle-level smoothness moves the
  previous huge maxima into BUNDLE_NON_SMOOTH counts.
- Remaining ry/rz ~3.3% on smooth bundles: float production Js DC
  cancellation (L3/L4 each float 0.01; L5 difference 0.0025-0.0089) and
  near-zero-derivative rel amplification.
- Per §31: fixing requires changing strong threshold / epsilon / gate
  (forbidden) -> STOP FOR OWNER.
- HISTORICAL DIAGNOSIS — SUPERSEDED: earlier "near-depth z<0.11 is
  non-smooth / eps=1e-6 required / float-Js 3.3% at z=0.065 only" — replaced
  by bundle-level finding (maxima were bundle branch crossings).
