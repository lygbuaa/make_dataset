#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import time, threading, socket, struct
import traceback, logging
import plogging
import numpy as np
from numba import njit, prange
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
    return img_bayer.numpy()


def bayer_to_raw12(img_raw8, save_to_path="./raw12.bin", endian="big"):
    c, h, w = img_raw8.shape
    g_logger.info("img_raw8 c=%d, h=%d, w=%d", c, h, w)
    # img_raw12 = np.zeros(shape=(h, w12), dtype=np.uint8)
    assert np.mod(w, 2) == 0
    assert c == 1

    img_raw12 = bayer_to_raw12_kernel(h, w, img_raw8, endian)
    g_logger.info("img_raw12 shape: {}".format(img_raw12.shape))

    if len(save_to_path) > 3:
        with open(save_to_path, "wb") as file:
            file.write(img_raw12.tobytes())
    return img_raw12

# convert bayer uint8 numppy to uint12 numpy
@njit(parallel=True)
def bayer_to_raw12_kernel(h, w, img_raw8, endian="big"):
    w12 = int(w*1.5)
    w_steps = w//2
    img_raw12 = np.zeros(shape=(h, w12), dtype=np.uint8)
    counter = 0
    for j in prange(h):
        for i in prange(w_steps):
            raw8_byte0 = img_raw8[0, j, i*2]
            raw8_byte1 = img_raw8[0, j, i*2+1]
            # g_logger.debug("raw8_byte0: {}, raw8_byte1: {}".format(raw8_byte0, raw8_byte1))

            # img_raw12[j, i*3]   = np.mod(np.left_shift(raw8_byte0,4), 256)
            if endian == "big":
                ### https://wiki.apertus.org/index.php/RAW12
                img_raw12[j, i*3]   = raw8_byte0
                img_raw12[j, i*3+1] = np.bitwise_and((np.right_shift(raw8_byte1, 4)), 0x0f)
                img_raw12[j, i*3+2] = np.bitwise_and((np.left_shift(raw8_byte1, 4)), 0xf0)
                # img_raw12[j, i*3] = counter%256
                # img_raw12[j, i*3+1] = (counter+1)%256
                # img_raw12[j, i*3+2] = (counter+2)%256
                # counter = counter + 3
            elif endian == "little":
                ### https://patches.linaro.org/project/lkml/patch/1502199018-28250-2-git-send-email-todor.tomov@linaro.org/
                # img_raw12[j, i*3]   = raw8_byte0
                # img_raw12[j, i*3+1] = 0x0
                # img_raw12[j, i*3+2] = raw8_byte1
                img_raw12[j, i*3]   = 0x0
                img_raw12[j, i*3]   = np.bitwise_or(np.left_shift(np.bitwise_and(raw8_byte0, 0x01), 3), img_raw12[j, i*3])
                img_raw12[j, i*3]   = np.bitwise_or(np.left_shift(np.bitwise_and(raw8_byte0, 0x02), 1), img_raw12[j, i*3])
                img_raw12[j, i*3]   = np.bitwise_or(np.right_shift(np.bitwise_and(raw8_byte0, 0x04), 1), img_raw12[j, i*3])
                img_raw12[j, i*3]   = np.bitwise_or(np.right_shift(np.bitwise_and(raw8_byte0, 0x08), 3), img_raw12[j, i*3])

                img_raw12[j, i*3+1] = 0x0
                img_raw12[j, i*3+1] = np.bitwise_or(np.left_shift(np.bitwise_and(raw8_byte0, 0x10), 3), img_raw12[j, i*3+1])
                img_raw12[j, i*3+1] = np.bitwise_or(np.left_shift(np.bitwise_and(raw8_byte0, 0x20), 1), img_raw12[j, i*3+1])
                img_raw12[j, i*3+1] = np.bitwise_or(np.right_shift(np.bitwise_and(raw8_byte0, 0x40), 1), img_raw12[j, i*3+1])
                img_raw12[j, i*3+1] = np.bitwise_or(np.right_shift(np.bitwise_and(raw8_byte0, 0x80), 3), img_raw12[j, i*3+1])

                img_raw12[j, i*3+2] = 0x0
                img_raw12[j, i*3+2] = np.bitwise_or(np.left_shift(np.bitwise_and(raw8_byte1, 0x01), 7), img_raw12[j, i*3+2])
                img_raw12[j, i*3+2] = np.bitwise_or(np.left_shift(np.bitwise_and(raw8_byte1, 0x02), 5), img_raw12[j, i*3+2])
                img_raw12[j, i*3+2] = np.bitwise_or(np.left_shift(np.bitwise_and(raw8_byte1, 0x04), 3), img_raw12[j, i*3+2])
                img_raw12[j, i*3+2] = np.bitwise_or(np.left_shift(np.bitwise_and(raw8_byte1, 0x08), 1), img_raw12[j, i*3+2])
                img_raw12[j, i*3+2] = np.bitwise_or(np.right_shift(np.bitwise_and(raw8_byte1, 0x10), 1), img_raw12[j, i*3+2])
                img_raw12[j, i*3+2] = np.bitwise_or(np.right_shift(np.bitwise_and(raw8_byte1, 0x20), 3), img_raw12[j, i*3+2])
                img_raw12[j, i*3+2] = np.bitwise_or(np.right_shift(np.bitwise_and(raw8_byte1, 0x40), 5), img_raw12[j, i*3+2])
                img_raw12[j, i*3+2] = np.bitwise_or(np.right_shift(np.bitwise_and(raw8_byte1, 0x80), 7), img_raw12[j, i*3+2])
            # g_logger.debug("raw12_byte0: {}, raw12_byte1: {}: raw12_byte2: {}".format(img_raw12[j, i*3], img_raw12[j, i*3+1], img_raw12[j, i*3+2]))
    return img_raw12

if __name__ == "__main__":
    plogging.init_logger(log_dir="./", file_name="rgb_to_raw12", level=logging.DEBUG)
    g_logger = plogging.get_logger()
    img_rgb = read_image(img_path="/home/hugoliu/tmp/lerong_fisheye_1080p.jpg", peek=False, resize_to=(2160, 3840))
    # img_rgb = torch.randint(low=0, high=255, size=(3, 4, 4), dtype=torch.uint8)
    img_bayer = rgb_to_bayer(img_rgb)
    img_raw12 = bayer_to_raw12(img_bayer, save_to_path="./raw12_8m_lr.bin", endian="big")
    # g_logger.info("img_raw12: {}".format(img_raw12))