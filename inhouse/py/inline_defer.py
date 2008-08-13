
from twisted.internet import reactor, defer
from twisted.web.client import getPage

import time


@defer.inlineCallbacks
def test():
    print 'Start test'
    query = "http://www.google.com"
    prev = time.time()
    print 'before getPage', prev
    content = yield getPage(query)
#    print content
    print 'after getPage', time.time() - prev
#    return content
    defer.returnValue(content)    
    print 'End test'
#test = defer.inlineCallbacks(test)

def out(t):
    print t
    reactor.stop()
        
def main():
    prev = time.time()
    print "Starting ", prev
    t = test()
    print "Stoping ", time.time() - prev
#    t.addCallback(out)
    reactor.callLater(5, out, t)
#    print t
#    reactor.stop()

reactor.callLater(0, main)
reactor.run()