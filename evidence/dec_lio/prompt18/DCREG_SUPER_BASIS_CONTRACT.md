# DCReg / Super-LIO basis contract

The exact Prompt17 Super registration basis is preserved:

```text
delta[0:3] = right/local/body rotational tangent
delta[3:6] = additive world translation xyz
```

In `AsymmetricEstimator.cpp`, the corresponding point-to-plane Jacobian is

```text
J_rot  = point_body x (R^T normal_world)
J_trans = normal_world
```

and the existing pose update is

```text
R_new = R * Exp(delta_rot)
t_new = t + delta_trans
```

Therefore the DCReg Schur blocks are formed on this exact matrix:

```text
H = [ A  B ]
    [ B^T D ]

S_rot   = A - B D^-1 B^T
S_trans = D - B^T A^-1 B
```

Diagnostics use the unambiguous labels
`rot_tangent_x/y/z` and `trans_world_x/y/z`. They are not global roll/pitch/yaw
labels. Canonical unit axes are applied only in this six-component Super basis
after Schur decoupling.

The following are frozen between A0 and A1: HKNN correspondences, residuals,
weights, H, b, outer iteration count, backtracking, convergence gate, and pose
update. Only the inner linear solver changes.
