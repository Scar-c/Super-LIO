# Asymmetric covariance lifecycle audit

## Problem statement

The current covariance is one 18-by-18 ESKF matrix with blocks for
[R,p,v,bg,ba,g]. In particular it contains P_RR, P_Rp, P_Rv, P_Rbg,
P_Rba, P_Rg, and the corresponding position/inertial cross blocks. Predict()
propagates the complete matrix, while UpdateObserve() computes a complete
18-D information update even though the LiDAR matrix occupies only its
leading pose block.

If an external LiDAR registration changes R,p after propagation, the nominal
mean no longer corresponds to the old P without a specified
conditioning/reset operation. The stale cross-covariances would still claim
correlation with the old pose mean, and the old P_RR/P_pp would not describe
the accepted registration uncertainty.

## Candidate strategies

### A. Retain the old full P unchanged

This is acceptable only for a zero-side-effect shadow that never becomes
state authority. As a production lifecycle it is mathematically inconsistent:
the nominal pose is externally replaced while P still describes the
propagated historical filter estimate and its old cross-covariances. It is
rejected for asymmetric production mode.

### B. Replace the pose block from LiDAR Hessian covariance

Using an inverse/pseudoinverse of H_L as a new pose covariance can represent
a local LiDAR-only uncertainty, but replacing only P_RR/P_Rp/P_pp does not
specify the joint covariance with v,bg,ba,g. Dropping or inventing the cross
blocks changes estimator semantics. A full joint construction would need the
LiDAR linearization, prior/state conditioning, and a defined registration noise
model. It is not a safe block assignment.

### C. Reset pose-related cross covariance

Setting pose-to-inertial cross blocks to zero and choosing a conservative pose
block can be a deliberately conservative approximation. It is discontinuous
and may discard useful correlations; the pose covariance and reset rule still
need a documented noise/observability policy. It is not intrinsically
consistent merely because it is simple. It remains a possible bounded
experimental policy, not the default design.

### D. Constrained/fixed-pose inertial update

Treat the accepted LiDAR pose as a fixed measurement/parameter and update only
the inertial substate, using a constrained ESKF or an equivalent conditional
Gaussian/block-Schur operation. This can preserve a declared covariance
meaning, but requires a new update path that knows which pose coordinates are
fixed and how the pose covariance is carried between epochs. The current
Update() always applies all 18 components and UpdateObserve() always solves
the full 18-D system, so D is not available by configuration alone. It is a
viable later architecture.

### E. BIEVR-style short-window inertial optimization

Keep accepted LiDAR poses in a state history, preintegrate IMU intervals, and
optimize velocity, shared biases, and gravity while making the endpoint pose
blocks constant. This gives an explicit factor-graph ownership contract and
matches the pinned BIEVR source. It requires state history, integrator
ownership, bias reintegration, window trimming/marginalization, and a
publication policy. Current Super-LIO has none of this short-window
infrastructure. It is viable, but more invasive than D.

## Decision boundary

For Prompt13, the viable directions are D (nearer to a filter-preserving
infrastructure) and E (clearer BIEVR-style semantics, recommended for a
later full implementation if the shadow demonstrates the mechanism). B and C
may be used only as explicitly labeled bounded experiments after their
uncertainty policy is specified. A is rejected for production asymmetric
authority and retained only as the no-mutation shadow rule.

No covariance strategy is selected or implemented in Prompt13.
