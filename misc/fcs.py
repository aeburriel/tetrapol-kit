#!/usr/bin/python3

from fcs_data import *


for data in data_bin:
    exp = data[-16:]
    data = data[:-16]
    data = xors(data, 16*[1])

#print_buf(data)
    exp = inv(exp)

    for i in 16,:
        r = poly_div(data + i*[0, ], pol)
        print_buf(xors(r, exp))
