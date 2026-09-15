#include "dec_lio/FinalCandidateGate.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace DecLIO {

FinalCandidateGate::FinalCandidateGate(FinalCandidateGateKind kind)
    : kind_(kind) {}

void FinalCandidateGate::reset() {
  samples_.clear();
  state_loose_ = false;
}

double FinalCandidateGate::rollingWeakFraction() const {
  if (samples_.empty()) return 0.0;
  std::size_t weak_count = 0;
  for (const Sample& sample : samples_) {
    if (sample.weak) ++weak_count;
  }
  return static_cast<double>(weak_count) /
         static_cast<double>(samples_.size());
}

double FinalCandidateGate::rollingQMedian(std::size_t* valid_count) const {
  std::vector<double> values;
  values.reserve(samples_.size());
  for (const Sample& sample : samples_) {
    if (sample.weak && std::isfinite(sample.q)) values.push_back(sample.q);
  }
  if (valid_count) *valid_count = values.size();
  if (values.empty()) return std::numeric_limits<double>::quiet_NaN();
  std::sort(values.begin(), values.end());
  const std::size_t middle = values.size() / 2;
  if (values.size() % 2 == 0) {
    return 0.5 * (values[middle - 1] + values[middle]);
  }
  return values[middle];
}

FinalCandidateGateDecision FinalCandidateGate::update(
    bool registration_success, bool weak_observed, double weak_multiplier,
    double q_weak) {
  FinalCandidateGateDecision decision;
  decision.state_before_loose = state_loose_;
  decision.registration_observed = registration_success;
  decision.weak_observed = registration_success && weak_observed;
  decision.weak_multiplier = decision.weak_observed
                                 ? weak_multiplier
                                 : std::numeric_limits<double>::quiet_NaN();
  decision.q_weak = decision.weak_observed
                       ? q_weak
                       : std::numeric_limits<double>::quiet_NaN();

  if (registration_success) {
    samples_.push_back(Sample{weak_observed, q_weak});
    if (samples_.size() > kWindow) samples_.pop_front();
  }

  decision.rolling_successful_frames = samples_.size();
  decision.rolling_weak_fraction = rollingWeakFraction();
  decision.rolling_q_median = rollingQMedian(&decision.rolling_q_frames);

  // Fewer than ten valid successful shadow registrations always means the
  // safety state is Native, independently of the selected gate design.
  if (samples_.size() < kMinimumValidFrames) {
    state_loose_ = false;
  } else if (kind_ == FinalCandidateGateKind::kG0) {
    if (!state_loose_ && decision.rolling_weak_fraction >= 0.75) {
      state_loose_ = true;
    } else if (state_loose_ && decision.rolling_weak_fraction <= 0.60) {
      state_loose_ = false;
    }
  } else {
    const bool enter = decision.rolling_weak_fraction >= 0.75 &&
                       std::isfinite(decision.rolling_q_median) &&
                       decision.rolling_q_median >= 0.30;
    const bool exit = decision.rolling_weak_fraction <= 0.60 ||
                      (std::isfinite(decision.rolling_q_median) &&
                       decision.rolling_q_median <= 0.20);
    if (!state_loose_ && enter) {
      state_loose_ = true;
    } else if (state_loose_ && exit) {
      state_loose_ = false;
    }
  }

  decision.state_after_loose = state_loose_;
  decision.state_changed = decision.state_before_loose != state_loose_;
  // A failed registration can never execute the loose update for this frame;
  // it leaves the rolling state untouched but safely selects Native now.
  decision.selected_loose = registration_success && state_loose_;
  return decision;
}

const char* finalCandidateGateName(FinalCandidateGateKind kind) {
  return kind == FinalCandidateGateKind::kG0 ? "g0" : "g1";
}

bool parseFinalCandidateGate(const std::string& value,
                             FinalCandidateGateKind& kind) {
  if (value == "g0") {
    kind = FinalCandidateGateKind::kG0;
    return true;
  }
  if (value == "g1") {
    kind = FinalCandidateGateKind::kG1;
    return true;
  }
  return false;
}

}  // namespace DecLIO
