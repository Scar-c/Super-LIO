#include "dec_lio/D3Solver.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <Eigen/Cholesky>
#include <Eigen/LU>

namespace {

using DecLIO::Matrix18d;
using DecLIO::Matrix6d;
using DecLIO::PCGConfig;
using DecLIO::PCGResult;
using DecLIO::Vector6d;
using DecLIO::Vector18d;
using Clock = std::chrono::steady_clock;

struct Snapshot {
  std::uint64_t frame = 0;
  int iteration = 0;
  double timestamp = 0.0;
  std::uint64_t n_used = 0;
  std::uint64_t source_index = 0;
  Matrix18d A = Matrix18d::Zero();
  Vector18d rhs = Vector18d::Zero();
  Matrix6d lidar_information = Matrix6d::Zero();
  Vector6d lidar_rhs = Vector6d::Zero();
  Vector18d dx_prior = Vector18d::Zero();
  double kappa_R = 0.0;
  double kappa_t = 0.0;
  int weak_rank_R = 0;
  int weak_rank_t = 0;
  double fused_condition = 0.0;
};

struct ManifestRow {
  std::string scene;
  int selected_rank = 0;
};

template <typename T>
bool readValue(std::ifstream& stream, T* value) {
  return static_cast<bool>(stream.read(reinterpret_cast<char*>(value), sizeof(T)));
}

bool readSnapshot(std::ifstream& stream, Snapshot* snapshot) {
  return readValue(stream, &snapshot->frame) &&
         readValue(stream, &snapshot->iteration) &&
         readValue(stream, &snapshot->timestamp) &&
         readValue(stream, &snapshot->n_used) &&
         readValue(stream, &snapshot->source_index) &&
         static_cast<bool>(stream.read(reinterpret_cast<char*>(snapshot->A.data()),
                                       sizeof(double) * 18 * 18)) &&
         static_cast<bool>(stream.read(reinterpret_cast<char*>(snapshot->rhs.data()),
                                       sizeof(double) * 18)) &&
         static_cast<bool>(stream.read(reinterpret_cast<char*>(snapshot->lidar_information.data()),
                                       sizeof(double) * 6 * 6)) &&
         static_cast<bool>(stream.read(reinterpret_cast<char*>(snapshot->lidar_rhs.data()),
                                       sizeof(double) * 6)) &&
         static_cast<bool>(stream.read(reinterpret_cast<char*>(snapshot->dx_prior.data()),
                                       sizeof(double) * 18)) &&
         readValue(stream, &snapshot->kappa_R) &&
         readValue(stream, &snapshot->kappa_t) &&
         readValue(stream, &snapshot->weak_rank_R) &&
         readValue(stream, &snapshot->weak_rank_t) &&
         readValue(stream, &snapshot->fused_condition);
}

std::vector<Snapshot> readSnapshots(const std::string& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) throw std::runtime_error("cannot open selected snapshot binary");
  char magic[8] = {};
  std::uint32_t version = 0;
  if (!stream.read(magic, sizeof(magic)) || !readValue(stream, &version) ||
      std::string(magic, sizeof(magic)) != "D3SNAP01" || version != 2) {
    throw std::runtime_error("unsupported selected snapshot header");
  }
  std::vector<Snapshot> snapshots;
  Snapshot snapshot;
  while (readSnapshot(stream, &snapshot)) snapshots.push_back(snapshot);
  if (!stream.eof()) throw std::runtime_error("truncated selected snapshot record");
  return snapshots;
}

std::vector<ManifestRow> readManifest(const std::string& path) {
  std::ifstream stream(path);
  if (!stream) throw std::runtime_error("cannot open selection manifest");
  std::string line;
  std::getline(stream, line);
  std::vector<ManifestRow> rows;
  while (std::getline(stream, line)) {
    std::stringstream parser(line);
    std::string field;
    ManifestRow row;
    std::getline(parser, field, ',');
    row.selected_rank = std::stoi(field);
    std::getline(parser, row.scene, ',');
    rows.push_back(row);
  }
  return rows;
}

double elapsedUs(const Clock::time_point& start) {
  return std::chrono::duration<double, std::micro>(Clock::now() - start).count();
}

Vector18d nativeSolve(const Snapshot& snapshot) {
  Matrix18d H = Matrix18d::Zero();
  H.block<6, 6>(0, 0) = snapshot.lidar_information;
  Vector18d b = Vector18d::Zero();
  b.head<6>() = snapshot.lidar_rhs;
  const Matrix18d Q = snapshot.A.inverse();
  return Q * b + (Q * H - Matrix18d::Identity()) * snapshot.dx_prior;
}

Vector18d ldltSolve(const Snapshot& snapshot) {
  Eigen::LDLT<Matrix18d> solver(snapshot.A);
  return solver.solve(snapshot.rhs);
}

double relativeError(const Vector18d& lhs, const Vector18d& rhs) {
  return (lhs - rhs).norm() / std::max(rhs.norm(), 1e-12);
}

double residual(const Snapshot& snapshot, const Vector18d& value) {
  return (snapshot.A * value - snapshot.rhs).norm() /
         std::max(snapshot.rhs.norm(), 1e-12);
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 4 || argc > 6) {
    std::cerr << "usage: d3_solver_benchmark selected.bin manifest.csv output.csv [warmup] [reps]\n";
    return 2;
  }
  const int warmup = argc >= 4 ? std::stoi(argv[4]) : 500;
  const int reps = argc >= 5 ? std::stoi(argv[5]) : 500;
  if (warmup < 500 || reps < 500) {
    std::cerr << "warmup and reps must both be >= 500\n";
    return 2;
  }
  const std::vector<Snapshot> snapshots = readSnapshots(argv[1]);
  const std::vector<ManifestRow> manifest = readManifest(argv[2]);
  if (snapshots.size() != manifest.size()) {
    std::cerr << "snapshot/manifest size mismatch\n";
    return 3;
  }
  std::ofstream output(argv[3]);
  if (!output) return 4;
  output << std::setprecision(17)
         << "selected_rank,scene,source_snapshot_index,frame,ieskf_iteration,"
            "A_condition,kappa_R,kappa_t,weak_rank_R,weak_rank_t,"
            "native_inverse_us,ldlt_factor_solve_us,cg_us,jacobi_setup_us,"
            "jacobi_solve_us,jacobi_total_us,dcreg_setup_us,dcreg_solve_us,"
            "dcreg_total_us,cg_converged,cg_iterations,cg_residual,"
            "jacobi_converged,jacobi_iterations,jacobi_residual,dcreg_valid,"
            "dcreg_converged,dcreg_iterations,dcreg_residual,ldlt_vs_native,"
            "cg_vs_ldlt,jacobi_vs_ldlt,dcreg_vs_ldlt\n";
  volatile double sink = 0.0;
  const PCGConfig config;
  for (std::size_t index = 0; index < snapshots.size(); ++index) {
    const Snapshot& snapshot = snapshots[index];
    Matrix18d H = Matrix18d::Zero();
    H.block<6, 6>(0, 0) = snapshot.lidar_information;
    for (int repeat = 0; repeat < warmup; ++repeat) {
      sink += nativeSolve(snapshot)(0);
      sink += ldltSolve(snapshot)(0);
      sink += DecLIO::solvePCG(snapshot.A, snapshot.rhs,
                               Matrix18d::Identity(), config).solution(0);
      Matrix18d jacobi;
      std::string failure;
      DecLIO::buildJacobiPreconditioner(snapshot.A, &jacobi, &failure);
      sink += DecLIO::solvePCG(snapshot.A, snapshot.rhs, jacobi, config).solution(0);
      Matrix18d dcreg;
      DecLIO::Characterization characterization;
      DecLIO::buildDCRegPreconditioner(snapshot.A, H, snapshot.lidar_rhs,
                                       10.0, 10.0, &dcreg, &characterization,
                                       &failure);
      sink += DecLIO::solvePCG(snapshot.A, snapshot.rhs, dcreg, config).solution(0);
    }

    const auto native_start = Clock::now();
    Vector18d native = Vector18d::Zero();
    for (int repeat = 0; repeat < reps; ++repeat) native = nativeSolve(snapshot);
    const double native_us = elapsedUs(native_start) / reps;

    const auto ldlt_start = Clock::now();
    Vector18d ldlt = Vector18d::Zero();
    for (int repeat = 0; repeat < reps; ++repeat) ldlt = ldltSolve(snapshot);
    const double ldlt_us = elapsedUs(ldlt_start) / reps;

    const auto cg_start = Clock::now();
    PCGResult cg;
    for (int repeat = 0; repeat < reps; ++repeat)
      cg = DecLIO::solvePCG(snapshot.A, snapshot.rhs, Matrix18d::Identity(), config);
    const double cg_us = elapsedUs(cg_start) / reps;

    const auto jacobi_setup_start = Clock::now();
    Matrix18d jacobi;
    std::string jacobi_failure;
    for (int repeat = 0; repeat < reps; ++repeat)
      DecLIO::buildJacobiPreconditioner(snapshot.A, &jacobi, &jacobi_failure);
    const double jacobi_setup_us = elapsedUs(jacobi_setup_start) / reps;
    const auto jacobi_start = Clock::now();
    PCGResult jacobi_result;
    for (int repeat = 0; repeat < reps; ++repeat)
      jacobi_result = DecLIO::solvePCG(snapshot.A, snapshot.rhs, jacobi, config);
    const double jacobi_solve_us = elapsedUs(jacobi_start) / reps;

    const auto dcreg_setup_start = Clock::now();
    Matrix18d dcreg;
    DecLIO::Characterization characterization;
    std::string dcreg_failure;
    bool dcreg_valid = false;
    for (int repeat = 0; repeat < reps; ++repeat)
      dcreg_valid = DecLIO::buildDCRegPreconditioner(
          snapshot.A, H, snapshot.lidar_rhs, 10.0, 10.0, &dcreg,
          &characterization, &dcreg_failure);
    const double dcreg_setup_us = elapsedUs(dcreg_setup_start) / reps;
    const auto dcreg_start = Clock::now();
    PCGResult dcreg_result;
    for (int repeat = 0; repeat < reps; ++repeat)
      dcreg_result = DecLIO::solvePCG(snapshot.A, snapshot.rhs, dcreg, config);
    const double dcreg_solve_us = elapsedUs(dcreg_start) / reps;

    output << index << ',' << manifest[index].scene << ',' << snapshot.source_index
           << ',' << snapshot.frame << ',' << snapshot.iteration << ','
           << snapshot.fused_condition << ',' << snapshot.kappa_R << ','
           << snapshot.kappa_t << ',' << snapshot.weak_rank_R << ','
           << snapshot.weak_rank_t << ',' << native_us << ',' << ldlt_us << ','
           << cg_us << ',' << jacobi_setup_us << ',' << jacobi_solve_us << ','
           << jacobi_setup_us + jacobi_solve_us << ',' << dcreg_setup_us << ','
           << dcreg_solve_us << ',' << dcreg_setup_us + dcreg_solve_us << ','
           << cg.converged << ',' << cg.iterations << ',' << residual(snapshot, cg.solution)
           << ',' << jacobi_result.converged << ',' << jacobi_result.iterations << ','
           << residual(snapshot, jacobi_result.solution) << ',' << dcreg_valid << ','
           << dcreg_result.converged << ',' << dcreg_result.iterations << ','
           << residual(snapshot, dcreg_result.solution) << ','
           << relativeError(ldlt, native) << ',' << relativeError(cg.solution, ldlt)
           << ',' << relativeError(jacobi_result.solution, ldlt) << ','
           << relativeError(dcreg_result.solution, ldlt) << '\n';
  }
  std::cerr << "benchmark_snapshots=" << snapshots.size() << " warmup=" << warmup
            << " reps=" << reps << " sink=" << sink << '\n';
  return 0;
}
