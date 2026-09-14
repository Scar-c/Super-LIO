# DCReg PCG source authority

External authority: `https://github.com/JokerJohn/DCReg`

Pinned source commit: `8ce8451b15491a4bbe17cf85ab02a8bed6696861`.
Read-only audit checkout: `/tmp/dec_lio_dcreg_prompt09_source`.

Audited source:

- `DCReg/include/dcreg.hpp`: `DegeneracyCharacterization::preconditioner`
  documents the left preconditioner convention `z_k = P r_k`, with `P ~= H^-1`.
- `DCReg/include/utils.hpp:82-92`: source defaults include degeneracy threshold
  10, kappa target 10, tolerance `1e-6`, and max iteration 10. Prompt09 does
  not inherit those loose stopping defaults; the audit uses max 36,
  relative tolerance `1e-12`, absolute tolerance `1e-14`.
- `DCReg/include/dcreg.hpp:313-350`: eigenvalues are aligned, weak values are
  clamped to `max/kappa_target` (with a positive floor), and only the inverse
  clamped diagonal is assembled into the preconditioner block.
- `DCReg/include/dcreg.hpp:363-470`: PCG applies the preconditioner to the
  original residual and uses the original Hessian; invalid/failing paths in
  upstream fall back to raw QR.

Prompt09 adaptation keeps the external preconditioner semantics but preserves
the native fused 18D `A` and `r`; no upstream objective fallback is connected
to estimator control.
