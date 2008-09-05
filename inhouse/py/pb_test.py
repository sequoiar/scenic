#!/usr/bin/env python

import sys

from twisted.spread import pb
from twisted.internet import reactor


class Connection(object):
    def __init__(self):
        self.address = "123.123.123.123"
        ccp = CCP(8800)
        self.ccp = ccp.pb_fac.root

    def send_msg(self, meth, *args):
        

class CCP(object):
    def __init__(self, port):
        self.pb_fac = pb.PBServerFactory(CCPRoot())
        reactor.listenTCP(port, self.pb_fac)

class CCPRoot(pb.Root):
    def __init__(self):
        self.objs = {}
        
    def remote_ccp(self, obj):
        print "received a called for", obj
        
    def add_obj(self, obj):
        self.objs[obj.__name__] = obj
        
class GST(pb.Referenceable):        
    def remote_test(self, arg):
        print "GST:", arg

class Two(pb.Referenceable):
    def remote_print(self, arg):
        print "Two.print() called with", arg

class Core(object):
    def __init__(self):
        self.connections = {}
        
    def send_msg(self, meth, *args):
        for connection in self.connections:
            connection.send_msg(meth, *args)

def got_obj(obj, two):
    print "got One:", obj
    print "giving it our two"
    obj.callRemote("takeTwo", two)
    
    
if __name__ == '__main__':
    core = Core()
    if len(sys.argv) > 1:
        core.connections["test"] = Connection()
#        pb_fac = pb.PBServerFactory(One())
        reactor.listenTCP(8800, pb_fac)
        print dir(pb_fac.root)
        reactor.run()
    else:
        two = Two()
        factory = pb.PBClientFactory()
        reactor.connectTCP("localhost", 8800, factory)
        def1 = factory.getRootObject()
        def1.addCallback(got_obj, two) # hands our 'two' to the callback
        reactor.run()






