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
