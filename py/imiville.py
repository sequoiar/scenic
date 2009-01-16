from twisted.internet import reactor 
from pprint import pprint
import miville
from utils.observer import Observer
import time
"""
Miville for ipython.


SYNOPSIS:
from imiville import *
start()
    
api.add_contact(me, 'alex', '10.10.10.99')
go()
api.get_contacts(me)
go()
    
print ret['value']
    
from utils import commands
commands.single_command_start(['ls'])
go()
api.notify(me, 'value', 'key')
go()
    
for i in miville.core.observers.itervalues(): pprint(i)
"""

#TODO: override notify in order to add reactor.iterate() there.

def go(duration=0.1): # num=999
    """
    Runs the reactor for n seconds
    """
    end = time.time() + duration
    while time.time() < end:
        reactor.iterate()
    #reactor.callLater(duration, reactor.stop)
    #reactor.run()
    #for i in range(num):
    #    reactor.iterate()

class IPythonController(object):
    pass

class Update(object):
    """
    Represents a notification from miville's core api.
    """
    def __init__(self, origin, key, value):
        self.value = value
        self.origin = origin
        self.key = key

class IPythonView(Observer):
    """  
    ipython results printer. 
    The View (Observer) in the MVC pattern. 
    """
    def __init__(self, subject, controller):
        Observer.__init__(self, subject)
        self.controller = controller
        
    def update(self, origin, key, value):
        global updates, last
        #if origin is self.controller:
        last = Update(origin, key, value)
        updates.append(last) # always appends new notifications
        print "\nKEY: %s" % (str(key))
        print "VALUE: "
        pprint(value)

# ------------------- main: --------------------
updates = []
last = None

miville.main()
go(0.25)

core = miville.core
api = miville.core.api
me = IPythonController()
view = IPythonView(core, me)

print "iMiville is ready for anything."

