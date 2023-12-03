#ifndef __LOG_UTILS_H__
#define __LOG_UTILS_H__

/* set non-debug mode */
#ifndef NDEBUG
    #define NDEBUG 
#endif

/** set debug mode */
// #ifdef NDEBUG
//     #undef NDEBUG 
// #endif

#include <stdio.h>      /* printf */
#include <stdlib.h>     /* getenv */
#include <chrono>
#include <memory>

//* log with fprintf *//
#define LOGPF(format, ...) fprintf(stderr ,"[%s:%d] " format "\n", __FILE__, __LINE__, ##__VA_ARGS__)

/**
 * 0: glog
 * 1: ROS1
 * 2: ROS2
 * others: fprintf
 */
#define GLOBAL_LOGGING_BACKEND  3

#if GLOBAL_LOGGING_BACKEND == 2
    #include <rclcpp/logging.hpp>
    #include <rclcpp/rclcpp.hpp>
    //* this should be a rclcpp::Node derived class *//
    #define RLOGD(...) RCLCPP_DEBUG(this->get_logger(), ##__VA_ARGS__)
    #define RLOGI(...) RCLCPP_INFO(this->get_logger(), ##__VA_ARGS__)
    #define RLOGW(...) RCLCPP_WARN(this->get_logger(), ##__VA_ARGS__)
    #define RLOGE(...) RCLCPP_ERROR(this->get_logger(), ##__VA_ARGS__)
    #define RLOGF(...) RCLCPP_FATAL(this->get_logger(), ##__VA_ARGS__)

    void _run_logger_test_(){
        class Ros2LoggerTest : public rclcpp::Node
        {
        public:
            Ros2LoggerTest() : Node("Ros2LoggerTest")
            {
                RLOGD("this is debug");
                RLOGI("this is info");
                RLOGW("this is warn");
                RLOGE("this is error");
                RLOGF("this is fatal");
            }
        };

        rclcpp::Node::SharedPtr _node = std::make_shared<Ros2LoggerTest>();
    }
#elif GLOBAL_LOGGING_BACKEND == 1
    #include <ros/ros.h>
    #define RLOGD(...) ROS_DEBUG(__VA_ARGS__)
    #define RLOGI(...) ROS_INFO(__VA_ARGS__)
    #define RLOGW(...) ROS_WARN(__VA_ARGS__)
    #define RLOGE(...) ROS_ERROR(__VA_ARGS__)
    #define RLOGF(...) ROS_FATAL(__VA_ARGS__)

    void _run_ros1_logger_test_(){
        RLOGD("this is debug");
        RLOGI("this is info");
        RLOGW("this is warn");
        RLOGE("this is error");
        RLOGF("this is fatal");
    }

#elif GLOBAL_LOGGING_BACKEND == 0
    // #include <gflags/gflags.h>
    #include <glog/logging.h>
    #include <cstdio>
    #include <mutex>

    /** Yes I know this is terrible, anyway I don't want glog style */
    #define     G_LOG_BUF_SIZE      1024
    char g_log_buf_[G_LOG_BUF_SIZE] = {0};
    std::mutex g_log_mtx_;

    #define RLOGD(format, ...) do{std::unique_lock<std::mutex> lock(g_log_mtx_);memset(g_log_buf_, 0, G_LOG_BUF_SIZE);sprintf(g_log_buf_,format,##__VA_ARGS__); DLOG(INFO)<<g_log_buf_<<std::flush;}while(0)
    #define RLOGI(format, ...) do{std::unique_lock<std::mutex> lock(g_log_mtx_);memset(g_log_buf_, 0, G_LOG_BUF_SIZE);sprintf(g_log_buf_,format,##__VA_ARGS__); LOG(INFO)<<g_log_buf_<<std::flush;}while(0)
    #define RLOGW(format, ...) do{std::unique_lock<std::mutex> lock(g_log_mtx_);memset(g_log_buf_, 0, G_LOG_BUF_SIZE);sprintf(g_log_buf_,format,##__VA_ARGS__); LOG(WARNING)<<g_log_buf_<<std::flush;}while(0)
    #define RLOGE(format, ...) do{std::unique_lock<std::mutex> lock(g_log_mtx_);memset(g_log_buf_, 0, G_LOG_BUF_SIZE);sprintf(g_log_buf_,format,##__VA_ARGS__); LOG(ERROR)<<g_log_buf_<<std::flush;}while(0)
    #define RLOGF(format, ...) do{std::unique_lock<std::mutex> lock(g_log_mtx_);memset(g_log_buf_, 0, G_LOG_BUF_SIZE);sprintf(g_log_buf_,format,##__VA_ARGS__); LOG(FATAL)<<g_log_buf_<<std::flush;}while(0)

    void _run_glog_test_()
    {
        RLOGD("this is debug: %d", 123);
        RLOGI("this is info: %.3f", 123.0f);
        RLOGW("this is warn: %x-%x-%x", 10,11,12);
        RLOGE("this is error: %s", "0x123");
        // RLOGF("this is fatal: %f", 2e20);
    }
#else
    #define RLOGD(...) LOGPF(__VA_ARGS__)
    #define RLOGI(...) LOGPF(__VA_ARGS__)
    #define RLOGW(...) LOGPF(__VA_ARGS__)
    #define RLOGE(...) LOGPF(__VA_ARGS__)
    #define RLOGF(...) LOGPF(__VA_ARGS__)
#endif

/** profiling utility */
static inline __attribute__((always_inline)) uint64_t gfGetCurrentMicros() 
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::time_point_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now()).time_since_epoch()).count();
}

static inline std::shared_ptr<uint64_t> gfHangStopWatch(const char* func_name)
{
    uint64_t* pts_us = new uint64_t;
    *pts_us = gfGetCurrentMicros();
    RLOGD("stopwatch tick by %s", func_name);
    return std::shared_ptr<uint64_t>(pts_us, [func_name](uint64_t* ptr){
        uint64_t ts_us = gfGetCurrentMicros();
        RLOGI("stopwatch tock by %s, elapse: %ld us", func_name, (ts_us - *ptr));
        delete ptr;
    });
}

#define HANG_STOPWATCH() auto _ProfilingUtilsPtr_ = gfHangStopWatch(__FUNCTION__);

static inline void _print_ros_env_(){
    HANG_STOPWATCH()
    LOGPF("\n*** ROS_VERSION: %s ***\n", getenv("ROS_VERSION"));
    LOGPF("\n*** ROS_DISTRO: %s ***\n", getenv("ROS_DISTRO"));
    LOGPF("\n*** ROS_PYTHON_VERSION: %s ***\n", getenv("ROS_PYTHON_VERSION"));
}

#endif