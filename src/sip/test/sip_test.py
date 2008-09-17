
import sys

from twisted.internet import reactor

import libsip_export as sip

#pool = {}

class Sip(object):
    def __init__(self, port):
        self.session = sip.SIPSession(port)
        
    def connect(self):
        self.session.connect()
    
    def disconnect(self):
        self.session.disconnect()

    def connection_callback(self, state):
        if state == 0:
            print 'Connection NULL'
        elif state == 1:
            print 'Connection ready'
        elif state == 2:
            print 'Connecting'
        elif state == 3:
            print 'Incoming connection'
        elif state == 4:
            print 'Connected'
        elif state == 5:
            print 'Disconnected'
        elif state == 6:
            print 'Connection failed: Timeout'
        elif state == 7:
            print 'Connection failed: Not acceptable'

    def set_python_instance(self):
        self.session.set_python_instance(self)

if __name__ == '__main__':
    session = Sip(5060)
    session.set_python_instance()
    if len(sys.argv) > 1:
        session.connect()
    reactor.run()
