#ifndef DEC_LIO_PAIRED_ATTENUATION_H_
#define DEC_LIO_PAIRED_ATTENUATION_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>

#include <Eigen/Core>

#include "dec_lio/DCRegAnalyzer.h"

namespace DecLIO {

enum PairedAttenuationMode {
  kPairedModeOff = 0,
  kPairedModeP1Directional = 1,
  kPairedModeUTrace = 2,
  kPairedModeUGamma = 3,
};

struct PairedAttenuationResult {
  bool dcreg_valid = false;
  bool attenuation_valid = false;
  bool attenuation_applied = false;
  int weak_rank_rot = 0;
  int weak_rank_trans = 0;
  int lifted_weak_rank = 0;
  double cond_rot = 0.0;
  double cond_trans = 0.0;
  double rho_weak_min = 1.0;
  double gamma_w = 1.0;
  double projector_symmetry_error = 0.0;
  double projector_idempotence_error = 0.0;
  double H_asym_rel = 0.0;
  double H_lambda_min = 0.0;
  double H_lambda_max = 0.0;
  double trace_raw_H = 0.0;
  double trace_att_H = 0.0;
  double trace_ratio = 1.0;
  double b_raw_norm = 0.0;
  double b_att_norm = 0.0;
  double b_ratio = 1.0;
  double min_eig_Hatt = 0.0;
  double min_eig_information_removed = 0.0;
  double modal_minimizer_error = 0.0;
  double strong_mode_max_reduction = 0.0;
  double dcreg_us = 0.0;
  double lift_us = 0.0;
  double spectral_us = 0.0;
  double total_us = 0.0;
  std::array<double, 6> gamma{{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
  std::array<double, 6> weak_occupancy{{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
  Matrix6d weak_projector = Matrix6d::Zero();
  Matrix6d lifted_weak_columns = Matrix6d::Zero();
  Matrix6d raw_H = Matrix6d::Zero();
  Matrix6d attenuated_H = Matrix6d::Zero();
  Vector6d raw_b = Vector6d::Zero();
  Vector6d attenuated_b = Vector6d::Zero();
  std::string fail_open_reason = "NONE";
};

double pairedGammaFromRho(double rho, double condition_threshold = 10.0);

PairedAttenuationResult computePairedAttenuation(
    const Matrix6d& raw_H, const Vector6d& raw_b,
    double condition_threshold = 10.0);

struct PairedControlResult {
  int mode = kPairedModeOff;
  bool valid = true;
  bool applied = false;
  double scalar = 1.0;
  double trace_H = 0.0;
  double trace_ratio = 1.0;
  double b_norm = 0.0;
  Matrix6d H = Matrix6d::Zero();
  Vector6d b = Vector6d::Zero();
  std::string failure_reason = "NONE";
};

PairedControlResult makePairedControl(
    int mode, const Matrix6d& raw_H, const Vector6d& raw_b,
    const PairedAttenuationResult& p1);

struct PairedAttenuationObservation {
  std::uint64_t frame = 0;
  int ieskf_iteration = 0;
  double timestamp = 0.0;
  std::size_t n_used = 0;
  int mode = kPairedModeOff;
  bool control_valid = true;
  bool control_applied = false;
  double control_scalar = 1.0;
  double control_us = 0.0;
  double trace_control_H = 0.0;
  double trace_control_ratio = 1.0;
  double b_control_norm = 0.0;
  std::string control_fail_reason = "NONE";
  bool shadow_only = true;
  bool counterfactual_finite = true;
  double raw_fused_dx_norm = 0.0;
  double counterfactual_fused_dx_norm = 0.0;
  double counterfactual_minus_raw_norm = 0.0;
  double weak_update_difference_norm = 0.0;
  double complement_update_difference_norm = 0.0;
  PairedAttenuationResult result;
};

class PairedAttenuationAudit {
 public:
  explicit PairedAttenuationAudit(const std::string& csv_path);
  ~PairedAttenuationAudit();

  PairedAttenuationAudit(const PairedAttenuationAudit&) = delete;
  PairedAttenuationAudit& operator=(const PairedAttenuationAudit&) = delete;

  void record(const PairedAttenuationObservation& observation);
  void finalize();

 private:
  void writeHeader();

  std::ofstream csv_;
  std::ofstream timing_csv_;
  bool finalized_ = false;
};

}  // namespace DecLIO

#endif
