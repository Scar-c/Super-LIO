# Prompt14 zero-side-effect shadow design

## Feasibility

Prompt14 is feasible as a diagnostic-only phase, subject to the implementation
surface below. It must not promote the shadow pose.

## Exact insertion surface

Place the shadow after Propagation_Undistort() and DownSample(), before the
native ESKF::UpdateObserve() can change the state and before UpdateMap() can
insert the current scan. The shadow inputs are:

    T_init        kf_->GetSE3() immediately before native observation update
    cloud         ds_undistort_ and its current point order
    map           pre-update ivox point set / read-only query view
    geometry      native plane/HKNN and point-to-plane semantics
    H_L,b_L       native first-iteration matched linearization

The implementation should factor or wrap the geometry path into a read-only
query/registration helper. Local masks, correspondences, Jacobians, pose
iterates, and objective accumulators belong to the shadow. If the existing
ivox_ API has mutable query bookkeeping, use an immutable snapshot or a
read-only adapter so the shadow cannot affect native correspondence state.

## Two non-equivalent diagnostics

1. Matched linearization: at exactly T_init, reuse exactly the native accepted
   points, H_L, and b_L. Solve only the six-dimensional LiDAR increment and
   compare it with the leading six entries of the native tight dx. This
   isolates prior suppression.
2. Nonlinear LiDAR-only: initialize a local SE(3) variable at T_init, recompute
   map correspondences and residuals, relinearize, and iterate only J_L(T).
   Record its objective and validity. This measures standalone registration
   behavior and must not be read as a pure prior decomposition.

## Hard parity invariants

With shadow disabled:

    native trajectory: byte-identical
    native map:        unchanged
    native P:          unchanged
    native state:      unchanged

With shadow enabled, the same four production artifacts must still be
identical to the disabled control. Only diagnostic files/log records may
differ. The shadow must not call state setters, covariance setters, map
insertion, output publication, or native mask mutation.

Prompt14 should use only already-available canonical local data: one
well-behaved sequence and one known-degeneracy sequence. It must not download
data, retune parameters, or alter dataset configuration.
