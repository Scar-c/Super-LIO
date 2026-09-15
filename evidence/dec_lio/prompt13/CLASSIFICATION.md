# Prompt13 classification

## Primary decision

**P13-B — FEASIBLE_WITH_MAJOR_STATE_REDESIGN**

## Basis

The BIEVR-style ownership split is semantically implementable: an IMU
propagated pose can seed deskew and registration, LiDAR can solve a standalone
6-DoF geometry objective, and a later inertial update can hold pose blocks
fixed. The pinned BIEVR source demonstrates this separation.

The current Super-LIO implementation, however, is not a drop-in host for that
split. Its native callback exposes geometry terms but not a standalone
nonlinear pose optimizer; ESKF::UpdateObserve() solves a coupled 18-D
prior-plus-LiDAR system; Update() writes pose and all inertial substates; and
one full covariance carries the pose/inertial cross blocks. A production
external pose acceptance would therefore require a substantial state,
covariance, and/or history redesign.

This is not P13-C: there is no fundamental semantic conflict requiring
replacement of the estimator. It is P13-B because the clean asymmetric
authority path is possible but substantially invasive in the current
architecture.

## Staged roadmap authority

    Prompt13  architecture, state ownership, covariance audit       COMPLETE
    Prompt14  LiDAR-only zero-side-effect shadow                     NEXT
    Prompt15  experimental LiDAR pose authority, bounded substate    LATER
    Prompt16  fixed-pose inertial v/bg/ba/g consistency               LATER
    Prompt17  hybrid nullspace treatment only if measurements need   CONDITIONAL

Prompt14 must precede any production pose-authority change. No Prompt13
performance or benchmark claim is made.
