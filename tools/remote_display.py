#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
### 3rd-party dependency:
sudo apt install libzmq3-dev
sudo apt install python-zmq
sudo pip install --upgrade setuptools
sudo pip install numpy Matplotlib
sudo pip install opencv-python
'''

import numpy as np
import cv2, sys, zmq, time, json, sys, os
from datetime import datetime

G_SERVER_IP = "127.0.0.1"
G_DATA_PORT = "10001"
G_IMG_H = 1080
G_IMG_W = 1920
#grab 1 frame out of this number of frames:
down_sample = 1

class ImageSnapper:
    saved_cnt = 0
    total_cnt = 0

    def __init__(self, snapshot_dir):
        print("snapper will save file to: %s" % snapshot_dir)

    def __del__(self):
        print("snapper quit.")

    def GetTimeStr(self, id):
        dt = datetime.now()
        dt.microsecond
        time_str  = "id{:06d}-{:04d}-{:02d}-{:02d}-{:02d}-{:02d}-{:02d}-{:06d}.jpg".format(int(id), dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, dt.microsecond)
        return time_str

def cls_to_color(in_tensor):
    palette = np.array([[0, 0, 0],
                        [255, 0, 0],
                        [0, 255, 0],
                        [0, 0, 255],
                        [0, 255, 255],
                        [255, 0, 255],
                        [255, 255, 0],
                        [255, 255, 255],
                        [128, 0, 0],
                        [0, 128, 0],
                        [0, 0, 128],
                        [0, 128, 128],
                        [128, 0, 128],
                        [128, 128, 0],
                        [128, 128, 128]])
    cls = np.argmax(in_tensor, axis=0)
    return palette[cls]

if __name__ == '__main__':
    context = zmq.Context()
    socket_data = context.socket(zmq.SUB)
    socket_data.connect ("tcp://" + G_SERVER_IP + ":" + G_DATA_PORT)
    socket_data.setsockopt(zmq.SUBSCRIBE, b'')
    time.sleep(1) #sleep util zmq ready.
    print("zmq listening on {}:{}".format(G_SERVER_IP, G_DATA_PORT))

    # show image
    try:
        counter = 0
        while True:
            msg = socket_data.recv()
            # show rgb image
            # img = np.frombuffer(msg, dtype=np.uint8).reshape(G_IMG_H, G_IMG_W, 3)

            # show jpeg image
            buf = np.frombuffer(msg, dtype=np.uint8)
            img = cv2.imdecode(buf, cv2.IMREAD_COLOR)

            # show segment tensor 16*256*480
            # img = np.frombuffer(msg, dtype=np.int8).reshape(16, 256, 480)
            # view_cls = cls_to_color(img)
            # img = view_cls.astype(np.uint8)

            # show lanline tensor 6*112*320
            # img = np.frombuffer(msg, dtype=np.int8).reshape(6, 112, 320)
            # view_cls = cls_to_color(img)
            # img = view_cls.astype(np.uint8)

            counter = counter + 1
            print("recv img counter: {}, shape: {}".format(counter, img.shape))

            cv2.imshow('zmq_image', img)
            #press any key to quit
            if cv2.waitKey(5) >= ord(' '):
                break

        cv2.destroyAllWindows()
        exit(0)
    except KeyboardInterrupt:
        cv2.destroyAllWindows()
        print('break by user.')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)