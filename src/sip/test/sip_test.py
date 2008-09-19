
import sys

from twisted.internet import reactor, defer

import libsip_export as sip

#pool = {}

class Sip(object):
    def __init__(self, port):
        self.session = sip.SIPSession(port)
        self.set_local_media("audio", "GSM/vorbis/PCMA/",12345,"sendrecv")
        
    def connect(self):
        return self.session.connect()

    def disconnect(self):
        return self.session.disconnect()

    def shutdown(self):
        return self.session.shutdown()

    def state(self):
        return self.session.state()

    def error_reason(self):
        return self.session.error_reason()

    def set_local_media(self, type, codecs, port, dir):
        self.session.set_local_media(type, codecs, port, dir)

    def reinvite(self):
        return self.session.reinvite()

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
        elif state == 8:
            print 'Message received:', self.session.get_message()

    def set_python_instance(self):
        print self
        self.session.set_python_instance(self)

if __name__ == '__main__':
    session = Sip(5060)
    session.set_python_instance()
    if len(sys.argv) > 1:
        session.connect()
    reactor.run()
