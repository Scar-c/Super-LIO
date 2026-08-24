#ifndef SCHEME_B_SHADOW_H_
#define SCHEME_B_SHADOW_H_

#include <array>
#include <cmath>
#include <cstdint>
#include <map>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace LI2Sup {

// Scheme-B (G-1V shadow) helpers.
// Patch is NOT the centroid: a patch keeps an immutable full-3D local offset
// d0 = P0 - mu_ref relative to the supporting parent surfel; the surfel
// provides geometry support via shortest-arc normal transport.

// Canonical normal sign: deterministic (largest-abs component positive).
inline Eigen::Vector3d canonicalNormal(Eigen::Vector3d n) {
  n.normalize();
  int i = 0;
  if (std::abs(n.y()) > std::abs(n(i))) i = 1;
  if (std::abs(n.z()) > std::abs(n(i))) i = 2;
  if (n(i) < 0.0) n = -n;
  return n;
}

// Shortest-arc rotation Q mapping n_ref -> n_k (with sign continuity).
// Near-parallel identity; near-antiparallel deterministic fallback (rotate
// around the axis of largest-abs component of n_ref by 180deg).
inline Eigen::Matrix3d shortestArcRotation(const Eigen::Vector3d& n_ref_in,
                                           const Eigen::Vector3d& n_k_in) {
  const Eigen::Vector3d a = canonicalNormal(n_ref_in);
  Eigen::Vector3d b = canonicalNormal(n_k_in);
  // sign continuity (v1 spec G-1V 18): flip before shortest-arc
  if (a.dot(b) < 0.0) b = -b;
  const double d = a.dot(b);
  if (d > 1.0 - 1e-12) return Eigen::Matrix3d::Identity();
  if (d < -1.0 + 1e-9) {
    // antiparallel: deterministic 180deg about a unit axis perpendicular to a
    // (any axis parallel to a would keep a fixed -> wrong mapping)
    int min_idx = 0;
    if (std::abs(a.y()) < std::abs(a(min_idx))) min_idx = 1;
    if (std::abs(a.z()) < std::abs(a(min_idx))) min_idx = 2;
    Eigen::Vector3d axis = Eigen::Vector3d::Unit(min_idx).cross(a);
    axis.normalize();
    return Eigen::AngleAxisd(M_PI, axis).toRotationMatrix();
  }
  const Eigen::Vector3d v = a.cross(b);
  const double s = v.norm();
  const double c = d;
  Eigen::Matrix3d V;
  V << 0.0, -v.z(), v.y(), v.z(), 0.0, -v.x(), -v.y(), v.x(), 0.0;
  return Eigen::Matrix3d::Identity() + V +
         V * V * ((1.0 - c) / (s * s));
}

// Scheme-B reconstructed anchor at update k:
//   P_B(k) = mu_k + Q * d0
inline Eigen::Vector3d reconstructAnchor(const Eigen::Vector3d& mu_k,
                                         const Eigen::Matrix3d& Q,
                                         const Eigen::Vector3d& d0) {
  return mu_k + Q * d0;
}

// Patch support plane: n_k^T (X - P_B(k)) = 0 (anchor, not centroid).

// Camera ray-plane intersection for warp prediction:
//   ray from camera center through pixel (u,v); intersect plane (anchor, n).
struct Pinhole {
  double fx = 0.0, fy = 0.0, cx = 0.0, cy = 0.0;
};

inline bool rayPlaneIntersect(const Pinhole& cam, double u, double v,
                              const Eigen::Vector3d& cam_center,
                              const Eigen::Vector3d& anchor,
                              const Eigen::Vector3d& n, Eigen::Vector3d& X,
                              double& depth) {
  // pixel -> camera-frame ray (z=1 plane)
  const Eigen::Vector3d dir_cam((u - cam.cx) / cam.fx, (v - cam.cy) / cam.fy, 1.0);
  const double denom = n.dot(dir_cam);
  if (std::abs(denom) < 1e-9) return false;
  const double s = n.dot(anchor - cam_center) / denom;
  if (s <= 1e-4) return false;
  X = cam_center + s * dir_cam;
  depth = s;
  return true;
}

// Sample 8x8 patch intensities from a grayscale image with bilinear sampling.
// Returns false if any sample falls outside the image (with a small margin).
inline bool samplePatch(const std::vector<uint8_t>& img, int w, int h,
                        double uc, double vc, int half, int size,
                        std::vector<float>& out) {
  out.resize(size * size);
  const int margin = half;
  if (uc - margin < 0.0 || uc + margin >= w || vc - margin < 0.0 ||
      vc + margin >= h) {
    return false;
  }
  for (int j = 0; j < size; ++j) {
    for (int i = 0; i < size; ++i) {
      const double u = uc + (i - half);
      const double v = vc + (j - half);
      const int u0 = static_cast<int>(std::floor(u));
      const int v0 = static_cast<int>(std::floor(v));
      const double fu = u - u0;
      const double fv = v - v0;
      const double v00 = img[static_cast<size_t>(v0) * w + u0];
      const double v10 = img[static_cast<size_t>(v0) * w + std::min(w - 1, u0 + 1)];
      const double v01 = img[static_cast<size_t>(std::min(h - 1, v0 + 1)) * w + u0];
      const double v11 = img[static_cast<size_t>(std::min(h - 1, v0 + 1)) * w +
                         std::min(w - 1, u0 + 1)];
      out[static_cast<size_t>(j) * size + i] =
          static_cast<float>((1.0 - fv) * ((1.0 - fu) * v00 + fu * v10) +
                             fv * ((1.0 - fu) * v01 + fu * v11));
    }
  }
  return true;
}

inline double patchSSE(const std::vector<float>& a, const std::vector<float>& b) {
  double s = 0.0;
  for (size_t i = 0; i < a.size(); ++i) {
    const double d = a[i] - b[i];
    s += d * d;
  }
  return s;
}

}  // namespace LI2Sup

#endif  // SCHEME_B_SHADOW_H_