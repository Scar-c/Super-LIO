#ifndef CAMERA_CALIBRATION_H_
#define CAMERA_CALIBRATION_H_

#include <string>

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace LI2Sup {

// Transform convention (see docs/super_livo/datasets/calibration_time_sync.md):
//   p_A = T_A_B * p_B
// NTU VIRAL yaml defines T_Body_Cam, i.e. camera expressed in Body(=IMU) frame:
//   p_body = T_Body_Cam * p_cam
struct CameraCalibration {
  bool valid = false;

  std::string dataset_name;
  std::string image_topic;
  std::string camera_name;
  std::string model_type;   // "PINHOLE" (radtan) first supported
  std::string distortion_model;
  int image_width = 0;
  int image_height = 0;

  double fx = 0.0, fy = 0.0, cx = 0.0, cy = 0.0;
  double k1 = 0.0, k2 = 0.0, p1 = 0.0, p2 = 0.0;

  Eigen::Matrix4d T_body_cam = Eigen::Matrix4d::Identity();
  std::string calib_file;
  std::string calib_hash;

  Eigen::Matrix4d T_cam_body() const { return T_body_cam.inverse(); }
};

// Lightweight parser for NTU VIRAL-style camera yaml (fixed field subset).
// Returns false and leaves calib.valid=false on any parse failure.
bool parseCameraCalibrationFile(const std::string& path,
                                CameraCalibration& calib);

// A->B frame transform for a body point: p_cam = T_cam_body * p_body.
inline Eigen::Vector3d transformPoint(const Eigen::Matrix4d& T,
                                      const Eigen::Vector3d& p) {
  const Eigen::Vector4d ph = T * Eigen::Vector4d(p.x(), p.y(), p.z(), 1.0);
  return Eigen::Vector3d(ph.x(), ph.y(), ph.z());
}

}  // namespace LI2Sup

#endif  // CAMERA_CALIBRATION_H_