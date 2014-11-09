#!/usr/bin/python3

"""Enable/disable outputs

Usage: tetrapol_cli_output_enabled.py CH_NO [open | close]"""

from xmlrpc import client
import sys

channel = int(sys.argv[1])
c = client.Server("http://localhost:60100")
if sys.argv[2] in ("1", "true", "enable", "open", "on"):
    enabled = True
elif sys.argv[2] in ("0", "false", "disable", "close", "off"):
    enabled = False
else:
    raise ValueError("Invalid parameter")
c.set_output_enabled(channel, enabled)

