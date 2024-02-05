#! /usr/bin/env python3
# -*- coding: utf-8 -*-

import time, threading, socket, struct
import traceback, logging
import plogging
import numpy as np
# from numba import njit, prange
import cv2

if __name__ == "__main__":
    cap = cv2.VideoCapture("/home/hugoliu/alaska/data/carla_l2_town04_highway/town04_highway_left_turn.mp4")
    counter = 0
    while True:
        ret, frame = cap.read()
        if not ret:
            print("video reach end.")
            break
        print("capture counter: {}".format(counter))
        counter += 1
        cv2.imshow("peek_image", frame)
        cv2.waitKey(1000)
        img_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        with open("./2m.rgb3", "wb") as file:
            file.write(img_rgb.tobytes())
