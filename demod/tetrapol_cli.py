#!/usr/bin/python

import time
from xmlrpc import client

c = client.Server("http://localhost:60100")


pwr = c.get_channels_pwr()

pwr = [(pwr[ch], ch) for ch in range(len(pwr))]
pwr.sort(reverse=True)
for i in range(30):
    print(pwr[i])

c.set_auto_tune(95)
time.sleep(15)

c.set_output_state(95, True)

