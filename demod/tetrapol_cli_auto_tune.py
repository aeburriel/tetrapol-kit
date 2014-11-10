#!/usr/bin/python3

"""Get/set channel for automatic frequency fine tunning.

Usage: tetrapol_cli_auto_tune.py [CH_NO]"""

from xmlrpc import client
import sys

c = client.Server("http://localhost:60100")
if len(sys.argv) == 2:
    c.set_auto_tune(int(sys.argv[1]))
else:
    print(c.get_auto_tune())
