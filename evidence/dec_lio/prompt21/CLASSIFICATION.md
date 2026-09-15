# Prompt21 classification

## P21-B — DIRECTIONALITY_PARTIALLY_SUPPORTED

The confidence-budget and implementation gates pass:

- L2 is constructed from the exact L1 covariance;
- rotation and translation traces match within each frame to double precision;
- L1/L2 covariance blocks are finite and PSD;
- both modes use the same DCReg analysis boundary and both have zero runtime
  covariance fallbacks;
- Native, L0, and L1 Alpha identity gates are byte-identical;
- all L2 state-health runs are finite with positive covariance spectra.

The controlled L1-versus-L2 result is sequence-dependent:

```text
Stairs:       L1 0.474189 m  < L2 5.640262 m  (directional benefit)
Tunnel2 Alpha:L1 2.519871 m  < L2 2.668083 m  (small directional benefit)
Tunnel2 Gamma:L1 1.414167 m  > L2 1.078247 m  (scalar control is better)
```

Therefore DCReg directional shaping is supported on Stairs and slightly on
Alpha, but not uniformly across the tunnel pair. Prompt21 does not justify
P21-A or P21-D. The result is not P21-C because L2 does not approximately
reproduce L1 on Stairs and Alpha.
