#ifndef __VIWO_UTILS_H__
#define __VIWO_UTILS_H__

#include <chrono>
#include <memory>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include "LogUtils.h"

// #define HANG_STOPWATCH() auto _ViwoUtilsPtr_ = ViwoUtils::HangStopWatch(__FUNCTION__);

class ViwoUtils
{
public:
    ViwoUtils() {}
    ~ViwoUtils() {}

    static inline __attribute__((always_inline)) uint64_t CurrentMicros() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::time_point_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now()).time_since_epoch()).count();
    }

    /*print function enter/exit/time_interval*/
    static std::shared_ptr<uint64_t> HangStopWatch(const char* func_name){
        uint64_t* pts_us = new uint64_t;
        *pts_us = CurrentMicros();
        // LOGPF("stop watch trigger by %s (epoch %ld us)", func_name, *pts_us);
        return std::shared_ptr<uint64_t>(pts_us, [func_name](uint64_t* ptr){
            uint64_t ts_us = CurrentMicros();
            // LOG(INFO) << "stop watch end: " << ts_us << " us.";
            LOGPF("stop watch end by %s (elapse = %ld us)", func_name, (ts_us - *ptr));
            delete ptr;
        });
    }

    /* between 0~k */
    static double RandDouble(double k = 1.0f){
        return k * rand() / double(RAND_MAX);
    }

    /* unit: m/s */
    static double WheelVelBias(double k = 0.01f){
        /* this noise should keep positive or negative */
        return fabs(RandDouble(k));
    }

    /* small mat debug print */
    static std::string CvMat2Str(const cv::Mat& mat){
        cv::Mat oneRow = mat.reshape(0, 1);
        std::ostringstream os;
        os << oneRow;
        return os.str();
    }

#if 0
    static double Ros2HeaderToSec(std_msgs::msg::Header& header){
        double t_sec = header.stamp.sec + header.stamp.nanosec * 1e-9;
        return t_sec;
    }

    static rclcpp::Time SecToRos2Stamp(const double t){
        int64_t sec = int64(t);
        int64_t nanosec = int64_t((t - sec)*1e9);
        return rclcpp::Time(sec, nanosec);
    }

    static rclcpp::Duration SecToRos2Duration(const double t){
        int64_t sec = int64(t);
        int64_t nanosec = int64_t((t - sec)*1e9);
        return rclcpp::Duration(sec, nanosec);
    }
#endif

    static double GetHostTimeSec(){
        struct timespec now;
        /** use CLOCK_MONOTONIC instead of CLOCK_REALTIME, in case of UTC time sync */
        clock_gettime(CLOCK_REALTIME, &now);
        double nowSec = now.tv_sec + now.tv_nsec*1e-9;
        return nowSec;
    }

    /** calc latency from host time and image timestamp, rquires ptp4l */
    static double CalcImageLatencySec(const uint8_t* uyvy_buffer)
    {
        int64_t img_ts_nsec;
        memcpy(&img_ts_nsec, uyvy_buffer+sizeof(int64_t), sizeof(int64_t)); 
        double img_ts_sec = img_ts_nsec * 1e-9;
        return GetHostTimeSec() - img_ts_sec;
    }

    static void MakeDir(std::string path){
        std::string cmd = "mkdir -p " + path;
        int32_t exit_code = system(cmd.c_str());
        LOGPF("ShellCall (%s) exit code: %d", cmd.c_str(), exit_code);
    }

};

#endif