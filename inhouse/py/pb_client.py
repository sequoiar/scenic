#!/usr/bin/env python

import sys

from twisted.spread import pb
from twisted.internet import reactor


class One(pb.Root):
    def __init__(self, objs):
        self.remote = None
        self.robjs = None
        self.objs = GST()
        
    def remote_init(self, obj):
        if not self.remote:
            print "received a called for", obj
            self.remote = obj
            obj.callRemote('init', self).addCallback(self.set_robjs)
            return self.objs
        
    def set_robjs(self, robjs):
        self.robjs = robjs
        
    def remote_test(self):
        print 'Test'
        
    def callRemote(self, *args):
        if self.remote:
            self.remote.callRemote(*args)
                
class GST(pb.Referenceable):        
    def remote_test(self, arg):
        print "GST:", arg

class PBList(object):
    def __init__(self):
        self.objs = {}
        
    def add(self, obj):
        self.objs[obj.__name__] = obj

def send(root):
    root.callRemote('test')
    
def send2(root):
    print root.robjs
    root.robjs.callRemote('test', 22)
    
if __name__ == '__main__':

    pbl = PBList()

    # Do this when after a connection (SIP) is establish
    factory = pb.PBClientFactory()
    reactor.connectTCP("localhost", 8800, factory)
    one = One(pbl)
    def1 = factory.getRootObject()
    def1.addCallback(one.remote_init)
    
    # simulate a call to a root method
    reactor.callLater(5, send, one)

    # simulate a call to another Referenceable class method
    reactor.callLater(7, send2, one)
    
    reactor.run()






