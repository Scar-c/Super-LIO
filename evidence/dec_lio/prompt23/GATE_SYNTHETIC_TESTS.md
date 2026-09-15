# Gate synthetic tests

`dec_lio_final_candidate_gate_test` passed after build with `catkin_make -C /home/lc/dec_lio -j4`.

Covered cases: warmup Native, G0 enter, G0 hysteresis retain and exit, G1 low-q rejection, G1 entry, G1 q exit, and failed registration not entering the rolling window or selecting L1 for that frame.
