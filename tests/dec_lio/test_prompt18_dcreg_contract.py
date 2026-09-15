import pathlib
import unittest


ROOT = pathlib.Path(__file__).parents[2]
SOURCE = (ROOT / "src/super_lio/src/dec_lio/AsymmetricEstimator.cpp").read_text()
RUNNER = (ROOT / "tools/dec_lio/run_baseline.sh").read_text()


class Prompt18DCRegContractTest(unittest.TestCase):
    def test_default_solver_is_plain_and_dcreg_receives_exact_h_b(self):
        self.assertIn('ASYMMETRIC_REGISTRATION_SOLVER="plain"', RUNNER)
        self.assertIn('registration_solver "$ASYMMETRIC_REGISTRATION_SOLVER"',
                      RUNNER)
        self.assertIn('DCRegCore::solve(h, b, parameters)', SOURCE)
        self.assertIn('accumulateRegistration(result.pose, points, h, b)',
                      SOURCE)

    def test_no_prior_or_spectral_attenuation_in_dcreg_registration_path(self):
        self.assertNotIn("P^-1", SOURCE)
        self.assertNotIn("paired_attenuation", SOURCE)
        self.assertNotIn("DCRegAnalyzer::", SOURCE)


if __name__ == "__main__":
    unittest.main()
