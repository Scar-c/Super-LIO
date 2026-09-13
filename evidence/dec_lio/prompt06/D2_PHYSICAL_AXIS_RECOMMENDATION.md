# D2 physical-axis recommendation

Scientific primary classification:

`C — TUNNEL2_WEAK_ROTATION_IS_LONGITUDINAL_ROLL_LIKE`

Supporting observations:

* Tunnel2 full/onset weak rank is predominantly 1.
* `O_long` is about 0.992 full and 0.970 at onset; `O_yaw` is about
  0.00085 full and 0.00109 at onset.
* The same right/local decomposition on Stairs also has near-zero yaw
  occupancy, so the physical axis is not a Tunnel2-only yaw mechanism.
* Prompt05 forcing peaks exceed the Stairs P95 only briefly; they are an
  annotation, not a persistent transferable gate signal.

D2 authority outcome:

`D2-PHYS-2 — yaw rejected; generic geometry/consistency`

No estimator gate is authorized.  A directional gate, yaw-specific gate, or
gain modification would require held-out full-pose tunnel evidence first.
