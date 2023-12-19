#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import time, threading, socket, struct
import traceback, logging
import plogging
import numpy as np
import torch, torchvision
import kornia
import cv2

global g_logger

# read rgb image into torch.tensor, resize_to=(1920, 1080)
def read_image(img_path, peek=False, resize_to=None):
    img_rgb = torchvision.io.read_image(img_path, torchvision.io.ImageReadMode.UNCHANGED)
    if resize_to:
        transform = torchvision.transforms.Resize(size = resize_to)
        img_rgb = transform(img_rgb)
    g_logger.info("img_tensor shape: {}, type: {}".format(img_rgb.shape, img_rgb.dtype))
    if peek:
        try:
            img_np = img_rgb.numpy().transpose(1, 2, 0)
            cv2.imshow("peek_image", img_np)
            cv2.waitKey(2000)
        except Exception as e:
            g_logger.error(traceback.format_exc())
    return img_rgb

# convert rgb tensor into bayer uint8 tensor
def rgb_to_bayer(img_tensor_rgb):
    img_bayer = kornia.color.rgb_to_raw(img_tensor_rgb, kornia.color.CFA.BG)
    g_logger.info("img_bayer shape: {}, type: {}, header: {}".format(img_bayer.shape, img_bayer.dtype, img_bayer[0, 0, 0]))
    return img_bayer

# convert bayer uint8 tensor to uint12 tensor
def bayer_to_raw12(img_raw8, save_to_path="./raw12.bin", endian="big"):
    c, h, w = img_raw8.shape
    g_logger.info("img_raw8 c=%d, h=%d, w=%d", c, h, w)
    w12 = int(w*1.5)
    img_raw12 = np.zeros(shape=(h, w12), dtype=np.uint8)
    g_logger.info("img_raw12 shape: {}".format(img_raw12.shape))
    assert np.mod(w, 2) == 0
    assert c == 1

    w_steps = w//2
    for j in range(h):
        for i in range(w_steps):
            # g_logger.info("i=%d, j=%d", i, j)
            # raw12_byte0 = img_raw12[j, i*3]
            # raw12_byte1 = img_raw12[j, i*3+1]
            # raw12_byte2 = img_raw12[j, i*3+2]

            raw8_byte0 = img_raw8[0, j, i*2]
            raw8_byte1 = img_raw8[0, j, i*2+1]
            g_logger.debug("raw8_byte0: {}, raw8_byte1: {}".format(raw8_byte0, raw8_byte1))

            # img_raw12[j, i*3]   = np.mod(np.left_shift(raw8_byte0,4), 256)
            if endian == "big":
                ### https://wiki.apertus.org/index.php/RAW12
                img_raw12[j, i*3]   = raw8_byte0
                img_raw12[j, i*3+1] = np.bitwise_and((np.right_shift(raw8_byte1,4)), 0x0f)
                img_raw12[j, i*3+2] = np.bitwise_and((np.left_shift(raw8_byte1,4)), 0xf0)
            elif endian == "little":
                img_raw12[j, i*3]   = np.bitwise_and((np.left_shift(raw8_byte0,4)), 0xf0)
                img_raw12[j, i*3+1] = np.bitwise_and(raw8_byte0, 0xf0)
                img_raw12[j, i*3+2] = raw8_byte1
            g_logger.debug("raw12_byte0: {}, raw12_byte1: {}: raw12_byte2: {}".format(img_raw12[j, i*3], img_raw12[j, i*3+1], img_raw12[j, i*3+2]))
    if len(save_to_path) > 3:
        with open(save_to_path, "wb") as file:
            file.write(img_raw12.tobytes())
    return img_raw12

if __name__ == "__main__":
    plogging.init_logger(log_dir="./", file_name="rgb_to_raw12", level=logging.INFO)
    g_logger = plogging.get_logger()
    img_rgb = read_image(img_path="/home/hugoliu/github/make_dataset/assets/frames/sample_000001.jpeg", peek=False, resize_to=(1080, 1920))
    # img_rgb = torch.randint(low=0, high=255, size=(3, 4, 4), dtype=torch.uint8)
    img_bayer = rgb_to_bayer(img_rgb)
    img_raw12 = bayer_to_raw12(img_bayer, endian="big")