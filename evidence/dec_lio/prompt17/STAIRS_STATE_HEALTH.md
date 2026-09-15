# Corrected asymmetric Stairs state health

Authoritative runs:

- A1: `/home/lc/dec_lio/runtime/prompt17/asymmetric/stairs_asym_a1_retry`
- A2: `/home/lc/dec_lio/runtime/prompt17/asymmetric/stairs_asym_a2`

Both outputs have 3446 trajectory rows and the same trajectory SHA256
`5b5b5d14f28b133b81c3840f3f5da282c9328eaf2cd61bc9543378bb3c8008af`.

Across each 3446-row diagnostics file:

- all tracked state-health fields are finite;
- velocity norm max: `2.0539326668`;
- gyro-bias norm max: `0.0470476225`;
- accel-bias norm max: `1.0428944826`; final: `0.0734317228`;
- gravity norm range: `[9.7945985794, 9.7946004868]`;
- gravity-direction error max: `16.683519°`; final: `12.886977°`;
- inertial attempted: `3442`, successful: `3442`, failures: `0`;
- canonical frames: `3433/3446`.

The old Prompt16 approximately `169°` gravity flip and hundred-scale bias
pathology do not recur after the sign correction. The remaining 13 registration
divergence events are recorded in the ICP audit and are not silently replaced
by the native estimator.
