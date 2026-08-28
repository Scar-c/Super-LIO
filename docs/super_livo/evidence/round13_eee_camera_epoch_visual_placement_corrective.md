# Round13 — camera-epoch Visual placement corrective (eee_01)

## Old failure / root cause

D-family IMU_ONLY camera epochs consumed (popped) the camera frame in
accountFullscanCamera before the scan-end V-4C/V-4A lifecycle could see it ->
Visual processed=0 (VISUAL_INACTIVE). Owner decision
ROUND13_EEE_DV0_PLACEMENT_FIX_AUTHORIZED.

## Corrective

- IMU_ONLY sync defers the pop (accountFullscanCameraNoPop); the frame stays
  owned through bookkeeping.
- statePropagateOnly (after PropagateTo(tc)) runs the V-4C lifecycle + V-4A
  sequential update with prior = latest committed posterior (LiDAR or prior
  Visual posterior) propagated to tc.
- releaseCameraPayload() terminally releases the payload exactly once;
  counters camera_visual_processed/rejected/payload_released.
- One full LiDAR geometry Observe per raw scan preserved (D-family invariant).

## Evidence (eee_01, transaction contract)

- D0_POST_FIX (Visual OFF, true D-family): camera_epoch=1;
  processed=0 rejected=1966 released=1966 (exact-once, OFF path);
  **ATE = 0.104098 m** (3981 rows).
- DV0_POST_FIX (Visual ON): camera_epoch=1;
  **processed=1966 rejected=0 released=1966**; VisualMap frames=5947;
  **ATE = 0.104204 m**.
- R_visual = 1.001 -> GREEN (<=1.10).
- LiDAR ownership/one-Observe-per-scan unchanged (s0 audit + scheduler code
  untouched).
- NOTE: earlier Round13 D0/DV0 runs were camera-off legacy (runner missed
  /camera/enabled + /lio/camera_epoch/enabled); their 0.1036 m ATE is NOT the
  D-family anchor. Corrected runner + reruns above.

## Caveat (next frontier)

VisualMap landmarks=0 (created=0) on eee: the lifecycle registers frames
(5947) and processes every camera epoch (1966) but creates no landmarks, so
the sequential update is zero-information (prior==posterior). Placement and
ownership gates PASS; the landmark-creation frontend is the next diagnostic.
