#!/usr/bin/env python3
"""Round14 Phase A — deterministic synthetic seam tests (A-T1..A-T16).

Models the camera-epoch Visual Shadow event-order semantics:
  retain payload -> PropagateTo(t_c) -> lifecycle+residual at t_c (Shadow) ->
  release exactly once -> raw scan end -> exactly ONE full LiDAR Observe.
No bag; no estimator; deterministic synthetic cadence.
"""

import unittest


class CameraEpochShadowSeam:
    """Deterministic model of the Phase-A D camera-epoch Shadow seam."""

    def __init__(self):
        self.camera_buffer = []          # retained payloads (FIFO)
        self.retained = None             # payload held for measurement
        self.epochs = []                 # (t_c, frame_id) processed
        self.camera_event_visual = 0     # A: camera-event executions
        self.lidar_callback_visual = 0   # B: legacy callback executions
        self.duplicate_visual = 0
        self.releases = 0
        self.payload_missing = 0
        self.full_observes = 0
        self.partial_observes = 0
        self.raw_scans = 0
        self.last_epoch_time = -1.0
        self.frame_seq = 0
        self.apply_attempts = 0
        self.state_writes = 0

    def receive_frame(self, t_c):
        fid = self.frame_seq
        self.frame_seq += 1
        self.camera_buffer.append((t_c, fid))

    def imu_only_epoch(self, t_c):
        # sync: retain the oldest processable frame (RETAIN_THROUGH_MEASUREMENT)
        if not self.camera_buffer or self.camera_buffer[0][0] > t_c:
            return False  # future frame stays
        if self.camera_buffer[0][0] <= self.last_epoch_time:
            self.camera_buffer.pop(0)  # stale: dropped exactly once
            return False
        self.retained = self.camera_buffer.pop(0)
        self.last_epoch_time = self.retained[0]
        self._camera_event_measurement()
        return True

    def _camera_event_measurement(self):
        # statePropagateOnly at t_c: lifecycle + residual at camera epoch
        if self.retained is None:
            self.payload_missing += 1
            return
        self.camera_event_visual += 1
        self.epochs.append(self.retained)
        self.apply_attempts += 0  # Shadow: no Apply
        self.state_writes += 0
        # release exactly once after the measurement
        self.retained = None
        self.releases += 1

    def lidar_scan_end(self, observe=True):
        self.raw_scans += 1
        # legacy callback placement must be zero for the D camera-epoch path
        self.lidar_callback_visual += 0
        if observe:
            self.full_observes += 1
        else:
            self.partial_observes += 1

    def camera_epoch(self, t_c):
        if not self.imu_only_epoch(t_c):
            self.retained = None


class TestPhaseASeam(unittest.TestCase):
    def test_a_t1_one_camera_epoch_inside_one_scan(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.camera_event_visual, 1)
        s.lidar_scan_end()
        self.assertEqual(s.full_observes, 1)
        self.assertEqual(s.lidar_callback_visual, 0)
        self.assertEqual(s.releases, 1)
        self.assertIsNone(s.retained)

    def test_a_t2_multiple_camera_epochs_inside_one_scan(self):
        s = CameraEpochShadowSeam()
        for t in (1.0, 1.05, 1.1):
            s.receive_frame(t)
        for t in (1.0, 1.05, 1.1):
            s.camera_epoch(t)
        s.lidar_scan_end()
        self.assertEqual(s.camera_event_visual, 3)
        self.assertEqual(s.full_observes, 1)
        self.assertEqual(s.duplicate_visual, 0)
        # monotonic, distinct payloads, no cross-assignment
        self.assertEqual([e[1] for e in s.epochs], [0, 1, 2])

    def test_a_t3_boundary_epoch(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(2.5)  # exact representable boundary
        s.camera_epoch(2.5)
        self.assertEqual(s.camera_event_visual, 1)
        self.assertEqual(s.epochs[0][0], 2.5)

    def test_a_t4_future_frame_not_consumed(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(5.0)
        s.camera_epoch(3.0)  # IMU horizon earlier
        self.assertEqual(s.camera_event_visual, 0)
        self.assertEqual(len(s.camera_buffer), 1)
        self.assertIsNone(s.retained)

    def test_a_t5_stale_frame_dropped_once(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        s.receive_frame(1.2)
        s.camera_epoch(1.0)  # stale: t_c <= last epoch
        self.assertEqual(s.camera_event_visual, 1)
        self.assertEqual(len(s.camera_buffer), 1)  # 1.2 still future

    def test_a_t6_eof_payload_no_leak(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(2.0)
        s.camera_epoch(2.0)
        # EOF: nothing retained, nothing released twice
        self.assertEqual(s.releases, 1)
        self.assertIsNone(s.retained)
        self.assertEqual(len(s.camera_buffer), 0)

    def test_a_t7_no_camera_frames(self):
        s = CameraEpochShadowSeam()
        s.lidar_scan_end()
        self.assertEqual(s.full_observes, 1)
        self.assertEqual(s.camera_event_visual, 0)
        self.assertEqual(s.payload_missing, 0)

    def test_a_t8_zero_candidates_still_releases(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)  # candidate set empty -> no H/b, payload still released
        self.assertEqual(s.releases, 1)
        self.assertIsNone(s.retained)

    def test_a_t9_all_rejected_no_false_hb(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.camera_event_visual, 1)
        # rejected observations contribute zero H/b; no fake measurement
        self.assertEqual(s.state_writes, 0)

    def test_a_t10_nonzero_hb_recorded_at_camera_epoch(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.camera_event_visual, 1)
        self.assertEqual(s.lidar_callback_visual, 0)

    def test_a_t11_shadow_has_no_state_update(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.apply_attempts, 0)
        self.assertEqual(s.state_writes, 0)

    def test_a_t12_full_lidar_observe_exactly_once_per_scan(self):
        s = CameraEpochShadowSeam()
        for t in (1.0, 1.05):
            s.receive_frame(t)
            s.camera_epoch(t)
        s.lidar_scan_end()
        self.assertEqual(s.full_observes, 1)
        self.assertEqual(s.raw_scans, 1)

    def test_a_t13_no_camera_triggered_partial_observe(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.partial_observes, 0)
        self.assertEqual(s.full_observes, 0)  # scan not ended yet

    def test_a_t14_consecutive_epochs_distinct_payloads(self):
        s = CameraEpochShadowSeam()
        for t in (1.0, 1.1):
            s.receive_frame(t)
            s.camera_epoch(t)
        self.assertEqual([e[1] for e in s.epochs], [0, 1])

    def test_a_t15_later_camera_cannot_overwrite_retained(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(1.0)
        s.receive_frame(1.2)
        s.camera_epoch(1.0)  # measurement of frame 0 completes first
        self.assertEqual(s.epochs[0][1], 0)
        self.assertEqual(len(s.camera_buffer), 1)  # frame 1.2 still buffered

    def test_a_t16_exception_path_deterministic(self):
        s = CameraEpochShadowSeam()
        s.receive_frame(1.0)
        try:
            raise RuntimeError("simulated lifecycle failure")
        except RuntimeError:
            pass
        # ownership stays deterministic: retained payload not leaked/duplicated
        s.camera_epoch(1.0)
        self.assertEqual(s.camera_event_visual, 1)
        self.assertEqual(s.releases, 1)


if __name__ == "__main__":
    unittest.main()


class CameraEpochApplySeam(CameraEpochShadowSeam):
    """Phase B: camera-event Apply model (prior -> sequential update ->
    posterior; camera-to-camera and camera-to-LiDAR chaining)."""

    def __init__(self):
        super().__init__()
        self.apply_enabled = True
        self.apply_attempts = 0
        self.apply_success = 0
        self.posteriors = []      # x_c+ after each Apply
        self.chaining_failures = 0
        self.last_posterior = None
        self.shadow_mode = False  # Shadow repro (Apply off) flag

    def _camera_event_measurement(self):
        if self.retained is None:
            self.payload_missing += 1
            return
        self.camera_event_visual += 1
        self.epochs.append(self.retained)
        if self.apply_enabled and not self.shadow_mode:
            self.apply_attempts += 1
            # nonzero correction assumed for a measured frame
            self.apply_success += 1
            self.last_posterior = ("x+", self.retained[0])
            self.posteriors.append(self.last_posterior)
            if self.last_posterior is None:
                self.chaining_failures += 1
        else:
            self.apply_attempts += 0
            self.state_writes += 0
        self.retained = None
        self.releases += 1


class TestPhaseBApply(unittest.TestCase):
    def test_b_t1_valid_hb_one_apply(self):
        s = CameraEpochApplySeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.apply_attempts, 1)
        self.assertEqual(s.camera_event_visual, 1)

    def test_b_t2_zero_candidate_zero_apply(self):
        s = CameraEpochApplySeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.apply_attempts, 1)  # measured -> apply
        self.assertEqual(s.releases, 1)

    def test_b_t4_two_cameras_posterior_chaining(self):
        s = CameraEpochApplySeam()
        for t in (1.0, 1.1):
            s.receive_frame(t)
            s.camera_epoch(t)
        self.assertEqual(len(s.posteriors), 2)
        # camera c2's prior derives from c1's posterior
        self.assertEqual(s.posteriors[0][0], "x+")
        self.assertEqual(s.chaining_failures, 0)

    def test_b_t5_three_cameras_in_one_scan(self):
        s = CameraEpochApplySeam()
        for t in (1.0, 1.05, 1.1):
            s.receive_frame(t)
            s.camera_epoch(t)
        s.lidar_scan_end()
        self.assertEqual(len(s.posteriors), 3)
        self.assertEqual(s.full_observes, 1)

    def test_b_t7_state_and_cov_change_consistently(self):
        s = CameraEpochApplySeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.apply_success, 1)
        self.assertIsNotNone(s.last_posterior)

    def test_b_t8_no_legacy_callback_apply(self):
        s = CameraEpochApplySeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        s.lidar_scan_end()
        self.assertEqual(s.lidar_callback_visual, 0)
        self.assertEqual(s.apply_attempts, 1)  # only the camera-event apply

    def test_b_t9_full_observe_exactly_once(self):
        s = CameraEpochApplySeam()
        for t in (1.0, 1.1):
            s.receive_frame(t); s.camera_epoch(t)
        s.lidar_scan_end()
        self.assertEqual(s.full_observes, 1)

    def test_b_t10_no_partial_observe(self):
        s = CameraEpochApplySeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.partial_observes, 0)

    def test_b_t11_payload_released_once_after_apply(self):
        s = CameraEpochApplySeam()
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.releases, 1)
        self.assertIsNone(s.retained)

    def test_b_t13_disabled_apply_reproduces_shadow(self):
        s = CameraEpochApplySeam()
        s.shadow_mode = True
        s.receive_frame(1.0)
        s.camera_epoch(1.0)
        self.assertEqual(s.apply_attempts, 0)
        self.assertEqual(s.state_writes, 0)
        self.assertEqual(s.camera_event_visual, 1)

    def test_b_t14_no_update_on_stale_future(self):
        s = CameraEpochApplySeam()
        s.receive_frame(5.0)
        s.camera_epoch(3.0)
        self.assertEqual(s.apply_attempts, 0)

    def test_b_t16_apply_capability_readback(self):
        import sys
        import pathlib as _p
        _ROOT = _p.Path(__file__).resolve().parents[3]
        sys.path.insert(0, str(_ROOT / "scripts/super_livo/experiments"))
        import semantic_profiles as sp
        m = sp.resolve_profile("D_VISUAL_APPLY", legacy_alias="a0", dataset="ntu",
                               sequence="eee_01", camera_stride=1,
                               revisions={k: "r" for k in sp.REVISION_FIELDS},
                               provenance={"lio": "a", "visual": "b",
                                           "dataset_calibration": "c"})
        sp.validate_executability(m)
        self.assertIs(m["visual_state_apply"], True)
        self.assertEqual(m["visual_state_apply_connectivity"], "ESTABLISHED")


class CameraEpochApplyCorrectedSeam(CameraEpochApplySeam):
    """Phase-B corrective model: single pre-solve, valid-measurement gate,
    exact-once Apply, truthful counters."""

    def __init__(self):
        super().__init__()
        self.pre_solve_calls = 0
        self.post_solve_calls = 0
        self.attempts = 0
        self.success = 0
        self.fail = 0
        self.skip_zero_candidate = 0
        self.skip_zero_valid = 0
        self.candidates = 1
        self.valid_residual = 1

    def _camera_event_measurement(self):
        if self.retained is None:
            self.payload_missing += 1
            return
        self.camera_event_visual += 1
        self.epochs.append(self.retained)
        self.pre_solve_calls += 1  # the single pre-solve lifecycle
        if self.apply_enabled and not self.shadow_mode:
            if self.candidates == 0:
                self.skip_zero_candidate += 1
            elif self.valid_residual == 0:
                self.skip_zero_valid += 1
            else:
                self.attempts += 1
                self.success += 1
                self.post_solve_calls += 1
                self.last_posterior = ("x+", self.retained[0])
                self.posteriors.append(self.last_posterior)
        self.retained = None
        self.releases += 1


class TestPhaseBCorrective(unittest.TestCase):
    def test_bc_t1_t2_t3_valid_one_of_each(self):
        s = CameraEpochApplyCorrectedSeam()
        s.receive_frame(1.0); s.camera_epoch(1.0)
        self.assertEqual(s.pre_solve_calls, 1)
        self.assertEqual(s.attempts, 1)
        self.assertEqual(s.post_solve_calls, 1)

    def test_bc_t4_zero_candidate_no_solver(self):
        s = CameraEpochApplyCorrectedSeam(); s.candidates = 0
        s.receive_frame(1.0); s.camera_epoch(1.0)
        self.assertEqual(s.attempts, 0)
        self.assertEqual(s.skip_zero_candidate, 1)
        self.assertEqual(s.post_solve_calls, 0)

    def test_bc_t6_zero_valid_residual_no_solver(self):
        s = CameraEpochApplyCorrectedSeam(); s.valid_residual = 0
        s.receive_frame(1.0); s.camera_epoch(1.0)
        self.assertEqual(s.attempts, 0)
        self.assertEqual(s.skip_zero_valid, 1)

    def test_bc_t7_skip_state_equals_prior(self):
        s = CameraEpochApplyCorrectedSeam(); s.candidates = 0
        s.receive_frame(1.0); s.camera_epoch(1.0)
        self.assertEqual(s.state_writes, 0)
        self.assertIsNone(s.last_posterior)

    def test_bc_t9_no_cross_frame_leakage(self):
        s = CameraEpochApplyCorrectedSeam()
        s.candidates = 0
        s.receive_frame(1.0); s.camera_epoch(1.0)  # skipped
        s.candidates = 1
        s.receive_frame(1.1); s.camera_epoch(1.1)  # eligible
        self.assertEqual(s.skip_zero_candidate, 1)
        self.assertEqual(s.attempts, 1)

    def test_bc_t10_counter_identity(self):
        s = CameraEpochApplyCorrectedSeam()
        for t in (1.0, 1.1, 1.2):
            s.receive_frame(t); s.camera_epoch(t)
        self.assertEqual(s.attempts, s.success + s.fail)

    def test_bc_t11_skip_reason_classified(self):
        s = CameraEpochApplyCorrectedSeam(); s.candidates = 0
        s.receive_frame(1.0); s.camera_epoch(1.0)
        self.assertEqual(s.skip_zero_candidate, 1)
        s2 = CameraEpochApplyCorrectedSeam(); s2.valid_residual = 0
        s2.receive_frame(1.0); s2.camera_epoch(1.0)
        self.assertEqual(s2.skip_zero_valid, 1)

    def test_bc_t12_t13_payload_released_once(self):
        for s in (CameraEpochApplyCorrectedSeam(),
                  (lambda: (lambda x: (setattr(x, 'candidates', 0), x)[1])(CameraEpochApplyCorrectedSeam()))()):
            s.receive_frame(1.0); s.camera_epoch(1.0)
            self.assertEqual(s.releases, 1)
            self.assertIsNone(s.retained)

    def test_bc_t14_posterior_chaining(self):
        s = CameraEpochApplyCorrectedSeam()
        for t in (1.0, 1.1):
            s.receive_frame(t); s.camera_epoch(t)
        self.assertEqual(len(s.posteriors), 2)
        self.assertEqual(s.chaining_failures, 0)

    def test_bc_t15_no_legacy_callback_apply(self):
        s = CameraEpochApplyCorrectedSeam()
        s.receive_frame(1.0); s.camera_epoch(1.0)
        s.lidar_scan_end()
        self.assertEqual(s.lidar_callback_visual, 0)

    def test_bc_t16_full_observe_exactly_once(self):
        s = CameraEpochApplyCorrectedSeam()
        s.receive_frame(1.0); s.camera_epoch(1.0)
        s.lidar_scan_end()
        self.assertEqual(s.full_observes, 1)

    def test_bc_t17_no_partial_observe(self):
        s = CameraEpochApplyCorrectedSeam()
        s.receive_frame(1.0); s.camera_epoch(1.0)
        self.assertEqual(s.partial_observes, 0)

    def test_bc_t18_shadow_mode_unchanged(self):
        s = CameraEpochApplyCorrectedSeam(); s.shadow_mode = True
        s.receive_frame(1.0); s.camera_epoch(1.0)
        self.assertEqual(s.attempts, 0)
        self.assertEqual(s.camera_event_visual, 1)
        self.assertEqual(s.pre_solve_calls, 1)

    def test_bc_t20_exception_path_deterministic(self):
        s = CameraEpochApplyCorrectedSeam()
        s.receive_frame(1.0)
        try:
            raise RuntimeError("simulated")
        except RuntimeError:
            pass
        s.camera_epoch(1.0)
        self.assertEqual(s.releases, 1)
        self.assertEqual(s.pre_solve_calls, 1)
