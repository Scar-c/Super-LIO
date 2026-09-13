# Prompt04 D2 recommendation

Primary Tunnel2 classification:

**C — TUNNEL2_IS_BRIDGE_LIKE_AMBIGUITY_OR_BIASED_ASSOCIATION**

Evidence:

- translation local error reaches 4.425 m in the sustained six-window episode;
- translation κ is moderate (median 1.823, p95 5.478, max 8.400), with no
  translation weak direction at any frame;
- XICP is FULL for 91.7001% of translation directions and NONE for 0%;
- translation mu remains around 0.64 median and translation eta/zeta are
  undefined because the directional weak rank is zero;
- at the defined onset, translation κ is not higher than its 5 s precursor;
- matched 5–10 bins separate Bridge (4.863 m) from Stairs (0.154 m) and
  Tunnel2 (0.860 m), but do not yield a transferable κ-only harmfulness rule.

Rotational Tunnel2 error cannot be used as a counterargument because its
official GT reference contains no attitude.  The available rotational shadow
signals are reported, but no rotational local-error claim is made.

Selected D2 outcome: **D2-C**.

The D2 scope should be restricted to geometry-positive, Stairs/Tunneling-like
diagnostic conditions; Bridge-like association/biased failures are outside a
directional-degeneracy-only harmfulness detector.  A future harmfulness gate
would still require estimator consistency/innovation evidence, because this
Prompt04 shadow cannot distinguish wrong association from state inconsistency.
No estimator gate, gamma, H/b/P modification, PCG, or freeze policy is
authorized by this recommendation.
