#ifndef RUN_STATS_H_
#define RUN_STATS_H_

#include <chrono>
#include <cmath>
#include <cstddef>

namespace LI2Sup {

inline double NowMs() {
  return std::chrono::duration<double, std::milli>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

struct RunningStats {
  std::size_t count = 0;
  double sum = 0.0;
  double sum_sq = 0.0;
  double min = 0.0;
  double max = 0.0;
  bool initialized = false;

  void add(double v) {
    if (!initialized) {
      min = v;
      max = v;
      initialized = true;
    } else {
      if (v < min) min = v;
      if (v > max) max = v;
    }
    ++count;
    sum += v;
    sum_sq += v * v;
  }

  double mean() const { return count ? sum / static_cast<double>(count) : 0.0; }

  double variance() const {
    if (count < 2) return 0.0;
    double m = mean();
    double c = static_cast<double>(count);
    double v = (sum_sq - 2.0 * m * sum + c * m * m) / (c - 1.0);
    return v > 0.0 ? v : 0.0;
  }

  double stddev() const { return std::sqrt(variance()); }

  void merge(const RunningStats& other) {
    if (other.count == 0) return;
    if (!initialized) {
      *this = other;
      return;
    }
    count += other.count;
    sum += other.sum;
    sum_sq += other.sum_sq;
    if (other.min < min) min = other.min;
    if (other.max > max) max = other.max;
  }

  void reset() {
    count = 0;
    sum = 0.0;
    sum_sq = 0.0;
    min = 0.0;
    max = 0.0;
    initialized = false;
  }
};

struct EpochTimings {
  double imu_propagation_ms = 0.0;
  double undistortion_ms = 0.0;
  double downsample_ms = 0.0;
  double state_update_ms = 0.0;
  double map_update_ms = 0.0;
  double total_ms = 0.0;
};

}  // namespace LI2Sup

#endif  // RUN_STATS_H_