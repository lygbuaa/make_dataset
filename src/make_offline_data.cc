#include "make_offline_data.h"
#include "SignalBase.h"

int main(int argc, char **argv)
{
    for(int i = 0; i < argc; i++)
    {
        LOGPF("argv[%d] = %s\n", i, argv[i]);
    }

    SignalBase::CatchSignal();

    if(argc < 3)
    {
        LOGPF("make_offline_data_bin  config.yaml  images_path");
        return -1;
    }

    auto g_make_offline_node = std::make_shared<MakeOfflineData>();

    std::string config_file_path = argv[1];
    std::string video_file_path = argv[2];
    RLOGI("config_file_path: %s, video_file_path: %s", argv[1], argv[2]);

    g_make_offline_node -> init(config_file_path, video_file_path);
    // g_pcie_bridge_node -> test_video_inject();

    g_make_offline_node -> destroy();

    return 0;
}
