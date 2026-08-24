#ifndef VISUAL_SUPPORT_STATS_H_
#define VISUAL_SUPPORT_STATS_H_

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Eigenvalues>

#include "camera/CameraCalibration.h"
#include "geometry/MicroSurfelStats.h"

namespace LI2Sup {

// G-1 plane validity + visual support aggregation (causal, pre-map-update).
// 16 candidate gates evaluated online in a single bag run; per-frame row is
// emitted for offline analysis (no per-threshold re-run).

struct PlaneGateParams {
  double q_flat;   // lambda0 / trace
  double q_line;   // lambda1 / lambda2
};

// Returns true if subvoxel is a valid plane under the gate.
inline bool evaluatePlaneGate(const SubvoxelStats& st, const PlaneGateParams& g,
                              double* q_flat_out = nullptr,
                              double* q_line_out = nullptr) {
  if (!st.active || st.n < 5) return false;
  const Eigen::Matrix3d S = GeometryStatsSidecar::unpackS(st.s);
  const double n = static_cast<double>(st.n);
  if (S.trace() <= 1e-12 || !S.allFinite()) return false;
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(S / n);
  const Eigen::Vector3d ev = es.eigenvalues();  // ascending
  if (!ev.allFinite()) return false;
  const double qf = ev(0) / (ev(0) + ev(1) + ev(2));
  const double ql = ev(2) > 1e-12 ? ev(1) / ev(2) : 0.0;
  if (q_flat_out) *q_flat_out = qf;
  if (q_line_out) *q_line_out = ql;
  if (qf > g.q_flat) return false;
  if (ql < g.q_line) return false;
  return true;
}

struct VisualSupportRow {
  double timestamp = 0.0;
  int n_fov = 0;                       // G_FOV size
  std::array<int, 21> n_hist_fov{};    // N histogram over FOV points
  std::array<int, 5> rN_point{};       // R3/R5/R8/R10/R20 point (FOV denominator)
  std::array<int, 16> plane_valid_point{};  // per gate (16 combos)
  std::array<int, 16> plane_valid_voxel{};
  std::array<int, 4> grid_cells{};     // cells with >=1 FOV / N5 / plane-valid / any
  int occupied_cols = 0;
  int occupied_rows = 0;
  std::array<int, 4> quadrant_hits{};
  double dt_cam = 0.0;                 // t_cam_effective - t_L
  int n_processed = 0;                 // effective candidates examined
};

// The 16 gate combinations (q_flat x q_line), fixed order.
inline const std::vector<PlaneGateParams>& gateSweep() {
  static const std::vector<double> qfs = {0.01, 0.02, 0.03, 0.05};
  static const std::vector<double> qls = {0.05, 0.10, 0.20, 0.30};
  static std::vector<PlaneGateParams> sweep = [] {
    std::vector<PlaneGateParams> v;
    for (double qf : qfs)
      for (double ql : qls) v.push_back({qf, ql});
    return v;
  }();
  return sweep;
}

// Aggregates one epoch; caller supplies per-point results.
class VisualSupportAggregator {
 public:
  VisualSupportRow& row() { return row_; }
  void beginEpoch(double t_lidar) {
    row_ = VisualSupportRow{};
    row_.timestamp = t_lidar;
  }
  void reset() { rows_.clear(); }
  const std::vector<VisualSupportRow>& rows() const { return rows_; }
  void commitEpoch() { rows_.push_back(row_); }

 private:
  VisualSupportRow row_;
  std::vector<VisualSupportRow> rows_;
};

}  // namespace LI2Sup

#endif  // VISUAL_SUPPORT_STATS_H_