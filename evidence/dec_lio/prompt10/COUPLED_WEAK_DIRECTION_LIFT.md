# Coupled weak-direction lift

For `H = [[A,B],[B^T,D]]`, raw DCReg Schur eigenvectors are used. A weak
rotational vector is lifted as

`d_R = [u_R; -D^{-1} B^T u_R]`.

A weak translational vector is lifted as

`d_t = [-A^{-1} B u_t; u_t]`.

The implementation solves with `FullPivLU` rather than forming explicit
inverse blocks, normalizes the columns, and uses dynamic SVD rank detection.
Synthetic T2/T3/T20 verify nonzero B coupling; T4/T5 verify a finite union
projector and no duplicate rank counting.
