#!/usr/bin/python3

"""Enable/disable outputs

Usage: tetrapol_cli_output_enabled.py { open | close } CH_NO [CH_NO [CH_NO ...]]"""

from xmlrpc import client
import sys

c = client.Server("http://localhost:60100")
if sys.argv[1] in ("true", "enable", "open", "on"):
    enabled = True
elif sys.argv[1] in ("false", "disable", "close", "off"):
    enabled = False
else:
    raise ValueError("Invalid parameter")
channels = list(set([int(c) for c in sys.argv[2:]]))
c.set_output_enabled(channels, enabled)

