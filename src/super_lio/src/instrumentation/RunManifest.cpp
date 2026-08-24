#include "instrumentation/RunManifest.h"

#include <sys/utsname.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace LI2Sup {

namespace {

std::string exec(const std::string& cmd) {
  std::array<char, 256> buf;
  std::string out;
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) return "";
  while (fgets(buf.data(), static_cast<int>(buf.size()), pipe)) {
    out += buf.data();
  }
  pclose(pipe);
  while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) {
    out.pop_back();
  }
  return out;
}

}  // namespace

std::string RunManifest::gitSha(const std::string& repo_root) {
  return exec("git -C " + repo_root + " rev-parse HEAD 2>/dev/null");
}

std::string RunManifest::gitDirtyCount(const std::string& repo_root) {
  std::string out =
      exec("git -C " + repo_root +
           " status --porcelain 2>/dev/null | grep -v '.scratch' | wc -l");
  return out.empty() ? "0" : out;
}

std::string RunManifest::branch(const std::string& repo_root) {
  return exec("git -C " + repo_root + " branch --show-current 2>/dev/null");
}

std::string RunManifest::hostname() {
  char buf[256];
  if (gethostname(buf, sizeof(buf)) != 0) return "unknown";
  return std::string(buf);
}

std::string RunManifest::cpuModel() {
  std::ifstream f("/proc/cpuinfo");
  std::string line;
  while (std::getline(f, line)) {
    if (line.rfind("model name", 0) == 0) {
      auto pos = line.find(':');
      if (pos != std::string::npos) return line.substr(pos + 2);
    }
  }
  return "unknown";
}

std::string RunManifest::buildType() {
#ifdef NDEBUG
  return "Release";
#else
  return "Debug";
#endif
}

std::string RunManifest::compiler() { return std::string("gcc ") + __VERSION__; }

std::string RunManifest::rosDistro() {
  const char* d = getenv("ROS_DISTRO");
  return d ? std::string(d) : "unknown";
}

std::string RunManifest::nowUtc() {
  auto now = std::chrono::system_clock::now();
  auto t = std::chrono::system_clock::to_time_t(now);
  std::tm tm_utc;
  gmtime_r(&t, &tm_utc);
  std::ostringstream oss;
  oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%SZ");
  return oss.str();
}

std::string RunManifest::buildYaml(const ManifestFields& fields) {
  std::ostringstream oss;
  oss << "git_sha: " << gitSha(fields.repo_root) << "\n";
  oss << "git_dirty: " << gitDirtyCount(fields.repo_root) << "\n";
  oss << "branch: " << branch(fields.repo_root) << "\n";
  oss << "dataset: " << fields.dataset << "\n";
  oss << "bag: " << fields.bag << "\n";
  oss << "playback_rate: " << fields.playback_rate << "\n";
  oss << "start_offset: " << fields.start_offset << "\n";
  oss << "duration: " << fields.duration << "\n";
  oss << "config: " << fields.config << "\n";
  oss << "config_hash: " << fields.config_hash << "\n";
  oss << "mode: " << fields.mode << "\n";
  oss << "input_mode: " << fields.input_mode << "\n";
  oss << "camera_enabled: " << fields.camera_enabled << "\n";
  oss << "sync_mode: " << fields.sync_mode << "\n";
  oss << "ros_distro: " << rosDistro() << "\n";
  oss << "build_type: " << buildType() << "\n";
  oss << "compiler: " << compiler() << "\n";
  oss << "hostname: " << hostname() << "\n";
  oss << "cpu: " << cpuModel() << "\n";
  oss << "run_start_time: " << nowUtc() << "\n";
  return oss.str();
}

}  // namespace LI2Sup