# Fused system derivation

For each native IESKF iteration the shadow records the exact native matrices:

```text
Lambda = Pk^-1
Hbar   = HTRH, with only the pose 6x6 block populated
A      = Lambda + Hbar
b      = [HTVr; 0]
r      = b - Lambda * dx_prior
A * dx = r
```

The identity used is `Q*b + (Q*Hbar-I)*d = Q*(b-Lambda*d)`. `A`, `r`, and the
objective are not changed by DCReg. The DCReg adapter constructs only a left
preconditioner `M`; final PCG residuals are evaluated against the original
fused `A` and `r`.

The v2 implementation also carries the original LiDAR `HTVr` separately so
the DCReg characterization cannot accidentally use the fused RHS.
