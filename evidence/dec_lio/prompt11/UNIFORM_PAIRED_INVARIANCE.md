# Uniform paired invariance

For each uniform arm the exact same scalar multiplies both the information
matrix and right-hand side. Therefore the isolated LiDAR minimizer is
unchanged: `(alpha H)^-1(alpha b)=H^-1 b` for positive alpha. Only LiDAR
confidence relative to the propagated prior changes.

The Prompt11 synthetic executable tested arbitrary SPD H/b, exact identity
bypass, anisotropic P1-vs-U-trace separation, and invalid fail-open. T1-T10
passed. No H-only or b-only production branch exists.
