#pragma once

#include <opencv2/opencv.hpp>
#include <dirent.h>
#include <libgen.h>
#include <chrono>
#include <unistd.h>
#include "tinyjson.hpp"
#include "ViwoUtils.h"

class JsonDataset
{
public:
    std::string path_;
    std::string json_file_path_;
    std::ofstream json_ofs_;
    std::ifstream json_ifs_;

public:
    /* must set the path, where to save images and json file */
    JsonDataset(const std::string& path){
        path_ = path;
        json_file_path_ = path_ + "/dataset.json";
    }

    ~JsonDataset(){
        close_writer();
        close_reader();
    }

    void init_writer(){
        ViwoUtils::MakeDir(path_);
        json_ofs_.open(json_file_path_.c_str(), std::ofstream::out);
        LOGPF("open json file %s for writing", json_file_path_.c_str());
    }

    void init_reader(){
        json_ifs_.open(json_file_path_.c_str(), std::ifstream::in);
        LOGPF("open json file %s for reading", json_file_path_.c_str());
    }

    void close_writer(){
        if(json_ofs_.is_open()){
            json_ofs_.close();
            LOGPF("close json file %s for writing", json_file_path_.c_str());
        }
    }

    void close_reader(){
        if(json_ifs_.is_open()){
            json_ifs_.close();
            LOGPF("close json file %s for reading", json_file_path_.c_str());
        }
    }

    bool feed(double timestamp, 
            const std::string& img_front_path, 
            const std::string& img_left_path, 
            const std::string& img_rear_path, 
            const std::string& img_right_path,
            double x=0.0f,
            double y=0.0f,
            double z=0.0f,
            double pitch=0.0f,
            double roll=0.0f,
            double yaw=0.0f)
    {
        const std::string tstr = std::to_string(timestamp);
        tiny::TinyJson obj;
        obj["timestamp"].Set(timestamp);
        obj["img_front_path"].Set(img_front_path);
        obj["img_left_path"].Set(img_left_path);
        obj["img_rear_path"].Set(img_rear_path);
        obj["img_right_path"].Set(img_right_path);
        obj["x"].Set(x);
        obj["y"].Set(y);
        obj["z"].Set(z);
        obj["pitch"].Set(pitch);
        obj["roll"].Set(roll);
        obj["yaw"].Set(yaw);
        std::string str = obj.WriteJson();
        json_ofs_ << str << std::endl;
        LOGPF("write json string: %s\n", str.c_str());
        return true;
    }

    int load(double& timestamp, 
            std::string& img_front_path, 
            std::string& img_left_path, 
            std::string& img_rear_path, 
            std::string& img_right_path,
            double& x,
            double& y,
            double& z,
            double& pitch,
            double& roll,
            double& yaw)
    {
        std::string line;
        if(std::getline(json_ifs_, line))
        {
            /* parse json */
            tiny::TinyJson obj;
            obj.ReadJson(line);
            timestamp = obj.Get<double>("timestamp");
            img_front_path = obj.Get<std::string>("img_front_path");
            img_left_path = obj.Get<std::string>("img_left_path");
            img_right_path = obj.Get<std::string>("img_right_path");
            img_rear_path = obj.Get<std::string>("img_rear_path");
            x = obj.Get<double>("x");
            y = obj.Get<double>("y");
            z = obj.Get<double>("z");
            pitch = obj.Get<double>("pitch");
            roll = obj.Get<double>("roll");
            yaw = obj.Get<double>("yaw");
            LOGPF("read json string, timestamp: %f, img_front_path: %s, img_left_path: %s, img_right_path: %s, img_rear_path: %s, x: %f, y: %f, z: %f, pitch: %f, roll: %f, yaw: %f", \
                    timestamp, img_front_path.c_str(), img_left_path.c_str(), img_right_path.c_str(), img_rear_path.c_str(), x, y, z, pitch, roll, yaw);
            return 0;
        }

        if(json_ifs_.eof())
        {
            /** reach end of file */
            json_ifs_.clear();
            json_ifs_.seekg (0, json_ifs_.beg);
            LOGPF("seekg to file beigin");
            return 1;
        }
        else
        {
            return -1;
        }
    }

    // void test_writer(){
    //     init_writer();
    //     cv::Mat img = cv::Mat::ones(1080, 1920, CV_8UC3);
    //     double timestamp = 0.01f;
    //     while(timestamp < 10.0f){
    //         feed(timestamp, img, img, img, img);
    //         timestamp += 1.0f;
    //     }
    //     close_writer();
    // }

    // void test_reader(){
    //     init_reader();
    //     double timestamp = -1.0f;
    //     cv::Mat img_front, img_left, img_rear, img_right;
    //     while(load(timestamp, img_front, img_left, img_right, img_rear)){
    //         LOGPF("load dataset t: %f, h: %d, w: %d\n", timestamp, img_front.rows, img_rear.cols);
    //     }
    //     close_reader();
    // }

};
