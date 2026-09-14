# Prompt11 control math

All arms begin each IESKF iteration from the same native raw `H_L=HTVH` and
`b_L=HTVr`. The existing Prompt10 P1 diagnostic is computed first.

* `0 OFF`: native raw `H_L,b_L`.
* `1 P1_DIRECTIONAL`: Prompt10 `attenuated_H,attenuated_b`.
* `2 U_TRACE`: `alpha_trace=trace(H_P1)/trace(H_L)` for a valid nonzero trace,
  then `H=alpha_trace*H_L`, `b=alpha_trace*b_L`.
* `3 U_GAMMA`: `H=gamma_w*H_L`, `b=gamma_w*b_L`.

No GT, scene label, trajectory error, onset time, history classifier, chi,
Psi, G/N, or map state enters either scalar. Invalid P1 diagnostics fail open
to the raw pair. `alpha_trace=1` and `gamma_w=1` are exact bypass cases.

The authoritative ROS parameter is the single integer
`/lio/dec_lio/paired_attenuation/mode`; default is `0`.
