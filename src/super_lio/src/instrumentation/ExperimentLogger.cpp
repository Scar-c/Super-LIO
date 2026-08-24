#include "instrumentation/ExperimentLogger.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace LI2Sup {

namespace {

std::string fmt(double v) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(6) << v;
  return oss.str();
}

std::string fmtU(unsigned long long v) { return std::to_string(v); }

}  // namespace

bool ExperimentLogger::open(const std::string& out_dir,
                            const ManifestFields& fields) {
  if (out_dir.empty()) return false;
  std::error_code ec;
  std::filesystem::create_directories(out_dir, ec);
  if (ec) return false;

  if (!timing_csv_.open(out_dir + "/timing.csv",
                        {"timestamp", "imu_propagation_ms", "undistortion_ms",
                         "downsample_ms", "state_update_ms", "map_update_ms",
                         "total_ms"})) {
    return false;
  }
  if (!lio_csv_.open(out_dir + "/lio_stats.csv",
                     {"timestamp", "effective_points", "iteration_count",
                      "residual_count", "residual_mean", "residual_std",
                      "residual_min", "residual_max"})) {
    return false;
  }
  if (!map_csv_.open(out_dir + "/map_stats.csv",
                     {"timestamp", "octvox_voxel_count", "octvox_capacity",
                      "estimated_octvox_bytes", "estimated_total_bytes"})) {
    return false;
  }

  std::ofstream manifest(out_dir + "/run_manifest.yaml");
  if (!manifest.is_open()) return false;
  manifest << RunManifest::buildYaml(fields);
  manifest.close();

  open_ = true;
  return true;
}

void ExperimentLogger::recordEpoch(double timestamp, const EpochTimings& t,
                                   std::size_t effective_points,
                                   std::size_t iterations,
                                   const RunningStats& residuals,
                                   std::size_t voxel_count,
                                   std::size_t capacity) {
  if (!open_) return;
  timing_csv_.writeRow({fmt(timestamp), fmt(t.imu_propagation_ms),
                        fmt(t.undistortion_ms), fmt(t.downsample_ms),
                        fmt(t.state_update_ms), fmt(t.map_update_ms),
                        fmt(t.total_ms)});
  lio_csv_.writeRow({fmt(timestamp), fmtU(effective_points), fmtU(iterations),
                     fmtU(residuals.count), fmt(residuals.mean()),
                     fmt(residuals.stddev()), fmt(residuals.min),
                     fmt(residuals.max)});
  map_csv_.writeRow({fmt(timestamp), fmtU(voxel_count), fmtU(capacity),
                     fmtU(voxel_count * 104ULL), fmtU(voxel_count * 160ULL)});
}

void ExperimentLogger::close() {
  if (!open_) return;
  timing_csv_.close();
  lio_csv_.close();
  map_csv_.close();
  open_ = false;
}

}  // namespace LI2Sup