#!/usr/bin/env python

import sys

from twisted.spread import pb
from twisted.internet import reactor


class One(pb.Root):
    def __init__(self):
        self.remote = None
        self.robjs = None
        self.objs = {}
        
    def remote_init(self, obj):
        if not self.remote:
            print "received a called for", obj
            self.remote = obj
            obj.callRemote('init', self)
#            obj.callRemote('init', self).addCallback(self.set_robjs)
#            return self.objs
        
    def set_robjs(self, robjs):
        self.robjs = robjs

            
    def remote_test(self, *args):
        if len(args) == 0:
            print 'Test'
        else:
            cmd = args[0]
            self.objs[cmd](*args[1:])
        
    def callRemote(self, *args):
        if self.remote:
            self.remote.callRemote('test', *args)
            
    def add_meth(self, meth):
        self.objs[meth.__name__] = meth
        print self.objs
                
class GST(object):        
    def gst(self, arg):
        print "GST:", arg

class PBList(pb.Copyable):
    def __init__(self):
        self.objs = []
        
    def add(self, obj):
        self.objs.append(obj)

def send(root):
    root.callRemote()
    
    
if __name__ == '__main__':
    
#    pbl = PBList()
    gst = GST()
#    pbl.add(gst)
    
    # Do this when connecting (SIP)
    one = One()
    one.add_meth(gst.gst)
    reactor.listenTCP(8800, pb.PBServerFactory(one))
    
    # simulate a call to a root method
    reactor.callLater(3, send, one)

    # simulate a call to another Referenceable class method

    
    reactor.run()






