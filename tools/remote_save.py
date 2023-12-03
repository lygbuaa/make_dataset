#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
### 3rd-party dependency:
sudo apt install libzmq3-dev
pip install pyzmq
pip install --upgrade setuptools
pip install numpy
pip install opencv-python
'''

import numpy as np
import cv2, sys, zmq, time, json, sys, os
from datetime import datetime

G_SERVER_IP = "127.0.0.1"
G_DATA_PORT = "10001"
G_IMG_H = 1080
G_IMG_W = 1920
#config file saving path:
G_SNAPSHOT_DIR = "/home/hugoliu/snap/"
#grab 1 frame out of this number of frames:
G_DOWN_SAMPLE = 10

class ImageSnapper(object):
    saved_cnt = 0
    total_cnt = 0

    def __init__(self):
        if not os.path.exists(G_SNAPSHOT_DIR):
            os.mkdir(G_SNAPSHOT_DIR)
        print("snapper will save file to: %s" % G_SNAPSHOT_DIR)

    def __del__(self):
        print("snapper quit.")

    def GetTimeStr(self, id):
        dt = datetime.now()
        dt.microsecond
        time_str  = "id{:06d}-{:04d}-{:02d}-{:02d}-{:02d}-{:02d}-{:02d}-{:06d}.jpeg".format(int(id), dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, dt.microsecond)
        return time_str

    def SaveBuffer(self, buf, id):
        self.total_cnt += 1
        if(self.total_cnt % G_DOWN_SAMPLE == 0):
            self.saved_cnt += 1
            fullpath = G_SNAPSHOT_DIR + self.GetTimeStr(id)
            file_handle = open(fullpath, 'wb', buffering=0)
            file_handle.write(buf)
            file_handle.close()
            if(self.saved_cnt % G_DOWN_SAMPLE == 0):
                print("saved {:d} frames.".format(self.saved_cnt))
        else:
            pass

    def SaveCvMat(self, image, id):
        self.total_cnt += 1
        if(self.total_cnt % G_DOWN_SAMPLE == 0):
            self.saved_cnt += 1
            fullpath = G_SNAPSHOT_DIR + self.GetTimeStr(id)
            cv2.imwrite(fullpath, image)
            if(self.saved_cnt % G_DOWN_SAMPLE == 0):
                print("saved {:d} frames.".format(self.saved_cnt))
        else:
            pass

if __name__ == '__main__':
    context = zmq.Context()
    socket_data = context.socket(zmq.SUB)
    socket_data.connect ("tcp://" + G_SERVER_IP + ":" + G_DATA_PORT)
    socket_data.setsockopt(zmq.SUBSCRIBE, b'')
    snapper = ImageSnapper()
    time.sleep(1) #sleep util zmq ready.
    print("zmq listening on {}:{}".format(G_SERVER_IP, G_DATA_PORT))

    # show image
    try:
        counter = 0
        while True:
            msg = socket_data.recv()
            buf = np.frombuffer(msg, dtype=np.uint8)
            snapper.SaveBuffer(buf, counter)
            # img = cv2.imdecode(buf, cv2.IMREAD_COLOR)
            counter = counter + 1
            print("recv img counter: {}, shape: {}".format(counter, buf.shape))
            # cv2.namedWindow('zmq_image', cv2.WINDOW_NORMAL)
            # cv2.imshow('zmq_image', img)
            # #press any key to quit
            # if cv2.waitKey(5) >= ord(' '):
            #     break

        cv2.destroyAllWindows()
        exit(0)
    except KeyboardInterrupt:
        cv2.destroyAllWindows()
        print('break by user.')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)