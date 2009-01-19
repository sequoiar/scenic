from twisted.internet import reactor 
from pprint import pprint, pformat
import miville
from utils.observer import Observer
from ui.cli import CliView
import time
"""
Miville for ipython.

See https://svn.sat.qc.ca/trac/miville/wiki/IPython    
"""

#TODO: override notify in order to add reactor.iterate() there.

def go(duration=0.1): # num=999
    """
    Runs the reactor for n seconds
    """
    end = time.time() + duration
    while time.time() < end:
        reactor.iterate()
    
class Update(object):
    """
    Represents a notification from miville's core api.
    """
    def __init__(self, origin, key, value):
        self.value = value
        self.origin = origin
        self.key = key
    
class IPythonController(object):
    def write (self, msg, prompt=False, endl=True):
        print "%s" % (msg.encode('utf-8'))
        
    def write_prompt(self):
        pass

class IPythonView(CliView):
    """  
    ipython results printer. 
    The View (Observer) in the MVC pattern. 
    """
    def __init__(self, subject, controller):
        #Observer.__init__(self, subject)
        #self.controller = controller
        CliView.__init__(self, subject, controller)
        
    def update(self, origin, key, value):
        global updates, last
        #if origin is self.controller:
        last = Update(origin, key, value)
        updates.append(last) # always appends new notifications
        print "-------------------------------------  update:  ------------------------------"
        print "KEY:    %s" % (str(key))
        print "ORIGIN: %s" % (str(origin))
        print "VALUE:  %s" % (pformat(value))
        print "------------------------------------------------------------------------------"
        CliView.update(self, origin, key, value)

updates = []
last = None
core = None
api = None
me = None
view = None

def main():
    global core, api, me, view
    miville.main()
    go(0.25)
    core = miville.core
    api = miville.core.api
    me = IPythonController()
    view = IPythonView(core, me)
    print "iMiville is ready for anything."

if __name__ == '__main__':
    main()

