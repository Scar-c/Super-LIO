cmake_minimum_required(VERSION 3.16)
project(super_lio)


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
find_package(Ceres REQUIRED)
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  basic
  pcl_ros
  rosbag
  std_msgs
  geometry_msgs
  sensor_msgs
  visualization_msgs
  nav_msgs
  message_generation
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
  CATKIN_DEPENDS geometry_msgs nav_msgs roscpp rospy rosbag std_msgs message_runtime
  DEPENDS EIGEN3 PCL
  INCLUDE_DIRS include
)

# LIO library source files
file(GLOB_RECURSE LIO_LIB_SRC "src/lio/*.cpp" "src/ros/*.cpp" "src/dec_lio/*.cpp"
  "src/dec_lio/D2ShadowAnalyzer.cpp")
list(APPEND LIO_LIB_SRC src/dec_lio/D3Solver.cpp)
list(APPEND LIO_LIB_SRC src/dec_lio/PoseFusion.cpp)

file(GLOB_RECURSE LIO_LIB_HEADERS "include/lio/*.hpp" "include/lio/*.h"
  "include/dec_lio/*.hpp" "include/dec_lio/*.h")

# library build
add_library(lio ${LIO_LIB_SRC} ${LIO_LIB_HEADERS})

# Target specific includes
target_include_directories(lio PUBLIC
  include
  3rdparty
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
  ${CERES_INCLUDE_DIRS}
)

# Linking libraries
target_link_libraries(lio
  ${EIGEN3_LIBRARIES}
  ${catkin_LIBRARIES}
  ${PCL_LIBRARIES}
  ${PYTHON_LIBRARIES}
  tbb 
  gflags 
  glog
  ${CERES_LIBRARIES}
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

# Algorithm-neutral offline transport adapter. It feeds the same ROSWrapper
# and SuperLIO library as super_lio_node; it contains no estimator logic.
add_executable(super_lio_offline_node
  src/apps/super_lio_offline_node.cpp
  offline/OfflineReader.cpp
)
target_include_directories(super_lio_offline_node PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(super_lio_offline_node
  lio
  ${catkin_LIBRARIES}
)

add_executable(dec_lio_dcreg_synthetic_test
  ../../tests/dec_lio/dcreg_synthetic_test.cpp
  src/dec_lio/DCRegAnalyzer.cpp
)
target_include_directories(dec_lio_dcreg_synthetic_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
)
add_test(NAME dec_lio_dcreg_synthetic_test COMMAND dec_lio_dcreg_synthetic_test)

add_executable(dec_lio_d2_shadow_synthetic_test
  ../../tests/dec_lio/d2_shadow_synthetic_test.cpp
  src/dec_lio/DCRegAnalyzer.cpp
  src/dec_lio/D2ShadowAnalyzer.cpp
)
target_include_directories(dec_lio_d2_shadow_synthetic_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
)
add_test(NAME dec_lio_d2_shadow_synthetic_test
  COMMAND dec_lio_d2_shadow_synthetic_test)

add_executable(dec_lio_consistency_synthetic_test
  ../../tests/dec_lio/consistency_synthetic_test.cpp
  src/dec_lio/DCRegAnalyzer.cpp
  src/dec_lio/D2ShadowAnalyzer.cpp
  src/dec_lio/ConsistencyAnalyzer.cpp
)
target_include_directories(dec_lio_consistency_synthetic_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
)
add_test(NAME dec_lio_consistency_synthetic_test
  COMMAND dec_lio_consistency_synthetic_test)

add_executable(dec_lio_weak_axis_synthetic_test
  ../../tests/dec_lio/weak_axis_synthetic_test.cpp
  src/dec_lio/WeakAxisAnalyzer.cpp
  src/dec_lio/DCRegAnalyzer.cpp
  src/dec_lio/D2ShadowAnalyzer.cpp
  src/dec_lio/ConsistencyAnalyzer.cpp
)
target_include_directories(dec_lio_weak_axis_synthetic_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
)
add_test(NAME dec_lio_weak_axis_synthetic_test
  COMMAND dec_lio_weak_axis_synthetic_test)

add_executable(dec_lio_point_selection_test
  ../../tests/dec_lio/point_selection_test.cpp)
target_include_directories(dec_lio_point_selection_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_test(NAME dec_lio_point_selection_test
  COMMAND dec_lio_point_selection_test)

add_executable(dec_lio_d3_solver_synthetic_test
  ../../tests/dec_lio/d3_solver_synthetic_test.cpp
  src/dec_lio/D3Solver.cpp
  src/dec_lio/DCRegAnalyzer.cpp)
target_include_directories(dec_lio_d3_solver_synthetic_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS})
add_test(NAME dec_lio_d3_solver_synthetic_test
         COMMAND dec_lio_d3_solver_synthetic_test)

add_executable(dec_lio_d3_solver_benchmark
  ../../tests/dec_lio/d3_solver_benchmark.cpp
  src/dec_lio/D3Solver.cpp
  src/dec_lio/DCRegAnalyzer.cpp)
target_include_directories(dec_lio_d3_solver_benchmark PRIVATE
  include ${EIGEN3_INCLUDE_DIRS})

add_executable(dec_lio_paired_attenuation_synthetic_test
  ../../tests/dec_lio/paired_attenuation_synthetic_test.cpp
  src/dec_lio/PairedAttenuation.cpp
  src/dec_lio/DCRegAnalyzer.cpp)
target_include_directories(dec_lio_paired_attenuation_synthetic_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS})
add_test(NAME dec_lio_paired_attenuation_synthetic_test
         COMMAND dec_lio_paired_attenuation_synthetic_test)

add_executable(dec_lio_prompt11_uniform_control_test
  ../../tests/dec_lio/prompt11_uniform_control_test.cpp
  src/dec_lio/PairedAttenuation.cpp
  src/dec_lio/DCRegAnalyzer.cpp)
target_include_directories(dec_lio_prompt11_uniform_control_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS})
add_test(NAME dec_lio_prompt11_uniform_control_test
         COMMAND dec_lio_prompt11_uniform_control_test)

add_executable(dec_lio_prompt14_shadow_test
  ../../tests/dec_lio/prompt14_shadow_test.cpp)
target_include_directories(dec_lio_prompt14_shadow_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS})
target_link_libraries(dec_lio_prompt14_shadow_test
  lio
  ${catkin_LIBRARIES})
add_test(NAME dec_lio_prompt14_shadow_test
         COMMAND dec_lio_prompt14_shadow_test)

add_executable(dec_lio_asymmetric_registration_test
  ../../tests/dec_lio/asymmetric_registration_test.cpp)
target_include_directories(dec_lio_asymmetric_registration_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS})
target_link_libraries(dec_lio_asymmetric_registration_test
  lio
  ${catkin_LIBRARIES})
add_test(NAME dec_lio_asymmetric_registration_test
  COMMAND dec_lio_asymmetric_registration_test)

add_executable(dec_lio_pose_fusion_test
  ../../tests/dec_lio/pose_fusion_test.cpp)
target_include_directories(dec_lio_pose_fusion_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS})
target_link_libraries(dec_lio_pose_fusion_test
  lio
  ${catkin_LIBRARIES})
add_test(NAME dec_lio_pose_fusion_test
  COMMAND dec_lio_pose_fusion_test)

add_executable(dec_lio_final_candidate_gate_test
  ../../tests/dec_lio/final_candidate_gate_test.cpp
  src/dec_lio/FinalCandidateGate.cpp)
target_include_directories(dec_lio_final_candidate_gate_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_test(NAME dec_lio_final_candidate_gate_test
  COMMAND dec_lio_final_candidate_gate_test)

add_executable(dec_lio_prompt17_gravity_residual_test
  ../../tests/dec_lio/prompt17_gravity_residual_test.cpp)
target_include_directories(dec_lio_prompt17_gravity_residual_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS})
target_link_libraries(dec_lio_prompt17_gravity_residual_test
  lio
  ${catkin_LIBRARIES})
add_test(NAME dec_lio_prompt17_gravity_residual_test
         COMMAND dec_lio_prompt17_gravity_residual_test)

add_executable(dec_lio_dcreg_core_solver_test
  ../../tests/dec_lio/dcreg_core_solver_test.cpp
  src/dec_lio/DCRegCoreSolver.cpp)
target_include_directories(dec_lio_dcreg_core_solver_test PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${EIGEN3_INCLUDE_DIRS})
add_test(NAME dec_lio_dcreg_core_solver_test
         COMMAND dec_lio_dcreg_core_solver_test)
