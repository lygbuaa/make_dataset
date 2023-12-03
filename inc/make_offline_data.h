#ifndef __MAKE_OFFLINE_NODE_H__
#define __MAKE_OFFLINE_NODE_H__

#include <dirent.h>
#include <libgen.h>
#include <thread>
#include "LogUtils.h"
#include "cv_param_loader.h"
#include "json_dataset.h"

enum class SvcIndex_t : int{
    VOID = -1,
    FRONT = 0,
    LEFT = 1,
    REAR = 2,
    RIGHT = 3,
    MAX = 4,
    ALL = 5,
};

typedef struct{
    double time;
    /** host timestamp for latency measure */
    double ptp_ts;
    cv::Mat img_front;
    cv::Mat img_left;
    cv::Mat img_rear;
    cv::Mat img_right;
    double x=0.0f;
    double y=0.0f;
    double z=0.0f;
    double pitch=0.0f;
    double roll=0.0f;
    double yaw=0.0f;
} SvcPairedImages_t;

class MakeOfflineData
{
private:
    std::unique_ptr<JsonDataset> ptr_json_dataset_;
    std::unique_ptr<std::thread> sync_thread_;
    std::string img_save_path_[4];
    bool working_;
    std::shared_ptr<CvParamLoader> param_loader_;
    std::deque<std::string> video_files_;

public:
    MakeOfflineData()
    {
    }
    ~MakeOfflineData()
    {}

    bool init(const std::string config_path, const std::string video_path)
    {
        param_loader_ = std::make_shared<CvParamLoader>(config_path);
        video_files_ = list_dir(video_path);
        ptr_json_dataset_ = std::unique_ptr<JsonDataset>(new JsonDataset(param_loader_->offline_data_path_));
        img_save_path_[(int)(SvcIndex_t::FRONT)] = param_loader_->offline_data_path_ + "/image_front/";
        img_save_path_[(int)(SvcIndex_t::LEFT)] = param_loader_->offline_data_path_ + "/image_left/";
        img_save_path_[(int)(SvcIndex_t::REAR)] = param_loader_->offline_data_path_ + "/image_rear/";
        img_save_path_[(int)(SvcIndex_t::RIGHT)] = param_loader_->offline_data_path_ + "/image_right/";
        for(int i=0; i<4; i++)
        {
            ViwoUtils::MakeDir(img_save_path_[i]);
        }

        sync_thread_ = std::unique_ptr<std::thread>(new std::thread(&MakeOfflineData::sync_process, this));
        return true;
    }

    void destroy()
    {
        // working_ = false;
        sync_thread_ -> join();
    }

    static uint64_t current_micros()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::time_point_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now()).time_since_epoch()).count();
    }

    static std::deque<std::string> list_dir(const std::string dirpath)
    {
        DIR* dp;
        std::deque<std::string> v_file_list;
        dp = opendir(dirpath.c_str());
        if (nullptr == dp)
        {
            RLOGE("read dirpath failed: %s", dirpath.c_str());
            return v_file_list;
        }

        struct dirent* entry;
        while((entry = readdir(dp)))
        {
            if(DT_DIR == entry->d_type){
                RLOGE("subdirectory ignored: %s", entry->d_name);
                continue;
            }else if(DT_REG == entry->d_type){
                std::string filepath = dirpath + "/" + entry->d_name;
                v_file_list.emplace_back(filepath);
            }
        }
        //sort into ascending order
        std::sort(v_file_list.begin(), v_file_list.end());
        RLOGI("find %ld files in path %s", v_file_list.size(), dirpath.c_str());
        for(auto& fp : v_file_list)
        {
            RLOGI("filepath: %s", fp.c_str());
        }

        return v_file_list;
    }

    cv::Mat load_nv12_image(const std::string& img_path)
    {
        FILE * pFile;
        long lSize;
        uint8_t * buffer;
        size_t result;

        pFile = fopen (img_path.c_str() , "rb");
        if (pFile==NULL) 
        {
            RLOGE ("File error"); 
            exit (1);
        }

        // obtain file size:
        fseek(pFile , 0 , SEEK_END);
        lSize = ftell (pFile);
        rewind (pFile);

        // allocate memory to contain the whole file:
        buffer = (uint8_t*) malloc (sizeof(uint8_t)*lSize);
        if (buffer == NULL) 
        {
            RLOGE ("Memory error"); 
            exit (2);
        }

        // copy the file into the buffer:
        result = fread (buffer,1,lSize,pFile);
        if (result != lSize) 
        {
            RLOGE ("Reading error"); 
            exit (3);
        }
        RLOGI("load img data %ld from %s", lSize, img_path.c_str());

        /* the whole file is now loaded in the memory buffer. */
        // std::vector<uint8_t> vimg(buffer, buffer+lSize);
        // cv::Mat(height * 3/2, Width, CV_8UC1, nv12Buffer);
        cv::Mat nv12_img((int)(1.5f*param_loader_->image_height_), param_loader_->image_width_, CV_8UC1, buffer);
        cv::Mat bgr_img;
        cv::cvtColor(nv12_img, bgr_img, cv::COLOR_YUV2BGR_NV12);

        // terminate
        fclose (pFile);
        free (buffer);
        return bgr_img;
    }


    void YUVToYUYV(const cv::Mat &yuv, std::vector<uint8_t> &yuyv)
    {
        int width = yuv.cols;
        int height = yuv.rows;

        // 分配YUYV422缓冲区大小（每个像素占2字节）
        yuyv.resize(width * height * 2);

        int yuyvIndex = 0;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x += 2)
            {
                // 获取相邻的两个像素
                cv::Vec3b yuvPixel1 = yuv.at<cv::Vec3b>(y, x);
                cv::Vec3b yuvPixel2 = yuv.at<cv::Vec3b>(y, x + 1);

                // 提取Y、U、V分量
                uint8_t y1 = yuvPixel1[0];
                uint8_t u = yuvPixel1[1];
                uint8_t y2 = yuvPixel2[0];
                uint8_t v = yuvPixel2[2];

                // 将Y、U、Y、V交错排列成YUYV形式
                // yuyv[yuyvIndex++] = y1;
                // yuyv[yuyvIndex++] = u;
                // yuyv[yuyvIndex++] = y2;
                // yuyv[yuyvIndex++] = v;

                // 将Y、U、Y、V交错排列成UYVY形式
                yuyv[yuyvIndex++] = u;
                yuyv[yuyvIndex++] = y1;
                yuyv[yuyvIndex++] = v;
                yuyv[yuyvIndex++] = y2;
            }
        }
    }

    bool save_uyvy_image(SvcIndex_t idx, const cv::Mat& rgbImage, double timestamp, const std::string& img_full_path)
    {
        static int counter[4] = {0};
        static uint64_t last_tick[4] = {0};

        cv::Mat yuvImage;
        yuvImage.create(rgbImage.rows, rgbImage.cols*2, CV_8UC2);
        cv::cvtColor(rgbImage, yuvImage, cv::COLOR_BGR2YUV);

        std::vector<uint8_t> yuyvImage;
        YUVToYUYV(yuvImage, yuyvImage);

        cv::Size newSize(rgbImage.cols, rgbImage.rows);
        cv::Mat tmp_img(yuvImage.rows, yuvImage.cols, CV_8UC2, yuyvImage.data());
        cv::resize(tmp_img, tmp_img, newSize);
        char* imageData = reinterpret_cast<char *>(tmp_img.data);

        int64_t timestampNs = (int64_t)(timestamp*1e9);
        size_t img_len = param_loader_->image_height_ * param_loader_->image_width_ * 2;
        
        /* attach timestamp in nano-seconds */
        if(imageData)
        {
            std::memcpy(imageData, &timestampNs, sizeof(timestampNs));
            RLOGD("idx[%d] attach timestampNs: %ld", idx, timestampNs);

            // struct timespec now;
            // /** use CLOCK_MONOTONIC instead of CLOCK_REALTIME, in case of UTC time sync */
            // clock_gettime(CLOCK_REALTIME, &now);
            // int64_t nowNs = now.tv_sec*1e9 + now.tv_nsec;
            // std::memcpy(imageData+sizeof(timestampNs), &nowNs, sizeof(nowNs));
            // RLOGD("chn[%d] attach nowNs: %ld", chn, nowNs);
        }

        FILE * pFile;
        
        pFile = fopen(img_full_path.c_str(), "w");
        if (pFile == NULL)
        {
            RLOGE("open file %s failed", img_full_path.c_str());
            fclose (pFile);
            return false;
        }

        fwrite(imageData, sizeof(char), img_len, pFile);
        fclose (pFile);

        counter[(int)idx] += 1;

        RLOGI("save image file %s done, [%d] counter: %d", img_full_path.c_str(), (int)idx, counter[(int)idx]);
        return true;
    }

    void sync_process()
    {
        RLOGI("svc camera sync process launch");
        ptr_json_dataset_ -> init_writer();

        double last_time = 0.0f;
        SvcPairedImages_t pis;
        working_ = true;
        while(working_)
        {
            /** read image from disk */
            if(video_files_.empty())
            {
                working_ = false;
                break;
            }

            std::string img_path = video_files_.front();
            video_files_.pop_front();
            if(img_path.size() < 4)
            {
                RLOGW("iamge file path invalid: %s", img_path.c_str());
                continue;
            }

            const char* fmt = param_loader_->image_format_.c_str();
            if(strncmp(fmt, "jpeg", 4)==0)
            {
                /** jpeg images */
                pis.img_front = cv::imread(img_path, cv::IMREAD_COLOR);
            }
            else if(strncmp(fmt, "nv12", 4)==0)
            {
                /** nv12 images */
                pis.img_front = load_nv12_image(img_path);
            }
            RLOGI("read img file %s, w=%d, h=%d, type=%d", img_path.c_str(), pis.img_front.cols, pis.img_front.rows, pis.img_front.type());

            if(true)
            {
                static uint64_t last_tick = ViwoUtils::CurrentMicros();
                static uint64_t frame_id = 0;
                pis.time = frame_id;

                /** save images */
                std::string img_file_name = std::to_string(pis.time) + ".uyvy";
                std::string img_front_path = img_save_path_[(int)(SvcIndex_t::FRONT)] + img_file_name;
                std::string img_left_path = img_save_path_[(int)(SvcIndex_t::LEFT)] + img_file_name;
                std::string img_rear_path = img_save_path_[(int)(SvcIndex_t::REAR)] + img_file_name;
                std::string img_right_path = img_save_path_[(int)(SvcIndex_t::RIGHT)] + img_file_name;

                ptr_json_dataset_ -> feed(
                    pis.time,
                    img_front_path,
                    img_left_path,
                    img_rear_path,
                    img_right_path,
                    pis.x,
                    pis.y,
                    pis.z,
                    pis.pitch,
                    pis.roll,
                    pis.yaw
                );

                save_uyvy_image(SvcIndex_t::FRONT, pis.img_front, pis.time, img_front_path);
                save_uyvy_image(SvcIndex_t::LEFT, pis.img_front, pis.time, img_left_path);
                save_uyvy_image(SvcIndex_t::REAR, pis.img_front, pis.time, img_rear_path);
                save_uyvy_image(SvcIndex_t::RIGHT, pis.img_front, pis.time, img_right_path);

                frame_id += 1;
                if(frame_id % 100 == 0)
                {
                    const uint64_t tick = ViwoUtils::CurrentMicros();
                    RLOGW("sync process frame_id: %ld, fps: %.2f", frame_id, 100.0*1e6/(tick-last_tick));
                    last_tick = tick;
                }
            }
            else
            {
                std::chrono::milliseconds dura(20);
                std::this_thread::sleep_for(dura);
            }
        }
        ptr_json_dataset_ -> close_writer();

        RLOGI("svc camera sync process exit");
    }


};

#endif