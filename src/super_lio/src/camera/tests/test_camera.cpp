#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "camera/CameraCalibration.h"
#include "camera/CameraFrame.h"

using namespace LI2Sup;

namespace {

// Minimal NTU VIRAL-style camera yaml for tests.
void writeTestCalib(const std::string& path, const char* t_body_cam = nullptr) {
  std::ofstream f(path);
  f << "%YAML:1.0\n\n"
       "dataset_name: test\n"
       "image_topic: /left/image_raw\n"
       "model_type: PINHOLE\n"
       "camera_name: camera\n"
       "image_width: 752\n"
       "image_height: 480\n\n"
       "distortion_model: radial-tangential\n"
       "distortion_parameters:\n"
       "   k1: -0.288105327549552\n"
       "   k2: 0.074578284234601\n"
       "   p1: 0.000778\n"
       "   p2: -0.000227\n"
       "projection_parameters:\n"
       "   fx: 425.0258563\n"
       "   fy: 426.7976260\n"
       "   cx: 386.0151866\n"
       "   cy: 241.9130336\n\n";
  if (t_body_cam) {
    f << "T_Body_Cam: !!opencv-matrix\n"
         "   rows: 4\n"
         "   cols: 4\n"
         "   dt: d\n"
         "   data: [" << t_body_cam << "]\n";
  } else {
    f << "T_Body_Cam: !!opencv-matrix\n"
         "   rows: 4\n"
         "   cols: 4\n"
         "   dt: d\n"
         "   data: [ 1.0, 0.0, 0.0, 0.1,\n"
         "           0.0, 1.0, 0.0, 0.0,\n"
         "           0.0, 0.0, 1.0, 0.2,\n"
         "           0.0, 0.0, 0.0, 1.0 ]\n";
  }
}

}  // namespace

int main() {
  const std::string dir = "/tmp/opencode/tb1_test";
  std::filesystem::create_directories(dir);

  // 1) calibration parse
  {
    const std::string p = dir + "/cam.yaml";
    writeTestCalib(p);
    CameraCalibration c;
    assert(parseCameraCalibrationFile(p, c));
    assert(c.valid);
    assert(c.image_width == 752 && c.image_height == 480);
    assert(std::abs(c.fx - 425.0258563) < 1e-9);
    assert(std::abs(c.cy - 241.9130336) < 1e-9);
    assert(c.model_type == "PINHOLE");
    assert(c.T_body_cam(0, 3) == 0.1);
    assert(c.T_body_cam(2, 3) == 0.2);
    // T_cam_body inverse
    Eigen::Matrix4d T = c.T_cam_body();
    Eigen::Matrix4d I = T * c.T_body_cam;
    assert((I - Eigen::Matrix4d::Identity()).norm() < 1e-12);
  }

  // 2) malformed calibration
  {
    const std::string p = dir + "/bad.yaml";
    std::ofstream f(p);
    f << "image_width: 100\n";  // missing projection
    CameraCalibration c;
    assert(!parseCameraCalibrationFile(p, c));
    assert(!c.valid);
  }

  // 3) missing file
  {
    CameraCalibration c;
    assert(!parseCameraCalibrationFile(dir + "/nope.yaml", c));
  }

  // 4) transform convention: p_A = T_A_B * p_B
  {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T(0, 3) = 1.0;
    Eigen::Vector3d pb(0.5, 0.25, 0.125);
    Eigen::Vector3d pa = transformPoint(T, pb);
    assert(std::abs(pa.x() - 1.5) < 1e-12);
    assert(std::abs(pa.y() - 0.25) < 1e-12);
  }

  // 5) camera buffer boundedness + eviction + accounting
  {
    CameraBuffer buf(2);
    CameraFrame f1, f2, f3;
    f1.timestamp = 1.0;
    f2.timestamp = 2.0;
    f3.timestamp = 3.0;
    buf.push(f1);
    buf.push(f2);
    assert(buf.size() == 2);
    assert(buf.dropped() == 0);
    assert(buf.peakSize() == 2);
    buf.push(f3);  // evicts f1
    assert(buf.size() == 2);
    assert(buf.dropped() == 1);
    assert(buf.oldest().timestamp == 2.0);
    assert(buf.newest().timestamp == 3.0);
    assert(buf.firstTimestamp() == 1.0);
    assert(buf.lastTimestamp() == 3.0);
    assert(buf.peakSize() == 2);
    buf.setCapacity(1);
    buf.push(f1);  // evicts f3 and f2
    assert(buf.size() == 1);
    assert(buf.dropped() == 3);
    buf.clear();
    assert(buf.size() == 0 && buf.dropped() == 3);
  }

  // 6) buffer timestamps monotonic accounting
  {
    CameraBuffer buf(5);
    for (int i = 1; i <= 6; ++i) {
      CameraFrame f;
      f.timestamp = i;
      buf.push(f);
    }
    assert(buf.dropped() == 1);
    assert(buf.oldest().timestamp == 2.0);
    assert(buf.newest().timestamp == 6.0);
  }

  std::printf("all tb1 camera tests passed\n");
  return 0;
}