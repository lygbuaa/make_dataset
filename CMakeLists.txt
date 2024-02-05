cmake_minimum_required(VERSION 3.10.0)
project(make_dataset_project)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -g -fPIC -rdynamic -export-dynamic")

add_compile_options(-msse4)

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
  ${PROJECT_SOURCE_DIR}/rgb2yuv/
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

add_executable(test_rgb2yuyv 
               rgb2yuv/test_rgb2yuyv.cc
               rgb2yuv/cpu.cc 
               rgb2yuv/rgb2yuyv.cc
)
target_link_libraries(test_rgb2yuyv
    -ldl
    -lrt
    -pthread
)

install (TARGETS make_offline_data_bin DESTINATION install_export)
install (TARGETS test_rgb2yuyv DESTINATION install_export)
# install (FILES assets DESTINATION install_export/assets)
install (FILES make_data.sh DESTINATION install_export)
