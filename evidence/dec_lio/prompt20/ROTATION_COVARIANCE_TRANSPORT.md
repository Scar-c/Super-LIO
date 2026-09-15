# Prompt20 rotation covariance transport

The registration perturbation is right/local at the raw LiDAR pose:

```text
R_L(epsilon) = R_L Exp(epsilon)
```

The EKF innovation rotation is prior-relative:

```text
z_R(epsilon) = Log(R_-^T R_L Exp(epsilon))
```

The implementation evaluates the central finite-difference Jacobian with
`h=1e-7` for each axis and maps `Sigma_R,reg` as
`J Sigma_R,reg J^T`. The result is symmetrized and checked finite before it
is inserted into the pose covariance. The synthetic test checks generic
non-identity prior/measurement rotations and PSD output.

