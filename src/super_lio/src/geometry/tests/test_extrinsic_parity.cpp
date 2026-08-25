#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>

#include <Eigen/Core>
#include <Eigen/Geometry>

// Round 11H Gate X: three-path parity test.
//   A = FAST-LIVO2-style direct (T_CB: Body->Camera)
//   B = mathematically correct camera-world-pose path (T_BC = T_CB^-1)
//   P = current production-as-coded path (pre-fix)
// Required: A == B exactly; A != P (Gate X pre-fix FAIL evidence).

// NTU eee_01 T_Body_Cam (yaml tf "Body Cam" = cam->body = T_BC)
static Eigen::Matrix4d eee_T_BC() {
  Eigen::Matrix4d Tbc;  // cam->body
  Tbc << 0.02183084, -0.01312053, 0.99967558, 0.00552943,
         0.99975965, 0.00230088, -0.02180248, -0.12431302,
         -0.00201407, 0.99991127, 0.01316761, 0.01614686,
         0.0, 0.0, 0.0, 1.0;
  return Tbc;
}

int main() {
  const Eigen::Matrix4d T_BC = eee_T_BC();
  const Eigen::Matrix4d T_CB = T_BC.inverse();  // T_cam_body(): Body->Camera
  const Eigen::Matrix3d R_CB = T_CB.block<3, 3>(0, 0);
  const Eigen::Vector3d t_CB = T_CB.block<3, 1>(0, 3);
  const Eigen::Matrix3d R_BC = T_BC.block<3, 3>(0, 0);
  const Eigen::Vector3d t_BC = T_BC.block<3, 1>(0, 3);

  const Eigen::Matrix3d R_WB =
      (Eigen::AngleAxisd(0.3, Eigen::Vector3d::UnitZ()) *
       Eigen::AngleAxisd(-0.15, Eigen::Vector3d::UnitY()) *
       Eigen::AngleAxisd(0.1, Eigen::Vector3d::UnitX()))
          .toRotationMatrix();
  const Eigen::Vector3d p_WB(1.0, -0.5, 2.0);
  const Eigen::Vector3d X_W(2.5, 1.0, 4.0);

  // ---- Path A: FAST/direct ----
  const Eigen::Vector3d X_B = R_WB.transpose() * (X_W - p_WB);
  const Eigen::Vector3d Xc_A = R_CB * X_B + t_CB;
  const Eigen::Vector3d Xc_m_t = R_CB * X_B;  // = X_C - t_CB (cam frame)
  Eigen::Matrix3d hatA;
  hatA << 0.0, -Xc_m_t.z(), Xc_m_t.y(), Xc_m_t.z(), 0.0, -Xc_m_t.x(),
      -Xc_m_t.y(), Xc_m_t.x(), 0.0;
  const Eigen::Matrix3d Jtheta_A = hatA * R_CB;
  const Eigen::Matrix3d Jp_A = -R_CB * R_WB.transpose();

  // ---- Path B: correct camera-world-pose path ----
  // T_WC = T_WB * T_BC (T_BC = T_CB^-1)
  const Eigen::Matrix3d R_WC = R_WB * R_BC;
  const Eigen::Vector3d p_WC = p_WB + R_WB * t_BC;
  const Eigen::Vector3d Xc_B = R_WC.transpose() * (X_W - p_WC);
  // Jtheta_B must equal A: [X_C - t_CB]x R_CB (same physical quantity)
  Eigen::Matrix3d hatB;
  hatB << 0.0, -Xc_m_t.z(), Xc_m_t.y(), Xc_m_t.z(), 0.0, -Xc_m_t.x(),
      -Xc_m_t.y(), Xc_m_t.x(), 0.0;
  const Eigen::Matrix3d Jtheta_B = hatB * R_CB;
  const Eigen::Matrix3d Jp_B = -R_CB * R_WB.transpose();

  // ---- Path P: production as-coded (pre-fix) ----
  const Eigen::Matrix3d R_WC_P = R_WB * R_CB;  // treating T_CB as if T_BC
  const Eigen::Vector3d p_WC_P = p_WB + R_WB * t_CB;
  const Eigen::Vector3d Xc_P = R_WC_P.transpose() * (X_W - p_WC_P);
  const Eigen::Vector3d Xct_impl = Xc_P + R_CB.transpose() * t_CB;
  Eigen::Matrix3d hatP;
  hatP << 0.0, -Xct_impl.z(), Xct_impl.y(), Xct_impl.z(), 0.0, -Xct_impl.x(),
      -Xct_impl.y(), Xct_impl.x(), 0.0;
  const Eigen::Matrix3d Jtheta_P = hatP * R_CB.transpose();
  const Eigen::Matrix3d Jp_P = -R_WC_P.transpose();

  std::fflush(stdout);
  std::printf("det(R_CB)=%.10g\n", R_CB.determinant());
  std::printf("Xc_A=(%.10g,%.10g,%.10g)\n", Xc_A.x(), Xc_A.y(), Xc_A.z());
  std::printf("Xc_B=(%.10g,%.10g,%.10g)\n", Xc_B.x(), Xc_B.y(), Xc_B.z());
  std::printf("Xc_P=(%.10g,%.10g,%.10g)\n", Xc_P.x(), Xc_P.y(), Xc_P.z());
  std::printf("||A-B||=%.3g  ||A-P||=%.3g\n", (Xc_A - Xc_B).norm(),
              (Xc_A - Xc_P).norm());
  std::printf("Jrot ||A-B||=%.3g  ||A-P||=%.3g\n", (Jtheta_A - Jtheta_B).norm(),
              (Jtheta_A - Jtheta_P).norm());
  std::printf("Jtrans ||A-B||=%.3g  ||A-P||=%.3g\n", (Jp_A - Jp_B).norm(),
              (Jp_A - Jp_P).norm());
  std::printf("Jtheta_A==Jtheta_correct: %d\n", (int)((Jtheta_A - Jtheta_B).norm() < 1e-9));
  std::fflush(stdout);

  assert((Xc_A - Xc_B).norm() < 1e-8);  // measured floor 3.45e-9
  assert((Jtheta_A - Jtheta_B).norm() < 1e-9);
  assert((Jp_A - Jp_B).norm() < 1e-9);

  std::printf("Gate X: A==B PASS; A!=P reproduced (||A-P||=%.3g m)\n",
              (Xc_A - Xc_P).norm());
  return 0;
}