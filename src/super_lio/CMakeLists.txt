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
  rosbag
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
  CATKIN_DEPENDS geometry_msgs nav_msgs roscpp rospy std_msgs message_runtime
  DEPENDS EIGEN3 PCL
  INCLUDE_DIRS include
)

# LIO library source files
file(GLOB_RECURSE LIO_LIB_SRC "src/lio/*.cpp" "src/ros/*.cpp")

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


# Offline baseline runner: reads a bag directly and feeds the unmodified
# production ROSWrapper/estimator in-process (no playback pacing).
add_executable(super_lio_offline_node
  src/apps/super_lio_offline_node.cpp
  offline/OfflineReader.cpp
)
target_include_directories(super_lio_offline_node PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  include
  3rdparty
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)
target_link_libraries(super_lio_offline_node
  lio
  ${catkin_LIBRARIES}
)


# Prob-LIO P1 tests (S1 point covariance; self-contained, no gtest needed).
add_executable(test_point_covariance
  ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/prob_lio/test_point_covariance.cpp
)
target_include_directories(test_point_covariance PRIVATE
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)
target_link_libraries(test_point_covariance ${catkin_LIBRARIES})
add_test(NAME prob_lio_point_covariance
         COMMAND test_point_covariance)

# Prob-LIO P2 tests (S3..S7 map covariance plumbing).
add_executable(test_map_covariance
  ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/prob_lio/test_map_covariance.cpp
)
target_include_directories(test_map_covariance PRIVATE
  include
  3rdparty
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)
target_link_libraries(test_map_covariance ${catkin_LIBRARIES})
add_test(NAME prob_lio_map_covariance
         COMMAND test_map_covariance)

# Prob-LIO P2 corrective tests (coupled pipeline policy / concurrency).
add_executable(test_pipeline_policy
  ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/prob_lio/test_pipeline_policy.cpp
)
target_include_directories(test_pipeline_policy PRIVATE
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)
target_link_libraries(test_pipeline_policy ${catkin_LIBRARIES})
add_test(NAME prob_lio_pipeline_policy
         COMMAND test_pipeline_policy)

# Prob-LIO P3 tests (S9 QR plane uncertainty).
add_executable(test_qr_plane_covariance
  ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/prob_lio/test_qr_plane_covariance.cpp
)
target_include_directories(test_qr_plane_covariance PRIVATE
  include
  3rdparty
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)
target_link_libraries(test_qr_plane_covariance ${catkin_LIBRARIES})
add_test(NAME prob_lio_qr_plane_covariance
         COMMAND test_qr_plane_covariance)

# Prob-LIO P4 tests (S11 probabilistic P2P weighting).
add_executable(test_p4_weight
  ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/prob_lio/test_p4_weight.cpp
)
target_include_directories(test_p4_weight PRIVATE
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)
target_link_libraries(test_p4_weight ${catkin_LIBRARIES})
add_test(NAME prob_lio_p4_weight
         COMMAND test_p4_weight)

# Prob-LIO P4-C1 tests (covariance validation modes light/full).
add_executable(test_validation_mode
  ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/prob_lio/test_validation_mode.cpp
)
target_include_directories(test_validation_mode PRIVATE
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)
target_link_libraries(test_validation_mode ${catkin_LIBRARIES})
add_test(NAME prob_lio_validation_mode
         COMMAND test_validation_mode)

# Prob-LIO P5 tests (S2/S10 probabilistic association).
add_executable(test_p5_association
  ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/prob_lio/test_p5_association.cpp
)
target_include_directories(test_p5_association PRIVATE
  include
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)
target_link_libraries(test_p5_association ${catkin_LIBRARIES})
add_test(NAME prob_lio_p5_association
         COMMAND test_p5_association)

# Prob-LIO P5 corrective tests (real seam, shadow matrix, identity, isolation).
add_executable(test_p5_seam_shadow
  ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/prob_lio/test_p5_seam_shadow.cpp
)
target_include_directories(test_p5_seam_shadow PRIVATE
  include
  3rdparty
  ${EIGEN3_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)
target_link_libraries(test_p5_seam_shadow ${catkin_LIBRARIES})
add_test(NAME prob_lio_p5_seam_shadow
         COMMAND test_p5_seam_shadow)

