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

# S-0 camera-epoch slice conservation tests (no ROS dependency)
add_executable(s0_slice_test
  src/common/tests/test_s0_slice.cpp
)
target_include_directories(s0_slice_test PUBLIC include)
target_link_libraries(s0_slice_test
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
