#pragma once

#include <opencv2/opencv.hpp>

class CvParamLoader
{
public:
    static constexpr double PI_ = 3.141592741;
    static constexpr double DEG2RAD = 180.0 / PI_;

    std::string offline_data_path_;

    int image_width_;
    int image_height_;
    int image_fps_;
    std::string image_format_;

    int image_front_chn_;
    int image_left_chn_;
    int image_rear_chn_;
    int image_right_chn_;

private:
    std::string yaml_path_;
    cv::FileStorage fs_;

public:
    CvParamLoader(std::string yaml_path)
    {
        yaml_path_ = yaml_path;
        fs_.open(yaml_path, cv::FileStorage::READ);
        if(!fs_.isOpened()){
            RLOGE("open config file failed: %s\n", yaml_path.c_str());
        }
        assert(fs_.isOpened());
        load_params();
    }

    ~CvParamLoader(){
        if(fs_.isOpened()){
            fs_.release();
        }
    }

#if 0
    /* this is project source code path */
    std::string get_package_src_path(const std::string& pkg_name = "ro2_demo_node"){
        std::string pkg_path = ament_index_cpp::get_package_share_directory(pkg_name);
        // return pkg_path + "/../";
        return pkg_path + "/../../../../ros-bridge/";
    }

    /* this is project source code path */
    std::string get_package_share_path(const std::string& pkg_name = "ro2_demo_node"){
        std::string pkg_share_path = ament_index_cpp::get_package_share_directory(pkg_name);
        return pkg_share_path + "/";
    }
#endif

    void load_params(){
        fs_["offline_data_path"] >> offline_data_path_;
        fs_["image_front_chn"] >> image_front_chn_;
        fs_["image_left_chn"] >> image_left_chn_;
        fs_["image_rear_chn"] >> image_rear_chn_;
        fs_["image_right_chn"] >> image_right_chn_;

        fs_["image_width"] >> image_width_;
        fs_["image_height"] >> image_height_;
        fs_["image_fps"] >> image_fps_;
        fs_["image_format"] >> image_format_;
        RLOGI("image_width: %d, image_height: %d, image_fps: %d, format: %s", image_width_, image_height_, image_fps_, image_format_.c_str());
    }

};