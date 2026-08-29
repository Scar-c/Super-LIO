cmake_minimum_required(VERSION 3.16)
project(super_lio)
execute_process(COMMAND git -C ${CMAKE_SOURCE_DIR} rev-parse --short HEAD
                OUTPUT_VARIABLE GIT_SHA OUTPUT_STRIP_TRAILING_WHITESPACE)


#DEBUG CONFIGURATION
# set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -pthread -fsanitize=address")

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -pthread -fexceptions -flto=auto")
add_definitions(-DROOT=\"${CMAKE_CURRENT_SOURCE_DIR}/\")

# Find packages
find_package(Eigen3 REQUIRED)
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  basic
  pcl_ros
  std_msgs
  geometry_msgs
  sensor_msgs
  visualization_msgs
  nav_msgs
  message_generation
  rosbag
)
find_package(PythonLibs REQUIRED)

add_message_files(
  FILES
  CloudPose.msg
  CloudPose2.msg
)

generate_messages(
  DEPENDENCIES
  std_msgs
  sensor_msgs
  geometry_msgs
)

# Include directories for libraries
include_directories(
  ${catkin_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
  include
  3rdparty
)

# Package setup for catkin
catkin_package(
  CATKIN_DEPENDS geometry_msgs nav_msgs roscpp rospy std_msgs message_runtime rosbag
  DEPENDS EIGEN3 PCL
  INCLUDE_DIRS include
)

# LIO library source files
file(GLOB_RECURSE LIO_LIB_SRC "src/lio/*.cpp" "src/ros/*.cpp" "src/offline/*.cpp" "src/instrumentation/*.cpp")
list(FILTER LIO_LIB_SRC EXCLUDE REGEX ".*/tests/.*")
file(GLOB LIO_LIB_SRC_CAM "src/camera/*.cpp")
list(APPEND LIO_LIB_SRC ${LIO_LIB_SRC_CAM})

file(GLOB_RECURSE LIO_LIB_HEADERS "include/lio/*.hpp" "include/lio/*.h")

# library build
add_library(lio ${LIO_LIB_SRC} ${LIO_LIB_HEADERS})

# Target specific includes
target_include_directories(lio PUBLIC
  include
  3rdparty
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
)

# Linking libraries
target_link_libraries(lio
  ${EIGEN3_LIBRARIES}
  ${catkin_LIBRARIES}
  jpeg
  ${PCL_LIBRARIES}
  ${PYTHON_LIBRARIES}
  tbb 
  gflags 
  glog
)


add_executable(super_lio_node src/apps/super_lio_node.cpp)
target_link_libraries(super_lio_node
  lio
  ${catkin_LIBRARIES}
)


add_executable(relocation_node src/apps/relocation_node.cpp)
target_link_libraries(relocation_node
  lio
  ${catkin_LIBRARIES}
)

# Offline bag-processing backend (direct rosbag reader, same estimator core)
add_executable(super_lio_offline_node src/apps/super_lio_offline_node.cpp)
target_link_libraries(super_lio_offline_node
  lio
  ${catkin_LIBRARIES}
)

# Round 11AB effective post-resolve configuration evidence tests
add_executable(effective_config_test
  src/instrumentation/tests/test_effective_config.cpp
)
target_include_directories(effective_config_test PUBLIC include)
target_link_libraries(effective_config_test
  lio
  ${catkin_LIBRARIES}
)

# Round 11P V-4R0 outlier gate tests
add_executable(v4r0_test
  src/geometry/tests/test_v4r0.cpp
)
target_include_directories(v4r0_test PUBLIC include)
target_link_libraries(v4r0_test
  Eigen3::Eigen
)

# Round 11N VI-0 weighted information tests
add_executable(vi0_test
  src/geometry/tests/test_vi0.cpp
)
target_include_directories(vi0_test PUBLIC include)
target_link_libraries(vi0_test
  Eigen3::Eigen
)

# Round 11K HB-0 production H/b audit helper tests
add_executable(hb0_audit_test
  src/geometry/tests/test_hb0_audit.cpp
)
target_include_directories(hb0_audit_test PUBLIC include)
target_link_libraries(hb0_audit_test
  Eigen3::Eigen
)

# Round 11J condition-aware Gate M classifier tests
add_executable(gate_classifier_test
  src/geometry/tests/test_gate_classifier.cpp
)
target_include_directories(gate_classifier_test PUBLIC include)
target_link_libraries(gate_classifier_test
  Eigen3::Eigen
)

# Round 11G extrinsic/rotation convention parity tests
add_executable(extrinsic_parity_test
  src/geometry/tests/test_extrinsic_parity.cpp
)
target_include_directories(extrinsic_parity_test PUBLIC include)
target_link_libraries(extrinsic_parity_test
  Eigen3::Eigen
)

# FD harness pure-logic tests (bundle smoothness + quota state)
add_executable(fd_harness_test
  src/geometry/tests/test_fd_harness.cpp
)
target_include_directories(fd_harness_test PUBLIC include)
target_link_libraries(fd_harness_test
  Eigen3::Eigen
)

# Exact bilinear sample+gradient primitive tests
add_executable(bilinear_test
  src/geometry/tests/test_bilinear.cpp
)
target_include_directories(bilinear_test PUBLIC include)
target_link_libraries(bilinear_test
  Eigen3::Eigen
)

# V-2 plane-aware warp Jacobian synthetic tests (T_cb-aware)
add_executable(v2_jacobian_test
  src/lio/tests/test_v2_jacobian.cpp
)
target_include_directories(v2_jacobian_test PUBLIC include)
target_compile_definitions(v2_jacobian_test PRIVATE
  GIT_SHA="\"${GIT_SHA}\"")
target_link_libraries(v2_jacobian_test
  Eigen3::Eigen
  ${catkin_LIBRARIES}
  gflags
  glog
)

# V-4C synthetic sequential-update tests (no ROS dependency)
add_executable(v4c_test
  src/lio/tests/test_v4c.cpp
  src/lio/ESKF.cpp
  src/lio/params.cpp
)
target_include_directories(v4c_test PUBLIC include)
target_link_libraries(v4c_test
  Eigen3::Eigen
  ${PCL_LIBRARIES}
  ${catkin_LIBRARIES}
  gflags
  glog
)

# Round 11R multi-bag View merge test
add_executable(multibag_test
  src/offline/tests/test_multibag.cpp
)
target_include_directories(multibag_test PUBLIC include)
target_link_libraries(multibag_test
  ${catkin_LIBRARIES}
)

# S-1 sequential-prior ESKF tests (no ROS dependency)
add_executable(s1_prior_test
  src/lio/tests/test_s1_prior.cpp
  src/lio/ESKF.cpp
  src/lio/params.cpp
)
target_include_directories(s1_prior_test PUBLIC include)
target_link_libraries(s1_prior_test
  Eigen3::Eigen
  ${PCL_LIBRARIES}
  ${catkin_LIBRARIES}
  gflags
  glog
)

# Round 11V S0 repeated pending-tail corrective tests
add_executable(s0_corrective_test
  src/common/tests/test_s0_corrective.cpp
)
target_include_directories(s0_corrective_test PUBLIC include)
target_link_libraries(s0_corrective_test
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

# S-0 camera-epoch slice conservation tests (no ROS dependency)
add_executable(s0_slice_test
  src/common/tests/test_s0_slice.cpp
)
target_include_directories(s0_slice_test PUBLIC include)
target_link_libraries(s0_slice_test
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

# Round 11Z mixed-rate camera temporal sampler tests.
add_executable(round11z_sampler_test
  src/common/tests/test_round11z_sampler.cpp
)
target_include_directories(round11z_sampler_test PUBLIC include)
target_link_libraries(round11z_sampler_test
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

# Prompt84 rebuild camera-epoch sequential seam checks.
add_executable(prompt84_rebuild_seam_test
  src/common/tests/test_prompt84_rebuild_seam.cpp
  src/lio/ESKF.cpp
)
target_include_directories(prompt84_rebuild_seam_test PUBLIC include)
target_link_libraries(prompt84_rebuild_seam_test
  ${catkin_LIBRARIES}
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

# Round 11X camera-epoch/full-scan cadence policy and ownership tests.
add_executable(round11x_cadence_test
  src/common/tests/test_round11x_cadence.cpp
)

# Round 11Y IMU segmentation attribution tests (ESKF propagation semigroup).
add_executable(y_segmentation_test
  src/common/tests/test_y_segmentation.cpp
  src/lio/ESKF.cpp
)
target_include_directories(y_segmentation_test PUBLIC include
  ${catkin_INCLUDE_DIRS})
target_link_libraries(y_segmentation_test
  ${catkin_LIBRARIES}
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)
target_include_directories(round11x_cadence_test PUBLIC include)
target_link_libraries(round11x_cadence_test
  Eigen3::Eigen
  ${PCL_LIBRARIES}
)

# G-1VR coordinate-origin sync tests (no ROS dependency)
add_executable(g1vr_surfel_sync_test
  src/geometry/tests/test_surfel_sync.cpp
)
target_include_directories(g1vr_surfel_sync_test PUBLIC include)
target_link_libraries(g1vr_surfel_sync_test
  Eigen3::Eigen
)

# G-1V Scheme-B shadow pure-logic tests (no ROS dependency)
add_executable(g1v_schemeb_test
  src/geometry/tests/test_schemeb.cpp
)
target_include_directories(g1v_schemeb_test PUBLIC include)
target_link_libraries(g1v_schemeb_test
  Eigen3::Eigen
)

# TB-0 instrumentation pure-logic tests (no ROS dependency)
file(GLOB INSTR_SRC "src/instrumentation/*.cpp")
list(FILTER INSTR_SRC EXCLUDE REGEX ".*/tests/.*")
add_executable(tb0_instrumentation_test
  src/instrumentation/tests/test_instrumentation.cpp
  ${INSTR_SRC}
)
target_include_directories(tb0_instrumentation_test PUBLIC include)
target_link_libraries(tb0_instrumentation_test
  ${EIGEN3_LIBRARIES}
)

# TB-1 camera input pure-logic tests (no ROS dependency)
file(GLOB CAMERA_TEST_SRC "src/camera/*.cpp")
list(FILTER CAMERA_TEST_SRC EXCLUDE REGEX ".*/tests/.*")
add_executable(tb1_camera_test
  src/camera/tests/test_camera.cpp
  ${CAMERA_TEST_SRC}
)
target_include_directories(tb1_camera_test PUBLIC include)
target_link_libraries(tb1_camera_test
  ${EIGEN3_LIBRARIES}
)

# G-0 sidecar pure-logic tests (Welford oracle + eviction seam)
add_executable(g0_sidecar_test
  src/geometry/tests/test_sidecar.cpp
)
target_include_directories(g0_sidecar_test PUBLIC
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)
target_link_libraries(g0_sidecar_test
  ${EIGEN3_LIBRARIES}
  ${PCL_LIBRARIES}
)
