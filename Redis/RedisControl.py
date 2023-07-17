from base64 import decode
from calendar import day_abbr
from pickle import FALSE
import numpy as np
import string
from time import time
from redis import Redis
from math import log2
import csv
import argparse
import struct

def Load(filename = "/share/datasets/CAIDA2018/dataset/130000.dat"):
    data = []
    with open(filename, "rb") as file:
        ftime = -1
        while True:
            trace = file.read(21)
            if len(trace) < 21:
                break
            tkey = struct.unpack("I", trace[:4])[0]
            ttime = struct.unpack("d", trace[13:21])[0]
            if ftime < 0:
                ftime = ttime
            data.append(str(tkey))
            data.append(str(ttime - ftime))
    return data[0:60000]

data = []

def test_frequence(d, n1, n2, n3, n4, test_id):
    rd = Redis(decode_responses=True)
    rd.delete("test1")
    rd.delete("test2")
    hyperbf = rd.execute_command(f"basic_hb.create", "test1", n1, d)
    calm = rd.execute_command(f"calm.create", "test2", n2, n3, n4, d, d*10)
    time_start_hyper = time()
    res = rd.execute_command(f"basic_hb.insert", "test1", *data)
    time_end_hyper = time()
    n = len(res)
    data_ = []
    for i in range(n):
        data_.append(data[i<<1])
        data_.append(data[i<<1|1])
        data_.append(str(res[i]))
    time_start_calm = time()
    res_ = rd.execute_command(f"calm.insert", "test2", *data_)
    time_end_calm = time()
    t_h = len(data)/(time_end_hyper - time_start_hyper)/2
    t_c = len(data)/(time_end_calm - time_start_calm)/2
    t_hc = len(data)/(time_end_calm - time_start_calm + time_end_hyper - time_start_hyper)/2
    return [t_h, t_c, t_hc]

if __name__ == "__main__":
    print("start")
    res = []

    parser = argparse.ArgumentParser()
    parser.add_argument("-f", type=str, required=True, help="Path of the dataset")
    parser.add_argument("-m", type=int, required=True, help="memory")
    parser.add_argument("-t", type=int, required=True, help="repeat time")
    parser.add_argument("-b", type=float, required=True, help="batch limit time")
    args = parser.parse_args()
    x_h = 0
    x_c = 0
    x_hc = 0
    data = Load(args.f)
    repeat = args.t
    b = args.b
    w = args.m
    n1 = 3000
    n3 = int(min(15000, w/15))
    n2 = int(w - n3*8 - n1)
    n4 = int(w - n1 - n2 - n3)
    print("Memory of HyperBF:", n1)
    print("Memory of TimeRecorder:", n2)
    print("Memory of Lru:", n3)
    print("Memory of SS:", n4)
    n2 /= 324
    n3 /= 72
    n4 /= 160
    for i in range(repeat):
        t = test_frequence(b, n1, n2, n3, n4, i)
        x_h += t[0]
        x_c += t[1]
        x_hc += t[2]
    print("Insert througput of HyperBF is: ", x_h / repeat)
    print("Insert througput of CalmSS is: ", x_c / repeat)
    print("Insert througput of HyperCalm is: ", x_hc / repeat)
    