#ifndef DEC_LIO_FINAL_CANDIDATE_GATE_H_
#define DEC_LIO_FINAL_CANDIDATE_GATE_H_

#include <cstddef>
#include <deque>
#include <limits>
#include <string>

namespace DecLIO {

enum class FinalCandidateGateKind { kG0, kG1 };

struct FinalCandidateGateDecision {
  bool state_before_loose = false;
  bool state_after_loose = false;
  bool state_changed = false;
  bool selected_loose = false;
  bool registration_observed = false;
  bool weak_observed = false;
  double weak_multiplier = std::numeric_limits<double>::quiet_NaN();
  double q_weak = std::numeric_limits<double>::quiet_NaN();
  double rolling_weak_fraction = 0.0;
  double rolling_q_median = std::numeric_limits<double>::quiet_NaN();
  std::size_t rolling_successful_frames = 0;
  std::size_t rolling_q_frames = 0;
};

// Prompt23's only production gate. The window contains successful shadow
// registrations only; a failed registration is deliberately not fabricated
// into a healthy or weak observation.
class FinalCandidateGate {
 public:
  static constexpr std::size_t kWindow = 20;
  static constexpr std::size_t kMinimumValidFrames = 10;

  explicit FinalCandidateGate(FinalCandidateGateKind kind);

  FinalCandidateGateDecision update(bool registration_success,
                                    bool weak_observed,
                                    double weak_multiplier,
                                    double q_weak);
  void reset();
  FinalCandidateGateKind kind() const { return kind_; }
  bool state_loose() const { return state_loose_; }

 private:
  struct Sample {
    bool weak = false;
    double q = std::numeric_limits<double>::quiet_NaN();
  };

  double rollingWeakFraction() const;
  double rollingQMedian(std::size_t* valid_count) const;

  FinalCandidateGateKind kind_;
  bool state_loose_ = false;
  std::deque<Sample> samples_;
};

const char* finalCandidateGateName(FinalCandidateGateKind kind);
bool parseFinalCandidateGate(const std::string& value,
                             FinalCandidateGateKind& kind);

}  // namespace DecLIO

#endif  // DEC_LIO_FINAL_CANDIDATE_GATE_H_
