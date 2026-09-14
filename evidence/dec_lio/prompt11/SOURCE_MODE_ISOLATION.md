# Source and mode isolation

The estimator has one authoritative `paired_attenuation_mode`:

```text
0 OFF / Native
1 P1_DIRECTIONAL
2 U_TRACE
3 U_GAMMA
```

`ESKF::UpdateObserve` computes P1 diagnostics from raw H/b, calls
`makePairedControl(mode, raw_H, raw_b, p1)`, records the control fields, and
only then replaces the 6x6 H and 6x1 b when the selected control is valid and
active. U-trace/U-gamma reconstruct from raw H/b, never from P1 directional
H/b. The runner writes the single mode explicitly and records it in `meta.txt`.

The source audit passed: no GT/dataset branch exists in the production control
path; no map/correspondence/preprocessing branch was added; D3/PCG remains
off. The historical Prompt10 counterfactual fields remain diagnostic only and
are excluded from Prompt11 causal conclusions.
