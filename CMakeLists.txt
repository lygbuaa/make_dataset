cmake_minimum_required(VERSION 3.10.0)
project(make_dataset_project)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -Wall -g -fPIC -rdynamic -export-dynamic")

# using opencv-4.2 with opencl support, but not cuda
find_package(OpenCV REQUIRED)
# using opencv-4.8 with cuda support, but not opencl
# include(/home/hugoliu/github/opencv_4_8/release/bin/lib/cmake/opencv4/OpenCVConfig.cmake)
message(STATUS "OpenCV_VERSION: ${OpenCV_VERSION}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_INSTALL_PATH: ${OpenCV_INSTALL_PATH}")

# find_package (glog REQUIRED)

include_directories(
  ${OpenCV_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/inc/
  ${PROJECT_SOURCE_DIR}/common/
)

EXECUTE_PROCESS(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE)
MESSAGE(STATUS "cpu architecture: ${ARCHITECTURE}")


link_directories(
              ${OpenCV_INSTALL_PATH}
              ${PROJECT_SOURCE_DIR}/lib
)

add_executable(make_offline_data_bin 
               src/make_offline_data.cc
)

target_link_libraries(make_offline_data_bin
    ${OpenCV_LIBS}
    -ldl
    -lrt
    -pthread
)

install (TARGETS make_offline_data_bin DESTINATION inject_export)
# install (FILES assets DESTINATION inject_export/assets)
install (FILES make_data.sh DESTINATION inject_export)
