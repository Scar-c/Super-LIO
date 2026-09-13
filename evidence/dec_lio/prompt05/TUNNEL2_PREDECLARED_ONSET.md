# Tunnel2 pre-declared onset comparison

The onset was frozen before consuming Prompt05 consistency results from the
Prompt04 six-window position-error analysis:

```text
start: 1706584541.828
end:   1706584579.030
```

Tunnel2 reference authority remains the supplied GEODE preprocessed position
reference (`Tunneling_tunnel2.txt`, SHA256
`b83ca5db4102838cd260fc78b0a20cca90da9db855ab9b762f9d7f2e5c856c5a`).  Its
quaternion fields are zero; rotational local error is not computed.

| lead before onset | pre median C_L / C_F | onset median C_L / C_F | pre median delta_C | onset median delta_C | pre median G/used | onset median G/used | pre max weak chi R | onset max weak chi R |
|---:|---|---|---:|---:|---:|---:|---:|---:|
| 1 s | 7.088 / 3.180 | 10.012 / 4.568 | 4.055 | 5.539 | 0.01120 | 0.02441 | 2.715 | 66.355 |
| 3 s | 6.225 / 2.646 | 10.012 / 4.568 | 3.819 | 5.539 | 0.00854 | 0.02441 | 2.715 | 66.355 |
| 5 s | 4.897 / 2.256 | 10.012 / 4.568 | 2.713 | 5.539 | 0.00558 | 0.02441 | 2.715 | 66.355 |

The rotational geometry maximum rises from approximately 69 before onset to
110.4 within onset; the translation maximum is 5.83 within onset.  Weak
rotational forcing rises sharply in the onset interval, with max `chi_R`
66.355 and max `psi_R` 33.432.  This supports a strong sustained rotational
degeneracy candidate, not a validated rotation-error causal chain.

The fixed onset evidence is also available in:

```text
/home/lc/dec_lio/runtime/prompt05/analysis/tunnel2.json
```
