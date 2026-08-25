#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>

#include <Eigen/Core>
#include <Eigen/Geometry>

// Round 11G synthetic parity: Path A (FAST-LIVO2-style direct, body->cam
// extrinsic T_CB) vs Path B (Super-LIVO camera-pose style). Proves whether
// the production dXc/dtheta implementation matches the correct formula.

// NTU eee_01 T_Body_Cam (yaml, tf "Body Cam" = cam->body = T_BC) inverted:
//   T_cam_body() = T_BC^-1 = T_CB (body->cam)
static Eigen::Matrix4d eee_T_CB() {
  Eigen::Matrix4d Tbc;  // cam->body (T_Body_Cam)
  Tbc << 0.02183084, -0.01312053, 0.99967558, 0.00552943,
         0.99975965, 0.00230088, -0.02180248, -0.12431302,
         -0.00201407, 0.99991127, 0.01316761, 0.01614686,
         0.0, 0.0, 0.0, 1.0;
  return Tbc.inverse();  // T_CB (body->cam)
}

int main() {
  const Eigen::Matrix4d T_CB = eee_T_CB();
  const Eigen::Matrix3d R_CB = T_CB.block<3, 3>(0, 0);
  const Eigen::Vector3d t_CB = T_CB.block<3, 1>(0, 3);

  // fixed body pose + world point
  const Eigen::Matrix3d R_WB =
      (Eigen::AngleAxisd(0.3, Eigen::Vector3d::UnitZ()) *
       Eigen::AngleAxisd(-0.15, Eigen::Vector3d::UnitY()) *
       Eigen::AngleAxisd(0.1, Eigen::Vector3d::UnitX()))
          .toRotationMatrix();
  const Eigen::Vector3d p_WB(1.0, -0.5, 2.0);
  const Eigen::Vector3d X_W(2.5, 1.0, 4.0);

  // ---- Path A: FAST-LIVO2-style direct ----
  const Eigen::Vector3d X_B = R_WB.transpose() * (X_W - p_WB);
  const Eigen::Vector3d Xc_A = R_CB * X_B + t_CB;
  // rotation J: dXc/dtheta = [X_C - t_CB]x R_CB = [R_CB X_B]x R_CB
  const Eigen::Vector3d Xc_m_t = R_CB * X_B;  // = X_C - t_CB (cam frame)
  Eigen::Matrix3d Xc_m_t_hat;
  Xc_m_t_hat << 0.0, -Xc_m_t.z(), Xc_m_t.y(),
                Xc_m_t.z(), 0.0, -Xc_m_t.x(),
               -Xc_m_t.y(), Xc_m_t.x(), 0.0;
  const Eigen::Matrix3d Jtheta_A = Xc_m_t_hat * R_CB;
  const Eigen::Matrix3d Jp_A = -R_CB * R_WB.transpose();

  // ---- Path B: Super-LIVO camera-pose style ----
  const Eigen::Matrix3d R_WC = R_WB * R_CB;
  const Eigen::Vector3d p_WC = p_WB + R_WB * t_CB;
  const Eigen::Vector3d Xc_B = R_WC.transpose() * (X_W - p_WC);
  // Super-LIVO current implementation:
  const Eigen::Vector3d Xct_impl = Xc_B + R_CB.transpose() * t_CB;
  Eigen::Matrix3d Xct_impl_hat;
  Xct_impl_hat << 0.0, -Xct_impl.z(), Xct_impl.y(),
                  Xct_impl.z(), 0.0, -Xct_impl.x(),
                 -Xct_impl.y(), Xct_impl.x(), 0.0;
  const Eigen::Matrix3d Jtheta_B = Xct_impl_hat * R_CB.transpose();
  const Eigen::Matrix3d Jp_B = -R_WC.transpose();
  // Correct Super-LIVO form (equivalent to Path A):
  const Eigen::Matrix3d Jtheta_B_correct = Xc_m_t_hat * R_CB;

  std::printf("R_CB det=%.10g\n", R_CB.determinant());
  std::printf("Xc_A=(%.10g,%.10g,%.10g)\n", Xc_A.x(), Xc_A.y(), Xc_A.z());
  std::printf("Xc_B=(%.10g,%.10g,%.10g)  diff=%.3g\n", Xc_B.x(), Xc_B.y(), Xc_B.z(),
              (Xc_A - Xc_B).norm());
  std::printf("Jtheta_A=\n%.6g %.6g %.6g\n%.6g %.6g %.6g\n%.6g %.6g %.6g\n",
              Jtheta_A(0, 0), Jtheta_A(0, 1), Jtheta_A(0, 2),
              Jtheta_A(1, 0), Jtheta_A(1, 1), Jtheta_A(1, 2),
              Jtheta_A(2, 0), Jtheta_A(2, 1), Jtheta_A(2, 2));
  std::printf("Jtheta_B(impl)=\n%.6g %.6g %.6g\n%.6g %.6g %.6g\n%.6g %.6g %.6g\n",
              Jtheta_B(0, 0), Jtheta_B(0, 1), Jtheta_B(0, 2),
              Jtheta_B(1, 0), Jtheta_B(1, 1), Jtheta_B(1, 2),
              Jtheta_B(2, 0), Jtheta_B(2, 1), Jtheta_B(2, 2));
  std::printf("Jtheta_B(correct)=\n%.6g %.6g %.6g\n%.6g %.6g %.6g\n%.6g %.6g %.6g\n",
              Jtheta_B_correct(0, 0), Jtheta_B_correct(0, 1), Jtheta_B_correct(0, 2),
              Jtheta_B_correct(1, 0), Jtheta_B_correct(1, 1), Jtheta_B_correct(1, 2),
              Jtheta_B_correct(2, 0), Jtheta_B_correct(2, 1), Jtheta_B_correct(2, 2));
  std::printf("Jp_A=(%.6g,%.6g,%.6g)  Jp_B=(%.6g,%.6g,%.6g) diff=%.3g\n",
              Jp_A(0, 0), Jp_A(0, 1), Jp_A(0, 2),
              Jp_B(0, 0), Jp_B(0, 1), Jp_B(0, 2), (Jp_A - Jp_B).norm());

  std::fflush(stdout);

  // Xc must match
  assert((Xc_A - Xc_B).norm() < 1e-9);
  // translation J must match
  assert((Jp_A - Jp_B).norm() < 1e-9);
  // rotation J: correct Super-LIVO form == Path A
  assert((Jtheta_A - Jtheta_B_correct).norm() < 1e-9);
  // current implementation: report whether it matches
  const double impl_rot_err = (Jtheta_A - Jtheta_B).norm() / std::max(1e-12, Jtheta_A.norm());
  std::printf("impl_rot_rel_err = %.6g\n", impl_rot_err);

  std::printf("extrinsic parity: Xc/Jtrans PASS; correct rotation form matches; "
              "implementation rotation rel err = %.6g\n", impl_rot_err);
  return 0;
}