#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "geometry/SchemeBShadow.h"

using namespace LI2Sup;

int main() {
  // 1) canonical sign: largest-abs component positive
  {
    Eigen::Vector3d n(-0.7, 0.2, -0.1);
    n.normalize();
    Eigen::Vector3d c = canonicalNormal(n);
    assert(c(0) > 0.0);
    assert(std::abs(c.norm() - 1.0) < 1e-12);
    assert(c.cross(n.normalized()).norm() < 1e-9);
    // flip
    Eigen::Vector3d c2 = canonicalNormal(-n);
    assert((c - c2).norm() < 1e-12);
  }

  // 2) shortest-arc: identity for same normal
  {
    Eigen::Vector3d n = Eigen::Vector3d(0.3, 0.4, 0.8).normalized();
    Eigen::Matrix3d Q = shortestArcRotation(n, n);
    assert((Q - Eigen::Matrix3d::Identity()).norm() < 1e-9);
  }

  // 3) shortest-arc: Q * n_ref == n_k for a 30-degree rotation
  {
    Eigen::Vector3d n_ref(0, 0, 1);
    Eigen::Vector3d n_k = Eigen::AngleAxisd(30.0 * M_PI / 180.0, Eigen::Vector3d::UnitY()) * n_ref;
    Eigen::Matrix3d Q = shortestArcRotation(n_ref, n_k);
    assert((Q * n_ref - n_k).norm() < 1e-9);
    // rotation is a rotation matrix
    assert(std::abs(Q.determinant() - 1.0) < 1e-9);
  }

  // 4) anti-parallel input: sign-continuity flips first (v1 18); fallback is
  //    a deterministic 180-deg about a perpendicular axis (covers the
  //    un-flipped numeric singularity case); result is direction-consistent
  {
    Eigen::Vector3d n_ref(0, 0, 1);
    Eigen::Vector3d n_k(0, 0, -1);
    Eigen::Matrix3d Q = shortestArcRotation(n_ref, n_k);
    // direction consistency: |Q*n_ref . n_k| ~ 1 (same line, sign absorbed)
    assert(std::abs(Q.col(2).dot(n_k)) > 1.0 - 1e-9);
    assert(std::abs(Q.determinant() - 1.0) < 1e-9);
    // deterministic (same result twice)
    Eigen::Matrix3d Q2 = shortestArcRotation(n_ref, n_k);
    assert((Q - Q2).norm() < 1e-12);
  }

  // 4b) hard antiparallel (bypassing sign flip): must still map to the same line
  {
    Eigen::Vector3d n_ref(0, 1, 0);
    Eigen::Vector3d n_k(0, -1, 0);
    Eigen::Matrix3d Q = shortestArcRotation(n_ref, n_k);
    assert(std::abs(Q.col(1).dot(n_k)) > 1.0 - 1e-9);
    assert(std::abs(Q.determinant() - 1.0) < 1e-9);
  }

  // 5) near-parallel
  {
    Eigen::Vector3d n_ref(0, 0, 1);
    Eigen::Vector3d n_k = Eigen::Vector3d(1e-8, 0, 1).normalized();
    Eigen::Matrix3d Q = shortestArcRotation(n_ref, n_k);
    assert((Q * n_ref - n_k).norm() < 1e-6);
  }

  // 6) offset exact reconstruction: P_B(t0) == P0
  {
    Eigen::Vector3d mu(1.0, 2.0, 3.0);
    Eigen::Vector3d P0(1.2, 2.1, 3.05);
    Eigen::Vector3d n_ref(0, 0, 1);
    Eigen::Vector3d d0 = P0 - mu;
    Eigen::Matrix3d Q = shortestArcRotation(n_ref, n_ref);
    Eigen::Vector3d P_B = reconstructAnchor(mu, Q, d0);
    assert((P_B - P0).norm() < 1e-12);
  }

  // 7) reconstruction after normal update: plane-through-anchor invariant
  {
    Eigen::Vector3d mu(1.0, 2.0, 3.0);
    Eigen::Vector3d P0(1.2, 2.1, 3.05);
    Eigen::Vector3d n_ref(0, 0, 1);
    Eigen::Vector3d n_k = Eigen::AngleAxisd(45.0 * M_PI / 180.0, Eigen::Vector3d::UnitY()) * n_ref;
    Eigen::Vector3d d0 = P0 - mu;
    Eigen::Matrix3d Q = shortestArcRotation(n_ref, n_k);
    Eigen::Vector3d P_B = reconstructAnchor(mu, Q, d0);
    // anchor lies on updated plane: n_k . (P_B - mu_k) == n_k . (Q d0) == n_ref . d0
    const double lhs = n_k.dot(P_B - mu);
    const double rhs = n_ref.dot(d0);
    assert(std::abs(lhs - rhs) < 1e-9);
    // no chained: direct reconstruction == (apply Q to immutable d0)
    assert((P_B - (mu + Q * d0)).norm() < 1e-12);
  }

  // 8) ray-plane intersection sanity (fronto-parallel plane at depth 2)
  {
    Pinhole cam{500.0, 500.0, 320.0, 240.0};
    Eigen::Vector3d center(0, 0, 0);
    Eigen::Vector3d anchor(0, 0, 2);
    Eigen::Vector3d n(0, 0, 1);
    Eigen::Vector3d X;
    double depth = 0;
    bool ok = rayPlaneIntersect(cam, 320.0, 240.0, center, anchor, n, X, depth);
    assert(ok);
    assert(std::abs(X.z() - 2.0) < 1e-9);
    assert(std::abs(depth - 2.0) < 1e-9);
  }

  // 9) tilted plane: intersection projects back to same pixel
  {
    Pinhole cam{500.0, 500.0, 320.0, 240.0};
    Eigen::Vector3d center(0, 0, 0);
    Eigen::Vector3d anchor(0.5, 0.0, 3.0);
    Eigen::Vector3d n = Eigen::AngleAxisd(-0.3, Eigen::Vector3d::UnitY()) * Eigen::Vector3d::UnitZ();
    Eigen::Vector3d X;
    double depth = 0;
    const double u = 420.0, v = 300.0;
    bool ok = rayPlaneIntersect(cam, u, v, center, anchor, n, X, depth);
    assert(ok && depth > 0);
    // project X back
    const double u2 = cam.fx * X.x() / X.z() + cam.cx;
    const double v2 = cam.fy * X.y() / X.z() + cam.cy;
    assert(std::abs(u2 - u) < 1e-6);
    assert(std::abs(v2 - v) < 1e-6);
  }

  // 10) no tangent-basis dependence: offset reconstruction does not use any
  //     tangent frame (full 3D), so no basis jump can occur across normal updates
  {
    Eigen::Vector3d mu(0, 0, 0);
    Eigen::Vector3d P0(0.1, 0.2, 0.3);
    Eigen::Vector3d n_ref = Eigen::Vector3d(1, 0, 0).normalized();
    Eigen::Vector3d d0 = P0 - mu;
    // two different updates (both rotate normal) must give consistent anchors
    Eigen::Vector3d n_k1 = Eigen::AngleAxisd(0.2, Eigen::Vector3d::UnitZ()) * n_ref;
    Eigen::Vector3d n_k2 = Eigen::AngleAxisd(-0.15, Eigen::Vector3d::UnitY()) * n_k1;
    Eigen::Vector3d P1 = reconstructAnchor(mu, shortestArcRotation(n_ref, n_k1), d0);
    Eigen::Vector3d P2 = reconstructAnchor(mu, shortestArcRotation(n_ref, n_k2), d0);
    // no jump: consecutive anchors move smoothly (norm difference bounded by rotation)
    assert((P2 - P1).norm() < 0.5);
  }

  std::printf("all g1v scheme-b tests passed\n");
  return 0;
}