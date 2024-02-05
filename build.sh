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

BUILD_TYPE=Release   #Debug or Release
ARCH_TYPE=x86_64    #x86_64 or aarch64
CUR_DIR_PATH=${PRJ_ROOT_PATH}
PROJECT_NAME=pcie_aumo_node

cd ${CUR_DIR_PATH}
mkdir -p ./build
rm -rf ./build/*
cd ./build

BUILD_CMD_LINE="-DCMAKE_INSTALL_PREFIX=${PWD}/install "
BUILD_CMD_LINE="${BUILD_CMD_LINE} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} "
BUILD_CMD_LINE="${BUILD_CMD_LINE} ${CUR_DIR_PATH} "

cmake ${BUILD_CMD_LINE}
make -j8
make install
cd ${CUR_DIR_PATH}