# Gate design

- one production mode: `dec_lio_final_candidate`
- rolling window: 20 successful plain LiDAR registrations
- fewer than 10 successful registrations: Native
- G0 enter/exit: weak fraction >= 0.75 / <= 0.60
- G1 enter: weak fraction >= 0.75 AND q median >= 0.30
- G1 exit: weak fraction <= 0.60 OR q median <= 0.20
- initial state: Native; hysteresis retains the previous state between thresholds
- weak flag: at least one DCReg rotational weak mode
- q: `r_w / max(p_w, 1e-15)` using the largest-covariance L1 rotational direction in the ESKF innovation tangent
- shadow registration production effect: NONE in Native frames
- loose branch: exact existing L1 pose-level ESKF update and its already computed covariance
