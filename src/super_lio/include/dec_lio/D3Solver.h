#ifndef DEC_LIO_D3_SOLVER_H_
#define DEC_LIO_D3_SOLVER_H_

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>

#include <Eigen/Core>

#include "dec_lio/DCRegAnalyzer.h"

namespace DecLIO {

using Matrix18d = Eigen::Matrix<double, 18, 18>;
using Vector6d = Eigen::Matrix<double, 6, 1>;
using Vector18d = Eigen::Matrix<double, 18, 1>;

struct PCGConfig {
  int max_iterations = 36;
  double relative_tolerance = 1e-12;
  double absolute_tolerance = 1e-14;
};

struct PCGResult {
  Vector18d solution = Vector18d::Zero();
  bool converged = false;
  bool finite_guard_pass = true;
  int iterations = 0;
  double original_relative_residual = 0.0;
};

struct ShadowInput {
  std::uint64_t frame = 0;
  int ieskf_iteration = 0;
  double timestamp = 0.0;
  std::size_t n_used = 0;
  Matrix18d A = Matrix18d::Zero();
  Matrix18d lambda = Matrix18d::Zero();
  Matrix18d lidar_information = Matrix18d::Zero();
  Vector6d lidar_rhs = Vector6d::Zero();
  Vector18d rhs = Vector18d::Zero();
  Vector18d dx_prior = Vector18d::Zero();
  Vector18d native_dx = Vector18d::Zero();
};

PCGResult solvePCG(const Matrix18d& A, const Vector18d& rhs,
                   const Matrix18d& preconditioner,
                   const PCGConfig& config = PCGConfig());

bool buildJacobiPreconditioner(const Matrix18d& A, Matrix18d* preconditioner,
                               std::string* failure_reason);

bool buildDCRegPreconditioner(const Matrix18d& A,
                              const Matrix18d& lidar_information,
                              const Eigen::Matrix<double, 6, 1>& lidar_rhs,
                              double condition_threshold,
                              double kappa_target,
                              Matrix18d* preconditioner,
                              Characterization* characterization,
                              std::string* failure_reason);

double objective(const Matrix18d& A, const Vector18d& rhs,
                 const Vector18d& solution);

class D3SolverAudit {
 public:
  D3SolverAudit(const std::string& csv_path, const std::string& snapshot_path);
  ~D3SolverAudit();

  D3SolverAudit(const D3SolverAudit&) = delete;
  D3SolverAudit& operator=(const D3SolverAudit&) = delete;

  void record(const ShadowInput& input);
  void finalize();

 private:
  void writeHeader();

  std::ofstream csv_;
  std::ofstream snapshot_;
  std::uint64_t snapshot_count_ = 0;
  bool finalized_ = false;
};

}  // namespace DecLIO

#endif
