from milhouse import *
class pycb(DictHandler):
    def cb(self, d):
        print d
        return {"command":"ack"}
c = TcpWrapConfig(1044,0)
cb = pycb()
#t = ThreadWrap(c,cb)
