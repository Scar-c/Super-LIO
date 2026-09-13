# Prompt03 D2 final design recommendation

Primary classification: **F — MULTI_SIGNAL_REQUIRED_BUT_NOT_YET_IDENTIFIABLE**.

Kappa alone is not sufficient: matched 5–10 and 10–20 bins have Bridge
translation errors of 4.863 m and 6.039 m versus Stairs 0.154 m and 0.208 m.
Absolute lambda and prior mu_min show the most stable continuous correlations,
but their event thresholds and distributions are sequence-dependent. XICP
non-FULL is more common on low-error Stairs than on Bridge; it is not a safe
standalone harmfulness signal. Persistence reduces isolated events but loses
high-error coverage and leaves many low-error activations.

Retain for a future reviewed snapshot: D1 Schur kappa, absolute lambda_min,
used count, raw-block kappa, XICP Lc/Ls/class as diagnostics, P_pred pose
validity, mu, and weak-direction eta. The minimal future object should carry
only the frame-authority values needed by an approved policy: `{frame,
timestamp, D1 weak projector/rank, kappa, lambda_min, N_used, prior_valid,
mu_min/mu_median, eta_weak}` plus optional XICP diagnostics for audit. Do not
copy full matrices into a future estimator object unless later authority
requires them.

No future gamma semantics is authorized here. If later approved, gamma must be
defined from frame-level pre-update values and validated on held-out sequences;
it must not be derived from posterior confidence. No freeze semantics is
authorized; a future freeze decision must be an explicit state-machine policy
with a tested reset condition. No hysteresis is implemented; persistence is a
shadow diagnostic only.

D2 remains planned. This evidence does not authorize H/b/P modification,
gamma application, PCG, or any Prob-LIO estimator implementation.
