#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace LI2Sup {

struct BoundedNormSummary {
  static constexpr std::size_t kMaxSamples = 8192;
  std::uint64_t count = 0;
  double max = 0.0;
  std::vector<double> samples;

  void add(double value) {
    ++count;
    if (value > max) max = value;
    if (samples.size() < kMaxSamples) samples.push_back(value);
  }
  double percentile(double p) const {
    if (samples.empty()) return 0.0;
    std::vector<double> sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    const std::size_t index = std::min(
        sorted.size() - 1, static_cast<std::size_t>(p * sorted.size()));
    return sorted[index];
  }
};

class VisualMeasurementEvidence {
 public:
  // Prompt75 F3: measurement-context separation. The global counters remain
  // cumulative (existing consumers unchanged); per-context sub-counters let
  // initial linearization and iterative solver callbacks be accounted
  // separately. Pure instrumentation: no algorithm behavior depends on it.
  enum class Context { INITIAL, SOLVER };

  explicit VisualMeasurementEvidence(bool enabled = false) : enabled_(enabled) {}
  void setEnabled(bool enabled) { enabled_ = enabled; }
  bool enabled() const { return enabled_; }
  void setContext(Context ctx) { context_ = ctx; }
  Context context() const { return context_; }

  void recordQueryHit() { recordQuery(query_hits_, solver_query_hits_); }
  void recordQueryMiss() { recordQuery(query_misses_, solver_query_misses_); }
  void recordQueryRejectedExplicit() {
    recordQuery(query_rejected_explicit_, solver_query_rejected_explicit_);
  }
  void recordObservation(bool valid) {
    if (!enabled_) return;
    ++candidate_observations_;
    ++(valid ? valid_observations_ : rejected_observations_);
    ++(context_ == Context::INITIAL ? initial_candidate_observations_
                                    : solver_candidate_observations_);
    ++(valid ? (context_ == Context::INITIAL ? initial_valid_observations_
                                             : solver_valid_observations_)
             : (context_ == Context::INITIAL ? initial_rejected_observations_
                                             : solver_rejected_observations_));
  }
  void recordMeasurementFrame() {
    if (enabled_) {
      ++measurement_frames_;
      ++(context_ == Context::INITIAL ? initial_measured_frames_
                                      : solver_measured_frames_);
    }
  }
  void recordResidualSamples(std::uint64_t count) {
    if (!enabled_) return;
    residual_samples_ += count;
    if (context_ == Context::INITIAL) initial_residual_samples_ += count;
    else solver_residual_samples_ += count;
  }
  void recordNormalEquations(double h_norm, bool h_finite,
                             double b_norm, bool b_finite) {
    if (!enabled_) return;
    classify(h_norm, h_finite, h_accumulations_, h_nonzero_, h_zero_,
             h_nonfinite_, h_norms_);
    classify(b_norm, b_finite, b_accumulations_, b_nonzero_, b_zero_,
             b_nonfinite_, b_norms_);
  }

  std::uint64_t queryAttempts() const { return query_attempts_; }
  std::uint64_t queryHits() const { return query_hits_; }
  std::uint64_t queryMisses() const { return query_misses_; }
  std::uint64_t queryRejectedExplicit() const { return query_rejected_explicit_; }
  std::uint64_t candidateObservations() const { return candidate_observations_; }
  std::uint64_t validObservations() const { return valid_observations_; }
  std::uint64_t rejectedObservations() const { return rejected_observations_; }
  std::uint64_t measurementFrames() const { return measurement_frames_; }
  std::uint64_t residualSamples() const { return residual_samples_; }
  std::uint64_t initialQueryAttempts() const { return initial_query_attempts_; }
  std::uint64_t initialQueryHits() const { return query_hits_; }
  std::uint64_t initialCandidateObservations() const {
    return initial_candidate_observations_;
  }
  std::uint64_t initialValidObservations() const { return initial_valid_observations_; }
  std::uint64_t initialRejectedObservations() const {
    return initial_rejected_observations_;
  }
  std::uint64_t initialMeasuredFrames() const { return initial_measured_frames_; }
  std::uint64_t initialResidualSamples() const { return initial_residual_samples_; }
  std::uint64_t solverQueryAttempts() const { return solver_query_attempts_; }
  std::uint64_t solverQueryHits() const { return solver_query_hits_; }
  std::uint64_t solverCandidateObservations() const {
    return solver_candidate_observations_;
  }
  std::uint64_t solverValidObservations() const { return solver_valid_observations_; }
  std::uint64_t solverRejectedObservations() const {
    return solver_rejected_observations_;
  }
  std::uint64_t solverMeasuredFrames() const { return solver_measured_frames_; }
  std::uint64_t solverResidualSamples() const { return solver_residual_samples_; }
  std::uint64_t hAccumulations() const { return h_accumulations_; }
  std::uint64_t hNonzero() const { return h_nonzero_; }
  std::uint64_t hZero() const { return h_zero_; }
  std::uint64_t hNonfinite() const { return h_nonfinite_; }
  std::uint64_t bAccumulations() const { return b_accumulations_; }
  std::uint64_t bNonzero() const { return b_nonzero_; }
  std::uint64_t bZero() const { return b_zero_; }
  std::uint64_t bNonfinite() const { return b_nonfinite_; }
  const BoundedNormSummary& hNorms() const { return h_norms_; }
  const BoundedNormSummary& bNorms() const { return b_norms_; }

 private:
  void recordQuery(std::uint64_t& outcome, std::uint64_t& solver_outcome) {
    if (!enabled_) return;
    ++query_attempts_;
    ++outcome;
    if (context_ == Context::INITIAL) ++initial_query_attempts_;
    else ++solver_query_attempts_;
    if (context_ == Context::SOLVER) ++solver_outcome;
  }
  static void classify(double norm, bool finite, std::uint64_t& total,
                       std::uint64_t& nonzero, std::uint64_t& zero,
                       std::uint64_t& nonfinite, BoundedNormSummary& norms) {
    ++total;
    if (!finite || !std::isfinite(norm)) { ++nonfinite; return; }
    norms.add(norm);
    if (norm == 0.0) ++zero; else ++nonzero;
  }

  bool enabled_ = false;
  Context context_ = Context::INITIAL;
  std::uint64_t initial_query_attempts_ = 0;
  std::uint64_t solver_query_attempts_ = 0, solver_query_hits_ = 0;
  std::uint64_t solver_query_misses_ = 0, solver_query_rejected_explicit_ = 0;
  std::uint64_t initial_candidate_observations_ = 0;
  std::uint64_t initial_valid_observations_ = 0, initial_rejected_observations_ = 0;
  std::uint64_t solver_candidate_observations_ = 0;
  std::uint64_t solver_valid_observations_ = 0, solver_rejected_observations_ = 0;
  std::uint64_t initial_measured_frames_ = 0, solver_measured_frames_ = 0;
  std::uint64_t initial_residual_samples_ = 0, solver_residual_samples_ = 0;
  std::uint64_t query_attempts_ = 0, query_hits_ = 0, query_misses_ = 0;
  std::uint64_t query_rejected_explicit_ = 0;
  std::uint64_t candidate_observations_ = 0, valid_observations_ = 0;
  std::uint64_t rejected_observations_ = 0, measurement_frames_ = 0;
  std::uint64_t residual_samples_ = 0;
  std::uint64_t h_accumulations_ = 0, h_nonzero_ = 0, h_zero_ = 0, h_nonfinite_ = 0;
  std::uint64_t b_accumulations_ = 0, b_nonzero_ = 0, b_zero_ = 0, b_nonfinite_ = 0;
  BoundedNormSummary h_norms_, b_norms_;
};

}  // namespace LI2Sup
