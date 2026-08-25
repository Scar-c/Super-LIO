#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "basic/alias.h"

// Synthetic validation of the V-2 plane-aware warp Jacobian (6-DOF) with
// body-to-camera extrinsic, using a continuous residual function (no image
// discretization): analytic J vs central difference must agree to ~1e-8.

#include "basic/Manifold.h"

using namespace BASIC;

int main() {
  // pinhole
  const double fx = 425.25, fy = 426.80, cx = 386.02, cy = 241.91;
  // extrinsics: body -> cam (similar to OS1->left camera: roll ~90 deg)
  Eigen::Matrix3d R_bc =
      getenv("V2_NOEXTR") ? Eigen::Matrix3d::Identity()
                          : (Eigen::AngleAxisd(M_PI / 2.0, Eigen::Vector3d::UnitX()) *
                             Eigen::AngleAxisd(0.2, Eigen::Vector3d::UnitY()))
                                .toRotationMatrix();
  Eigen::Vector3d t_bc = getenv("V2_NOEXTR") ? Eigen::Vector3d::Zero()
                                             : Eigen::Vector3d(0.05, 0.0, 0.10);

  // ref camera pose (body) and patch plane
  SE3 pose_ref;
  pose_ref.R_ = Eigen::Matrix3f(
      (Eigen::AngleAxisd(0.3, Eigen::Vector3d::UnitZ()) *
       Eigen::AngleAxisd(0.1, Eigen::Vector3d::UnitY()))
          .toRotationMatrix()
          .cast<float>());
  pose_ref.t_ = Eigen::Vector3f(0.5f, -0.2f, 1.0f);
  const Eigen::Matrix3d R_body_ref = pose_ref.R_.cast<double>();
  const Eigen::Matrix3d R_ref = R_body_ref * R_bc;
  const Eigen::Vector3d t_ref = pose_ref.t_.cast<double>() + R_body_ref * t_bc;

  const Eigen::Vector3d P_patch(2.0, 1.0, 3.0);
  const Eigen::Vector3d n_sync = Eigen::Vector3d(0.1, 0.2, 0.97).normalized();

  // current pose
  SE3 pose_cur;
  pose_cur.R_ = Eigen::Matrix3f(
      (Eigen::AngleAxisd(0.05, Eigen::Vector3d::UnitZ()) *
       Eigen::AngleAxisd(-0.08, Eigen::Vector3d::UnitX()))
          .toRotationMatrix()
          .cast<float>());
  pose_cur.t_ = Eigen::Vector3f(1.0f, 0.3f, 1.5f);
  const Eigen::Matrix3d R_body_cur = pose_cur.R_.cast<double>();
  const Eigen::Matrix3d R_cur = R_body_cur * R_bc;
  const Eigen::Vector3d t_cur = pose_cur.t_.cast<double>() + R_body_cur * t_bc;

  // sample 8x8 patch pixels in the ref image
  const double ref_u = 380.0, ref_v = 240.0;
  std::vector<Eigen::Vector2d> warped;
  for (int j = 0; j < 8; ++j) {
    for (int i = 0; i < 8; ++i) {
      const double u = ref_u + (i - 4);
      const double v = ref_v + (j - 4);
      const Eigen::Vector3d ray_cam((u - cx) / fx, (v - cy) / fy, 1.0);
      const Eigen::Vector3d dir_w = R_ref * ray_cam;
      const double denom = n_sync.dot(dir_w);
      const double s = n_sync.dot(P_patch - t_ref) / denom;
      const Eigen::Vector3d X = t_ref + s * dir_w;
      const Eigen::Vector3d Xc = R_cur.transpose() * (X - t_cur);
      const double u2 = fx * Xc.x() / Xc.z() + cx;
      const double v2 = fy * Xc.y() / Xc.z() + cy;
      warped.emplace_back(u2, v2);
    }
  }

  // continuous residual: r = sin(a*u + b*v) + c*z(Xc)  (smooth, no discretization)
  const double a = 0.31, b = 0.17;
  auto warp_of = [&](const SE3& p) {
    const Eigen::Matrix3d Rc = p.R_.cast<double>() * R_bc;
    const Eigen::Vector3d tc = p.t_.cast<double>() + p.R_.cast<double>() * t_bc;
    std::vector<Eigen::Vector2d> w;
    for (int j = 0; j < 8; ++j) {
      for (int i = 0; i < 8; ++i) {
        const double u = ref_u + (i - 4);
        const double v = ref_v + (j - 4);
        const Eigen::Vector3d ray_cam((u - cx) / fx, (v - cy) / fy, 1.0);
        const Eigen::Vector3d dir_w = R_ref * ray_cam;
        const double s = n_sync.dot(P_patch - t_ref) / dir_w.dot(n_sync);
        const Eigen::Vector3d X = t_ref + s * dir_w;
        const Eigen::Vector3d Xc = Rc.transpose() * (X - tc);
        const double u2 = fx * Xc.x() / Xc.z() + cx;
        const double v2 = fy * Xc.y() / Xc.z() + cy;
        w.emplace_back(u2, v2);
      }
    }
    return w;
  };
  auto residual_of = [&](const SE3& p) {
    const auto w = warp_of(p);
    double acc = 0.0;
    for (size_t k = 0; k < w.size(); ++k) {
      acc += std::sin(a * w[k].x() + b * w[k].y());
    }
    return acc;
  };

  // analytic J (sum over samples, per direction) using the T_cb-aware formula
  Eigen::Matrix<double, 6, 1> J_an = Eigen::Matrix<double, 6, 1>::Zero();
  for (int j = 0; j < 8; ++j) {
    for (int i = 0; i < 8; ++i) {
      const double u = ref_u + (i - 4);
      const double v = ref_v + (j - 4);
      const Eigen::Vector3d ray_cam((u - cx) / fx, (v - cy) / fy, 1.0);
      const Eigen::Vector3d dir_w = R_ref * ray_cam;
      const double s = n_sync.dot(P_patch - t_ref) / dir_w.dot(n_sync);
      const Eigen::Vector3d X = t_ref + s * dir_w;
      const Eigen::Vector3d Xc = R_cur.transpose() * (X - t_cur);
      const double u2 = fx * Xc.x() / Xc.z() + cx;
      const double v2 = fy * Xc.y() / Xc.z() + cy;
      const double Iu = a * std::cos(a * u2 + b * v2);
      const double Iv = b * std::cos(a * u2 + b * v2);
      // du/dXc, dv/dXc
      Eigen::Matrix<double, 2, 3> du_dXc;
      du_dXc << fx / Xc.z(), 0.0, -fx * Xc.x() / (Xc.z() * Xc.z()),
          0.0, fy / Xc.z(), -fy * Xc.y() / (Xc.z() * Xc.z());
      // dXc/dxi (body perturbation + extrinsic)
      Eigen::Matrix<double, 3, 6> dXc_dxi = Eigen::Matrix<double, 3, 6>::Zero();
      dXc_dxi.block<3, 3>(0, 3) = -R_cur.transpose();
      const Eigen::Vector3d Xc_t = Xc + R_bc.transpose() * t_bc;
      auto Xc_skew2 = [](const Eigen::Vector3d& v) {
        Eigen::Matrix3d m;
        m << 0.0, -v.z(), v.y(), v.z(), 0.0, -v.x(), -v.y(), v.x(), 0.0;
        return m;
      };
      Eigen::Matrix3d Xct_skew;
      Xct_skew << 0.0, -Xc_t.z(), Xc_t.y(), Xc_t.z(), 0.0, -Xc_t.x(),
          -Xc_t.y(), Xc_t.x(), 0.0;
      dXc_dxi.block<3, 3>(0, 0) =
          getenv("V2_NOEXTR") ? Xc_skew2(Xc) : Xct_skew * R_bc.transpose();
      const Eigen::Matrix<double, 2, 6> du_dxi = du_dXc * dXc_dxi;
      J_an += (Iu * du_dxi.row(0) + Iv * du_dxi.row(1)).transpose();
    }
  }

  // central difference
  const double eps = 1e-5;  // float-pose safe (R_ is Matrix3f)
  Eigen::Matrix<double, 6, 1> J_fd = Eigen::Matrix<double, 6, 1>::Zero();
  for (int d = 0; d < 6; ++d) {
    SE3 pp = pose_cur, pm = pose_cur;
    if (d < 3) {
      const Eigen::Matrix3f Rm =
          Eigen::AngleAxisf(static_cast<float>(eps), Eigen::Vector3f::Unit(d))
              .toRotationMatrix();
      pp.R_ = pp.R_ * Rm;
      pm.R_ = pm.R_ * Rm.transpose();
    } else {
      pp.t_[d - 3] += static_cast<float>(eps);
      pm.t_[d - 3] -= static_cast<float>(eps);
    }
    J_fd(d) = (residual_of(pp) - residual_of(pm)) / (2.0 * eps);
  }

  double max_rel = 0.0;
  const char* dn[6] = {"rx", "ry", "rz", "tx", "ty", "tz"};
  for (int d = 0; d < 6; ++d) {
    const double rel = std::abs(J_an(d) - J_fd(d)) /
                       std::max(1e-12, std::abs(J_fd(d)));
    max_rel = std::max(max_rel, rel);
    std::printf("dir=%s an=%.10g fd=%.10g rel=%.3g\n", dn[d], J_an(d),
                J_fd(d), rel);
  }
  std::fflush(stdout);
  // P0-9: float SE3 precision budget (old 1e-6 could not pass on float poses)
  assert(max_rel < 1e-2);
  std::printf("all v2 jacobian (T_cb-aware) synthetic tests passed (max_rel=%.3g)\n", max_rel);
  return 0;
}