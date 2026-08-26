#!/usr/bin/env python3
"""Derive Super-LIO Oxford Quarter01 configs from the OFFICIAL calibration.

Read-only inputs (Oxford Spires official):
  cam0.yaml              Kalibr intrinsics (equidistant k1..k4)
  cam-lidar-imu.yaml     DiffCal extrinsics (T_cam_lidar, C_q_CI/C_r_CI)
  imu.yaml               Allan-variance IMU noise

Conventions (official README.docx):
  T_cam_lidar: lidar frame -> camera frame
  C_q_CI/C_r_CI: IMU frame -> camera frame (C = camera, I = IMU)
  Body = IMU

Derived:
  T_Body_Lidar = inv(T_C_I) * T_C_L
  T_Body_Cam   = inv(T_C_I)
  camera intrinsics fx/fy/cx/cy + distortion (Super-LIO calib file format)
  IMU noise imu_na/imu_ng/imu_nba/imu_nbg

No invented values: everything derives from the official files. Camera
time offset t_shift = 0.0 (Oxford Spires common-clock hardware sync;
no per-sensor offset published in the provided calibration).
"""
import argparse
import math
import pathlib
import sys
import yaml


def quat_to_rot(q):
    """quaternion (x,y,z,w) -> 3x3 rotation matrix."""
    x, y, z, w = q
    return [
        [1 - 2 * (y * y + z * z), 2 * (x * y - w * z), 2 * (x * z + w * y)],
        [2 * (x * y + w * z), 1 - 2 * (x * x + z * z), 2 * (y * z - w * x)],
        [2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y)],
    ]


def mat_mul(A, B):
    n = len(A)
    m = len(B[0])
    p = len(B)
    return [[sum(A[i][k] * B[k][j] for k in range(p)) for j in range(m)]
            for i in range(n)]


def rigid_inv(T):
    R = [[T[i][j] for j in range(3)] for i in range(3)]
    t = [T[i][3] for i in range(3)]
    Rt = [[R[j][i] for j in range(3)] for i in range(3)]
    tb = [-sum(Rt[i][j] * t[j] for j in range(3)) for i in range(3)]
    return [Rt[i] + [tb[i]] for i in range(3)] + [[0, 0, 0, 1]]


def to_4x4(R, t):
    return [R[i] + [t[i]] for i in range(3)] + [[0, 0, 0, 1]]


def flatten(T):
    return [val for row in T for val in row]


def fmt_row_major(values, per_line=4):
    lines = []
    for i in range(0, len(values), per_line):
        lines.append("    - " + " ".join("%.12g" % v for v in values[i:i + per_line]))
    return "\n".join(lines)


def derive(calib_dir):
    with open(calib_dir / "cam-lidar-imu.yaml") as f:
        ext = yaml.safe_load(f)
    with open(calib_dir / "cam0.yaml") as f:
        cam0 = yaml.safe_load(f)
    with open(calib_dir / "imu.yaml") as f:
        imu = yaml.safe_load(f)

    cam = ext["cam0"]
    T_C_L = cam["T_cam_lidar"]
    q_CI = cam["C_q_CI"]
    r_CI = cam["C_r_CI"]
    T_C_I = to_4x4(quat_to_rot(q_CI), r_CI)
    T_I_C = rigid_inv(T_C_I)
    T_I_L = mat_mul(T_I_C, T_C_L)

    cm = cam0["camera_matrix"]["data"]
    fx, fy, cx, cy = cm[0], cm[4], cm[2], cm[5]
    w, h = cam0["image_width"], cam0["image_height"]
    dist = cam0["distortion_coefficients"]["data"]

    return {
        "T_I_L": T_I_L,
        "T_I_C": T_I_C,
        "fx": fx, "fy": fy, "cx": cx, "cy": cy,
        "w": w, "h": h,
        "dist": dist,
        "na": imu["accelerometer_noise_density"],
        "ng": imu["gyroscope_noise_density"],
        "nba": imu["accelerometer_random_walk"],
        "nbg": imu["gyroscope_random_walk"],
    }


def render_config(d, lidar_topic, imu_topic, camera_topic, calib_path):
    T = d["T_I_L"]
    return f"""# Oxford Quarter01 (Oxford Spires Multi-Modal) — derived from official
# calibration via prepare_oxford_calibration.py (Round11AA).
lio:
  map:
    save_map: false
    if_filter: true
    save_map_dir: "map"
    map_name: "map.pcd"
    ds_size: 0.25
    save_interval: 500
  ros:
    lidar_topic: "{lidar_topic}"
    imu_topic: "{imu_topic}"
  sensor:
    lidar_type: 2
    blind: 2.0
    maxrange: 60.0
    filter_rate: 3.0
    enable_downsample: true
    voxel_fliter_size: 0.5
    gravity_norm: 9.81
    imu_type: 1
    imu_na: {d['na']:.15g}
    imu_ng: {d['ng']:.15g}
    imu_nba: {d['nba']:.15g}
    imu_nbg: {d['nbg']:.15g}
  extrinsic:
    lidar_imu:
{fmt_row_major(flatten(T))}
    odom_robo: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
  hash_map:
    hash_capacity: 2000000
    vox_resolution: 0.5
  kf:
    kf_type: 0
    kf_max_iterations: 4
    kf_align_gravity: true
    kf_quit_eps: 0.001
  output:
    robot: false
    plan_env_world: false
    plan_env_body: false
    ml_map: false
    planner: false
    map: false
    dense: true
    pub_step: 1
  eva:
    timer: true
"""


def render_camera_calib(d, camera_topic):
    T = d["T_I_C"]
    data = " ".join("%.12g" % v for v in flatten(T))
    return f"""T_Body_Cam: !!opencv-matrix
   rows: 4
   cols: 4
   dt: d
   data: [{data}]
# Oxford Quarter01 cam0 (Alphasense; Body = IMU); derived via
# prepare_oxford_calibration.py; topic {camera_topic}
t_shift: 0.0
fx: {d['fx']:.15g}
fy: {d['fy']:.15g}
cx: {d['cx']:.15g}
cy: {d['cy']:.15g}
image_width: {d['w']}
image_height: {d['h']}
distortion: [{', '.join('%.15g' % v for v in d['dist'])}]
"""


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--calib-dir", required=True,
                    help="Oxford Calibration directory (official files)")
    ap.add_argument("--config-out", required=True)
    ap.add_argument("--camera-calib-out", required=True)
    ap.add_argument("--lidar-topic", default="/hesai/pandar")
    ap.add_argument("--imu-topic", default="/alphasense_driver_ros/imu")
    ap.add_argument("--camera-topic",
                    default="/alphasense_driver_ros/cam0/debayered/image/compressed")
    args = ap.parse_args(argv)

    d = derive(pathlib.Path(args.calib_dir))
    pathlib.Path(args.config_out).write_text(
        render_config(d, args.lidar_topic, args.imu_topic, args.camera_topic,
                      args.camera_calib_out))
    pathlib.Path(args.camera_calib_out).write_text(
        render_camera_calib(d, args.camera_topic))
    print("derived T_Body_Lidar:", flatten(d["T_I_L"]))
    print("derived T_Body_Cam:", flatten(d["T_I_C"]))
    print("intrinsics: fx=%.6f fy=%.6f cx=%.6f cy=%.6f %dx%d" %
          (d["fx"], d["fy"], d["cx"], d["cy"], d["w"], d["h"]))
    print("distortion:", d["dist"])
    print("imu noise: na=%.10g ng=%.10g nba=%.10g nbg=%.10g" %
          (d["na"], d["ng"], d["nba"], d["nbg"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
