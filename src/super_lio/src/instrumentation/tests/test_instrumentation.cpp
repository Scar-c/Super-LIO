#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "instrumentation/CsvWriter.h"
#include "instrumentation/ExperimentLogger.h"
#include "instrumentation/RunManifest.h"
#include "instrumentation/RunStats.h"

using namespace LI2Sup;

int main() {
  RunningStats s;
  for (double v : {1.0, 2.0, 3.0, 4.0}) s.add(v);
  assert(s.count == 4);
  assert(std::abs(s.mean() - 2.5) < 1e-12);
  assert(std::abs(s.min - 1.0) < 1e-12);
  assert(std::abs(s.max - 4.0) < 1e-12);
  assert(std::abs(s.stddev() - std::sqrt(5.0 / 3.0)) < 1e-12);
  RunningStats empty;
  assert(empty.count == 0 && empty.mean() == 0.0 && empty.stddev() == 0.0);

  RunningStats a, b;
  for (double v : {1.0, 2.0}) a.add(v);
  for (double v : {3.0, 4.0}) b.add(v);
  a.merge(b);
  assert(a.count == 4 && std::abs(a.mean() - 2.5) < 1e-12);

  {
    std::string dir = "/tmp/opencode/tb0_test";
    std::filesystem::create_directories(dir);
    {
      CsvWriter w;
      assert(w.open(dir + "/t.csv", {"a", "b"}));
      w.writeRow({"1.0", "2.0"});
      w.writeRow({"3.0", "4.0"});
      w.close();
    }
    std::ifstream f(dir + "/t.csv");
    std::stringstream ss;
    ss << f.rdbuf();
    std::string content = ss.str();
    assert(content.find("a,b\n") == 0);
    assert(content.find("1.0,2.0\n") != std::string::npos);
    assert(content.find("3.0,4.0\n") != std::string::npos);
  }

  assert(1000ULL * 104ULL == 104000ULL);
  assert(1000ULL * 160ULL == 160000ULL);

  {
    ManifestFields f;
    f.repo_root = "/home/lc/super_livo/src/Super-LIO";
    f.dataset = "eee_01";
    f.bag = "/x/eee_01.bag";
    f.playback_rate = "1.0";
    f.config_hash = "deadbeef";
    std::string y = RunManifest::buildYaml(f);
    assert(y.find("git_sha: ") != std::string::npos);
    assert(y.find("branch: super-livo") != std::string::npos);
    assert(y.find("dataset: eee_01") != std::string::npos);
    assert(y.find("mode: lio_only_legacy") != std::string::npos);
    assert(y.find("camera_enabled: false") != std::string::npos);
    assert(y.find("sync_mode: lidar_end") != std::string::npos);
    assert(y.find("config_hash: deadbeef") != std::string::npos);
    assert(y.find("build_type: Release") != std::string::npos ||
           y.find("build_type: Debug") != std::string::npos);
    assert(y.find("hostname: ") != std::string::npos);
    assert(y.find("run_start_time: ") != std::string::npos);
  }

  {
    std::string dir = "/tmp/opencode/tb0_test/exp";
    std::filesystem::remove_all(dir);
    ExperimentLogger logger;
    ManifestFields f;
    f.repo_root = "/home/lc/super_livo/src/Super-LIO";
    f.dataset = "eee_01";
    assert(logger.open(dir, f));
    EpochTimings t;
    t.imu_propagation_ms = 0.5;
    t.undistortion_ms = 1.5;
    t.downsample_ms = 0.2;
    t.state_update_ms = 3.0;
    t.map_update_ms = 0.4;
    t.total_ms = 5.6;
    RunningStats r;
    for (double v : {0.1, 0.2, 0.3}) r.add(v);
    logger.recordEpoch(123.456, t, 1000, 4, r, 5000, 2000000);
    logger.recordEpoch(123.556, t, 990, 3, r, 5100, 2000000);
    logger.close();

    std::ifstream f1(dir + "/timing.csv");
    std::stringstream ss;
    ss << f1.rdbuf();
    assert(ss.str().find("timestamp,imu_propagation_ms") == 0);
    assert(ss.str().find("123.456000") != std::string::npos);
    assert(ss.str().find("0.500000,1.500000,0.200000,3.000000,0.400000,5.600000") !=
           std::string::npos);

    std::ifstream f2(dir + "/lio_stats.csv");
    ss.str("");
    ss << f2.rdbuf();
    assert(ss.str().find("timestamp,effective_points,iteration_count") == 0);
    assert(ss.str().find(",1000,4,") != std::string::npos);
    assert(ss.str().find(",990,3,") != std::string::npos);

    std::ifstream f3(dir + "/map_stats.csv");
    ss.str("");
    ss << f3.rdbuf();
    assert(ss.str().find("timestamp,octvox_voxel_count") == 0);
    assert(ss.str().find("5000,2000000,520000,800000") != std::string::npos);

    std::ifstream f4(dir + "/run_manifest.yaml");
    ss.str("");
    ss << f4.rdbuf();
    assert(ss.str().find("git_sha: ") != std::string::npos);
  }

  std::printf("all instrumentation tests passed\n");
  return 0;
}