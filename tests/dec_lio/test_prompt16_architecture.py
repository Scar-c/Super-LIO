#!/usr/bin/env python3
"""Static contracts for the Prompt16 asymmetric architecture."""

import pathlib
import unittest


class Prompt16ArchitectureTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.repo = pathlib.Path(__file__).resolve().parents[2]
        cls.super_lio = (cls.repo / "src/super_lio/src/lio/super_lio.cpp").read_text(
            encoding="utf-8"
        )
        cls.estimator = (
            cls.repo / "src/super_lio/src/dec_lio/AsymmetricEstimator.cpp"
        ).read_text(encoding="utf-8")

    def test_mode_defaults_native_and_has_explicit_asymmetric_dispatch(self):
        wrapper = (self.repo / "src/super_lio/src/ros/ROSWrapper.cpp").read_text(
            encoding="utf-8"
        )
        self.assertIn('g_estimator_mode, std::string("native")', wrapper)
        self.assertIn('g_estimator_mode == "asymmetric"', self.super_lio)
        self.assertIn("ObserveAsymmetric();", self.super_lio)

    def test_asymmetric_observation_bypasses_native_tight_update(self):
        start = self.super_lio.index("void SuperLIO::ObserveAsymmetric()")
        end = self.super_lio.index("void SuperLIO::Observe(){", start)
        branch = self.super_lio[start:end]
        self.assertNotIn("kf_->UpdateObserve", branch)
        self.assertIn("acceptPose", branch)
        self.assertIn("acceptPredictedPose", branch)

    def test_registration_is_lidar_only_and_rank_safe(self):
        solve_start = self.estimator.index(
            "AsymmetricRegistrationResult AsymmetricLidarRegistration::solve"
        )
        solve = self.estimator[solve_start:]
        self.assertIn("solveFullRank", solve)
        self.assertIn("REGISTRATION_RANK_FAILURE", solve)
        self.assertNotIn("GetCov", solve)
        self.assertNotIn("UpdateObserve", solve)
        self.assertNotIn("P_pred", solve)
        self.assertNotIn("inverse()", solve)

    def test_map_insertion_uses_accepted_asymmetric_pose(self):
        start = self.super_lio.index("void SuperLIO::UpdateMap()")
        end = self.super_lio.index("void SuperLIO::Output()", start)
        update_map = self.super_lio[start:end]
        self.assertIn("asymmetric_estimator_ ? asymmetric_estimator_->pose()", update_map)
        self.assertIn("ivox_->insert", update_map)

    def test_fixed_pose_is_not_an_inertial_parameter(self):
        optimizer = self.estimator[self.estimator.index(
            "bool AsymmetricEstimator::optimizeInertialWindow()"
        ):]
        self.assertIn("segment->R_i", self.estimator)
        self.assertIn("segment->p_i", self.estimator)
        self.assertIn("velocities[i].data()", optimizer)
        self.assertNotIn("segment->R_i.R_.data()", optimizer)
        self.assertNotIn("segment->p_i.data()", optimizer)


if __name__ == "__main__":
    unittest.main()
