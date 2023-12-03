#!/bin/bash

function find_project_root_path() {
    # echo "@i@ --> find dir: ${0}"
    this_script_dir=$( dirname -- "$0"; )
    pwd_dir=$( pwd; )
    project_root_path=${pwd_dir}"/"${this_script_dir}"/"
    echo "${project_root_path}"
}

PRJ_ROOT_PATH=$( find_project_root_path )
echo "project_root_path: ${PRJ_ROOT_PATH}" 

DATA_PATh=${PRJ_ROOT_PATH}/data
BIN_PATH=${PRJ_ROOT_PATH}/build
CONFIG_PATH=${PRJ_ROOT_PATH}/config/video.yaml
# IMG_PATH=${PRJ_ROOT_PATH}/assets/img_front.png
IMG_PATH=${PRJ_ROOT_PATH}/assets/frames

${BIN_PATH}/make_offline_data_bin ${CONFIG_PATH} ${IMG_PATH}