
import random, sys

from twisted.internet import reactor

import libsip_export as sip


pool = {}


class Sip(object):
    def __init__(self, id, port):
        self.session = sip.SIPSession(id, port)
        
    def connect(self):
        self.session.connect()
    
    def connection_made(arg):
        print arg



class SipFactory(object):
    
    pool = {}
    
    def get(self, port):
        id = self.get_id()
        session = Sip(id, port)
        self.pool[id] = session
        return session
       
    
    def get_id(self):
        id = random.randint(1, 100)
        if id in self.pool:
            id = self.get_id()
        return id
        
        

def main():
    factory = SipFactory()
    session = factory.get(50600)
    if len(sys.argv) > 1:
        session.connect()

def connection_made_cb(id, arg):
    session = SipFactory.pool[id]
    session.connection_made(arg)


if __name__ == '__main__':
    main()
    reactor.run()