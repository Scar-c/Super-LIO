#ifndef DEC_LIO_COUNTERFACTUAL_REPLAY_H_
#define DEC_LIO_COUNTERFACTUAL_REPLAY_H_

#include <cstdint>
#include <fstream>
#include <string>

#include "dec_lio/LidarOnlyShadow.h"

namespace DecLIO {

// Prompt15 is deliberately a one-event, mean-only sensitivity probe. It does
// not own or modify estimator covariance, map, or sensor history.
class CounterfactualReplayAnalyzer {
 public:
  CounterfactualReplayAnalyzer(const std::string& csv_path,
                               std::uint64_t target_frame,
                               double condition_threshold);
  ~CounterfactualReplayAnalyzer();

  CounterfactualReplayAnalyzer(const CounterfactualReplayAnalyzer&) = delete;
  CounterfactualReplayAnalyzer& operator=(const CounterfactualReplayAnalyzer&) = delete;

  // Returns true only when the target frame is eligible for intervention.
  bool prepare(std::uint64_t frame, double timestamp,
               const LidarOnlyShadowResult& result);
  bool has_pending() const { return pending_; }
  const BASIC::SE3& pending_pose() const { return pending_pose_; }

  void commit(const BASIC::SE3& native_pose,
              const BASIC::SE3& intervention_pose);
  void cancel(const std::string& reason);
  void finalize();

 private:
  struct Mode {
    std::string type = "NONE";
    int index = -1;
    double rho = 0.0;
    double condition = 0.0;
    double c_l = 0.0;
    double c_tight = 0.0;
    double suppression_ratio = 0.0;
    bool valid = false;
  };

  static void makeParent(const std::string& path);
  static bool finitePose(const BASIC::SE3& pose);
  Mode selectMode(const LidarOnlyShadowResult& result) const;
  bool eligible(const LidarOnlyShadowResult& result, std::string& reason,
                Mode& mode) const;
  void writeHeader();
  void writePose(const BASIC::SE3& pose);
  void writeRecord(bool applied, const std::string& reason,
                   const BASIC::SE3& native_pose,
                   const BASIC::SE3& intervention_pose);

  std::ofstream csv_;
  std::uint64_t target_frame_ = 0;
  double condition_threshold_ = 10.0;
  bool target_seen_ = false;
  bool pending_ = false;
  bool written_ = false;
  std::string cancellation_reason_;
  std::uint64_t frame_ = 0;
  double timestamp_ = 0.0;
  std::string selection_reason_ = "TARGET_NOT_REACHED";
  Mode mode_;
  LidarOnlyShadowResult result_;
  BASIC::SE3 pending_pose_;
};

}  // namespace DecLIO

#endif  // DEC_LIO_COUNTERFACTUAL_REPLAY_H_
