# Prompt16 classification

Final classification: **P16-C — TIGHT_COUPLING_SUPERIOR**.

The implementation is internally runnable and repeatable on the primary
stairs sequence (A1/A2 trajectory SHA identical), but asymmetric APE is much
worse on both healthy primary sequences: stairs `11.1251 m` versus native
`0.197724 m`, and tunnel1 gamma `1717.956 m` versus native `0.142507 m` under
one fixed native alignment. Stress sequences are also worse in APE. This is a
negative result for BIEVR-style pose authority on the current Super
point-to-plane/HKNN frontend, not a claim that BIEVR-LIO is incorrect.

The tunnel2-alpha gap-fix run completes the full timestamp span, but its
fixed-pose inertial state develops very large bias norms; that is reported as
state-health evidence, not hidden by a native fallback.

The classification was written after runs and metrics were frozen; no GT-based
parameter tuning was performed.

Decision rule:

- P16-A: both healthy primary sequences complete and show reproducible,
  meaningful asymmetric benefit without major stability regression.
- P16-B: valid implementation with mixed or small/inconsistent primary result.
- P16-C: valid implementation but asymmetric consistently worsens healthy
  primary sequences or is materially less stable.
- P16-D: lifecycle/identity/solver validity is not established; do not read ATE
  scientifically.
