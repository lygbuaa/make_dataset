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

cd ${PRJ_ROOT_PATH}
mkdir -p build
rm -rf build/*
cd build
cmake ..
make