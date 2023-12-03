#!/bin/bash

HEVC_IMAGE_PATH=/home/hugoliu/snap

function scan_dir() {
    echo "@i@ --> traverse dir: ${1}"
    for name in ${1}/*
    do
        if [ -d "$name" ]
        then
            scan_dir $name
        elif [ -f "$name" ]
        then
            transcode_image $name
        fi
    done 
}

function transcode_image() {
    filepath=${1}
    # get lower-case filename from the full path
    filename=`echo ${filepath##*/} | tr A-Z a-z`
    # echo "@i@ --> scan file: ${filename}"

    if [[ ${filename} == *.mkv ]]
    then
        ffmpeg -i ${filepath} -r 0.05 -f image2 -pix_fmt yuvj444p ${filepath}.jpg
        echo "@i@ --> transcode image ${filepath}"
    fi
}

scan_dir ${HEVC_IMAGE_PATH}
