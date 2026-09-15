# Fixed-pose inertial update options

The target after a future LiDAR registration is:

    R,p     fixed at the accepted LiDAR pose
    v,bg,ba,g updated by the inertial subsystem

## Option 1: constrained minimal ESKF update

### Required state and history

Retain the current nominal state and covariance, but add an explicit update
mode with a fixed-pose mask. The update must define whether the accepted pose
uncertainty is conditioned on, injected as a measurement, or carried as a
separate pose covariance. It must operate on the inertial substate and its
cross blocks rather than silently deleting them.

### IMU requirements

The existing Predict() Jacobian/noise propagation is reusable in principle.
The new update needs the pose-fixed linearized constraint and a stable
conditioning/block-Schur calculation for v,bg,ba,g, including rotation
manifold/reset handling and a policy for the next pose propagation.

### Cost and invasiveness

Per LiDAR epoch this remains a small fixed-size matrix operation and is less
expensive than a window solver. In the current source it is nevertheless
substantially invasive: UpdateObserve() embeds LiDAR information in an 18-D
solve and Update() writes every state group. New ESKF APIs, covariance block
lifecycle, external-pose installation, and tests are required.

### Consistency

It can be consistent if the fixed-pose constraint, pose covariance, cross
covariance, and reset convention are all explicit. A direct R,p assignment
followed by the current Predict() is not that implementation.

## Option 2: BIEVR-style short-window IMU optimization

### Required state and history

Maintain timestamped pose/velocity states and preintegrated IMU intervals.
Maintain shared or time-varying bg, ba, and gravity parameters according to
the chosen model. The accepted LiDAR pose enters the history directly; the
inertial factors connect consecutive states.

### IMU requirements

Use preintegration with bias Jacobians, gravity, and reintegration when bias
changes. The pinned BIEVR implementation uses ImuIntegrator, a 9-residual
InertialFactor, shared accelerometer/gyro bias and gravity blocks, and a
short configurable window.

### Cost and invasiveness

The optimization is larger and more expensive than a fixed-size ESKF update,
with nonlinear solver overhead and window maintenance. It is architecturally
more invasive for Super-LIO because no equivalent state-history, factor,
reintegration, or marginalization layer currently exists.

### Consistency

This is the clearest ownership separation: LiDAR owns pose parameters while
the inertial factors update the remaining parameters. It still requires a
precise rule for the covariance published to the filter/output and for
anchoring/gravity observability; “pose fixed” alone does not solve every
uncertainty question.

## Recommendation

Prompt14 must first measure the mechanism with a no-side-effect shadow.
If conversion is justified, evaluate Option 1 against Option 2 in a later
design gate. Option 1 is the smaller infrastructure step; Option 2 is the
recommended semantic target for a BIEVR-style long-lived asymmetric mode
because it makes fixed pose blocks and optimized inertial blocks explicit.
Neither option is implemented here.
