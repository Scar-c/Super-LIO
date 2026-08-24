#include "camera/CameraCalibration.h"

#include <cstdio>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>

namespace LI2Sup {

namespace {

// Minimal NTU VIRAL yaml subset parser. Only the fields we consume.
class YamlLite {
 public:
  explicit YamlLite(const std::string& path) {
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line)) {
      if (line.empty() || line[0] == '#') continue;
      auto colon = line.find(':');
      if (colon == std::string::npos) continue;
      std::string key = line.substr(0, colon);
      std::string val = line.substr(colon + 1);
      key = trim(key);
      val = trim(val);
      if (!key.empty()) map_[key] = val;
    }
  }

  bool has(const std::string& key) const { return map_.count(key) > 0; }

  bool getString(const std::string& key, std::string& out) const {
    auto it = map_.find(key);
    if (it == map_.end()) return false;
    out = it->second;
    return true;
  }

  bool getDouble(const std::string& key, double& out) const {
    auto it = map_.find(key);
    if (it == map_.end()) return false;
    try {
      out = std::stod(it->second);
      return true;
    } catch (...) {
      return false;
    }
  }

  bool getInt(const std::string& key, int& out) const {
    auto it = map_.find(key);
    if (it == map_.end()) return false;
    try {
      out = std::stoi(it->second);
      return true;
    } catch (...) {
      return false;
    }
  }

  // data: [ v0, v1, ... ] possibly spanning lines: consume until ']'
  bool getMatrixData(std::ifstream& f, std::vector<double>& out) const;

 private:
  static std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n\"");
    size_t e = s.find_last_not_of(" \t\r\n\"");
    if (b == std::string::npos) return "";
    return s.substr(b, e - b + 1);
  }

  std::map<std::string, std::string> map_;
};

bool YamlLite::getMatrixData(std::ifstream& f, std::vector<double>& out) const {
  std::string line;
  while (std::getline(f, line)) {
    if (line.find(']') != std::string::npos) return true;
  }
  return false;
}

}  // namespace

bool parseCameraCalibrationFile(const std::string& path,
                                CameraCalibration& calib) {
  calib = CameraCalibration{};
  calib.calib_file = path;

  std::ifstream f(path);
  if (!f.is_open()) {
    std::printf("[CameraCalibration] ERROR: cannot open %s\n", path.c_str());
    return false;
  }

  YamlLite y(path);

  int w = 0, h = 0;
  if (!y.getInt("image_width", w) || !y.getInt("image_height", h)) {
    std::printf("[CameraCalibration] ERROR: missing image size in %s\n",
                path.c_str());
    return false;
  }

  double fx = 0, fy = 0, cx = 0, cy = 0;
  if (!y.getDouble("fx", fx) || !y.getDouble("fy", fy) ||
      !y.getDouble("cx", cx) || !y.getDouble("cy", cy)) {
    std::printf("[CameraCalibration] ERROR: missing projection in %s\n",
                path.c_str());
    return false;
  }

  // T_Body_Cam: opencv-matrix block
  //   T_Body_Cam: !!opencv-matrix
  //      rows: 4 ... data: [ ... ]
  Eigen::Matrix4d T_body_cam = Eigen::Matrix4d::Identity();
  {
    std::ifstream f2(path);
    std::string line;
    bool in_block = false;
    bool in_data = false;
    std::vector<double> vals;
    while (std::getline(f2, line)) {
      if (line.find("T_Body_Cam") != std::string::npos) {
        in_block = true;
        continue;
      }
      if (!in_block) continue;
      auto trim = [](std::string s) {
        size_t b = s.find_first_not_of(" \t\r\n");
        size_t e = s.find_last_not_of(" \t\r\n");
        return b == std::string::npos ? std::string() : s.substr(b, e - b + 1);
      };
      if (!in_data) {
        auto colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string key = trim(line.substr(0, colon));
        std::string val = trim(line.substr(colon + 1));
        if (key == "rows" || key == "cols" || key == "dt") continue;
        if (key == "data") in_data = true;
        if (!in_data) continue;
        line = val;
      }
      // data continuation lines have no colon; accumulate until ']'
      if (line.find("[") != std::string::npos) {
        line = line.substr(line.find("[") + 1);
      }
      size_t close_pos = line.find("]");
      if (close_pos != std::string::npos) line = line.substr(0, close_pos);
      std::stringstream ss(line);
      std::string tok;
      while (std::getline(ss, tok, ',')) {
        try {
          vals.push_back(std::stod(tok));
        } catch (...) {
        }
      }
      if (close_pos != std::string::npos) break;
    }
    if (vals.size() == 16) {
      for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c) T_body_cam(r, c) = vals[r * 4 + c];
    } else {
      std::printf(
          "[CameraCalibration] ERROR: T_Body_Cam needs 16 values in %s (got "
          "%zu)\n",
          path.c_str(), vals.size());
      return false;
    }
  }

  calib.image_width = w;
  calib.image_height = h;
  calib.fx = fx;
  calib.fy = fy;
  calib.cx = cx;
  calib.cy = cy;
  calib.T_body_cam = T_body_cam;
  y.getString("dataset_name", calib.dataset_name);
  y.getString("image_topic", calib.image_topic);
  y.getString("camera_name", calib.camera_name);
  y.getString("model_type", calib.model_type);
  y.getString("distortion_model", calib.distortion_model);
  y.getDouble("k1", calib.k1);
  y.getDouble("k2", calib.k2);
  y.getDouble("p1", calib.p1);
  y.getDouble("p2", calib.p2);
  calib.valid = true;
  return true;
}

}  // namespace LI2Sup