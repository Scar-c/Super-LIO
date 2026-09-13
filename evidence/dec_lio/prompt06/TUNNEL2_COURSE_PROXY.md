# Tunnel2 trajectory course proxy

This is labelled `TRAJECTORY COURSE-DIRECTION ERROR`; it is a course/heading
divergence proxy only.  Tunnel2's supplied reference contains no valid
quaternion attitude, so no GT attitude error is claimed.

Definition:

* one global SE(3) alignment, no per-window realignment;
* primary centered linear-fit half-window 1.0 s (2.0 s duration);
* robustness half-windows 0.5/1.0/2.0 s;
* speed thresholds 0.05/0.10/0.20 m/s;
* horizontal projection uses the captured filter gravity direction;
* signed error is `atan2(g_hat dot (v_GT,h x v_est,h), v_GT,h dot v_est,h)`;
* reject only insufficient fit samples, invalid fit, or low horizontal speed;
  high-curvature records are not selectively removed.

Primary threshold 0.10 m/s, half-window 1.0 s:

| interval | valid records | coverage of 2739 axis rows | signed median (deg) | absolute median / P90 / max (deg) | GT speed median (m/s) | GT fit RMSE median (m) |
|---|---:|---:|---:|---:|---:|---:|
| pre5 | 45 | 1.64% | 12.8106 | 12.8106 / 14.8851 / 15.2044 | 0.456660 | 0.0338247 |
| pre3 | 30 | 1.10% | 13.3892 | 13.3892 / 14.9625 / 15.2044 | 0.617440 | 0.0306636 |
| pre1 | 10 | 0.365% | 14.7993 | 14.7993 / 15.0558 / 15.2044 | 0.643408 | 0.0287452 |
| onset | 273 | 9.97% | 12.0566 | 13.5235 / 146.642 / 178.357 | 0.838852 | 0.0550607 |

Full primary coverage is 876/2739 = 31.98%, with absolute median/P90/max
4.32853/31.0486/178.357 deg.  The 178-degree tail is retained because the
rule does not reject high curvature selectively.  These values remain a
diagnostic proxy only and are not used to authorize a yaw causal chain.

GT attitude claimed: MUST BE NO.
