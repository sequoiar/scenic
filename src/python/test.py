#!/usr/bin/env python

import sys
sys.path.append('.libs')
from milhouse import TcpWrapConfig
from milhouse import DictHandler

class Pycb(DictHandler):
    """
    Basic test for TCP wrapper
    """
    def __init__(self):
        """ nothing """
        pass

    def callback(self, message):
        """
        Callback from TCP
        """
        print message
        return {"command":"ack"}

Config = TcpWrapConfig(1044, 0)
Callback_tested = Pycb()
#t = ThreadWrap(c,cb)
