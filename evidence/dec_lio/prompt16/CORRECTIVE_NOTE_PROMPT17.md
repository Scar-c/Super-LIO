# Prompt17 corrective note for Prompt16

Prompt16's classification **P16-C** and its asymmetric estimator interpretation are
superseded and are not scientifically authoritative. The Prompt16 numerical files
remain preserved as historical evidence; they are not deleted or rewritten.

Prompt17 identified two correctness defects that invalidate the old asymmetric
interpretation:

1. The inertial factor mixed Super-LIO's physical world gravity convention
   (`g_W ~= [0, 0, -g]`, with `a_W = R a_body + g_W`) with BIEVR's opposite-sign
   predictor variable (`G = -g_W`). Its position and velocity residuals therefore
   added, rather than subtracted, the physical gravity contribution.
2. The Prompt16 evaluator reported raw world-frame displacement differences for
   RPE and segment errors, which is not invariant to a constant global rotation
   between trajectories. It also presented a native-fit comparison as though it
   were the only APE frame.

Prompt17 corrects the residual signs, anchors the gravity-direction diagnostic to
the actual initialized physical gravity direction, adds deterministic residual
and BIEVR-parity tests, and replaces RPE/segment scoring with relative SE(3)
translation error. It reports independent branch alignment separately from the
explicit common-frame comparison.

All Prompt16 values are retained for auditability, but no scientific conclusion
from the old P16-C label should be used after this note.
