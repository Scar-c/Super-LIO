#include "common/CadencePolicy.h"

#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <vector>

using LI2Sup::CadenceAction;
using LI2Sup::CadenceInputs;
using LI2Sup::FullScanOwnershipAudit;
using LI2Sup::LidarUpdatePolicy;
using LI2Sup::parseLidarUpdatePolicy;
using LI2Sup::selectFullScanCadenceAction;

namespace {

void expect(const char* name, bool condition) {
  std::printf("%s: %s\n", name, condition ? "PASS" : "FAIL");
  assert(condition);
}

CadenceInputs readyWithinScan() {
  CadenceInputs in;
  in.filter_initialized = true;
  in.have_camera = true;
  in.camera_time = 1.04;
  in.camera_has_lidar_coverage = true;
  in.camera_has_imu_coverage = true;
  in.have_full_scan = true;
  in.full_scan_end_time = 1.10;
  in.full_scan_has_imu_coverage = true;
  return in;
}

}  // namespace

int main() {
  // X-T1: typed parser and fail-closed unknown value.
  expect("X-T1 partial parser",
         parseLidarUpdatePolicy("partial") == LidarUpdatePolicy::PARTIAL);
  expect("X-T1 shadow parser",
         parseLidarUpdatePolicy("shadow_fullscan") ==
             LidarUpdatePolicy::SHADOW_FULLSCAN);
  expect("X-T1 imu parser",
         parseLidarUpdatePolicy("imu_fullscan") ==
             LidarUpdatePolicy::IMU_FULLSCAN);
  bool unknown_failed = false;
  try {
    (void)parseLidarUpdatePolicy("unknown");
  } catch (const std::invalid_argument&) {
    unknown_failed = true;
  }
  expect("X-T1 unknown fails closed", unknown_failed);

  // X-T2: partial remains routed to the accepted S0 scheduler.
  expect("X-T2 partial scheduler unchanged",
         selectFullScanCadenceAction(LidarUpdatePolicy::PARTIAL,
                                     readyWithinScan()) ==
             CadenceAction::USE_PARTIAL_SCHEDULER);

  // X-T3/X-T4: shadow consumes/account cameras, but schedules no estimator
  // action at tc; the same raw scan is later selected exactly once.
  CadenceInputs shadow = readyWithinScan();
  expect("X-T3 shadow camera accounting only",
         selectFullScanCadenceAction(LidarUpdatePolicy::SHADOW_FULLSCAN,
                                     shadow) ==
             CadenceAction::ACCOUNT_CAMERA_ONLY);
  shadow.have_camera = false;
  expect("X-T4 shadow full scan once",
         selectFullScanCadenceAction(LidarUpdatePolicy::SHADOW_FULLSCAN,
                                     shadow) == CadenceAction::FULL_SCAN);

  // X-T5: imu_fullscan selects propagation only at an in-scan camera epoch.
  expect("X-T5 camera epoch is IMU only",
         selectFullScanCadenceAction(LidarUpdatePolicy::IMU_FULLSCAN,
                                     readyWithinScan()) ==
             CadenceAction::IMU_ONLY);

  // X-T6/X-T7: three camera epochs do not consume geometry; the full raw
  // scan is recorded once at scan end and all selected points are owned.
  FullScanOwnershipAudit ownership;
  ownership.recordInput(7, 100);
  for (double tc : {1.02, 1.05, 1.08}) {
    CadenceInputs in = readyWithinScan();
    in.camera_time = tc;
    expect("X-T6 point-preserving IMU segment",
           selectFullScanCadenceAction(LidarUpdatePolicy::IMU_FULLSCAN, in) ==
               CadenceAction::IMU_ONLY);
  }
  ownership.recordGeometryUse(7, 100);
  expect("X-T7 full raw scan used once", ownership.usedPoints() == 100 &&
                                              ownership.duplicatePoints() == 0 &&
                                              ownership.neverUsedPoints() == 0);

  // X-T8: the next scan has a distinct ownership identity. Reusing the old
  // scan is detected without corrupting the new scan's accounting.
  ownership.recordInput(8, 80);
  ownership.recordGeometryUse(8, 80);
  ownership.recordGeometryUse(7, 100);
  expect("X-T8 next-scan boundary ownership",
         ownership.usedPoints() == 180 && ownership.duplicatePoints() == 100 &&
             ownership.neverUsedPoints() == 0);

  // The scan-end event wins over an equal/later camera timestamp, preventing
  // propagation beyond a raw scan before its geometry update.
  CadenceInputs boundary = readyWithinScan();
  boundary.camera_time = boundary.full_scan_end_time;
  expect("X-T8 scan-end ordering",
         selectFullScanCadenceAction(LidarUpdatePolicy::IMU_FULLSCAN,
                                     boundary) == CadenceAction::FULL_SCAN);

  // X-T9: camera-disabled/B0 always selects the legacy full scan path.
  CadenceInputs b0 = readyWithinScan();
  b0.camera_epoch_enabled = false;
  expect("X-T9 B0 unaffected",
         selectFullScanCadenceAction(LidarUpdatePolicy::IMU_FULLSCAN, b0) ==
             CadenceAction::FULL_SCAN);

  std::printf("X-T1..X-T9: ALL PASS\n");
  return 0;
}
