#ifndef EXPERIMENT_LOGGER_H_
#define EXPERIMENT_LOGGER_H_

#include <cstddef>
#include <string>

#include "instrumentation/CsvWriter.h"
#include "instrumentation/RunManifest.h"
#include "instrumentation/RunStats.h"

namespace LI2Sup {

class ExperimentLogger {
 public:
  ExperimentLogger() = default;
  ~ExperimentLogger() { close(); }

  bool open(const std::string& out_dir, const ManifestFields& fields);
  void recordEpoch(double timestamp, const EpochTimings& timings,
                   std::size_t effective_points, std::size_t iterations,
                   const RunningStats& residuals, std::size_t voxel_count,
                   std::size_t capacity);
  void close();
  bool isOpen() const { return open_; }

 private:
  bool open_ = false;
  CsvWriter timing_csv_;
  CsvWriter lio_csv_;
  CsvWriter map_csv_;
};

}  // namespace LI2Sup

#endif  // EXPERIMENT_LOGGER_H_