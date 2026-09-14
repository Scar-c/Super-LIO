# Prompt09 authority correction

The historical Prompt09 raw classification was D3-E because its strict
numerical gate compared native float inverse arithmetic with a double shadow.
That was an over-strict numerical interpretation, not an algebraic failure:

`A dx = b - Lambda d` is algebraically equivalent to the native information
form update when evaluated in the same arithmetic domain.

The corrected engineering authority is D3-C: direct native LDLT is preferred;
the PCG reformulation is not worthwhile as a production path. Historical
microbenchmark medians were approximately 2.09 us (direct LDLT), 4.69 us
(Jacobi-PCG), and 16.15 us (DCReg-PCG), with DCReg-PCG about 7.7x slower than
direct LDLT. No PCG production path was added.

old classification: D3-E (strict float/double shadow gate)
corrected engineering classification: D3-C
raw Prompt09 evidence rerun: NO
Prompt10 production D3 solver shadow: OFF
