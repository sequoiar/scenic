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
        self.objs = []
        
    def add(self, obj):
        self.objs.append(obj)

def send(root):
    root.callRemote('test')
    
    
if __name__ == '__main__':
    
    pbl = PBList()
    gst = GST()
    pbl.add(gst)
    
    # Do this when connecting (SIP)
    one = One(pbl)
    reactor.listenTCP(8800, pb.PBServerFactory(one))
    
    # simulate a call to a root method
    reactor.callLater(3, send, one)

    # simulate a call to another Referenceable class method

    
    reactor.run()






