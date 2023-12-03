#!/bin/bash

function find_project_root_path() {
    # echo "@i@ --> find dir: ${0}"
    this_script_dir=$( dirname -- "$0"; )
    pwd_dir=$( pwd; )
    project_root_path=${pwd_dir}"/"${this_script_dir}"/../"
    echo "${project_root_path}"
}

PRJ_ROOT_PATH=$( find_project_root_path )
echo "project_root_path: ${PRJ_ROOT_PATH}" 

IMG_W=3840
IMG_H=2160
PRJ_ASSETS_PATH=${PRJ_ROOT_PATH}/assets/
VIDEO_PATH=${PRJ_ASSETS_PATH}/sample_hevc_4k.mkv
OUTPUT_PATH=${PRJ_ASSETS_PATH}/frames
# IMG_PATH=${PRJ_ROOT_PATH}/assets/img_front.png
MAX_LOOP=10

# -r 0.5, extract 0.5 frame per second
ffmpeg -i ${VIDEO_PATH} -r 0.5 -s ${IMG_W}x${IMG_H} -f image2 -pix_fmt yuvj444p ${OUTPUT_PATH}/sample_%06d.jpeg