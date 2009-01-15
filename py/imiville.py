from twisted.internet import reactor 
from pprint import pprint
import miville
from utils.observer import Observer
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

def go(num=999):
    for i in range(num):
        reactor.iterate()

class IPythonController(object):
    pass

class IPythonView(Observer):
    """  
    ipython results printer. 
    The View (Observer) in the MVC pattern. 
    """
    def __init__(self, subject, controller):
        Observer.__init__(self, subject)
        self.controller = controller
        
    def update(self, origin, key, value):
        global ret
        #if origin is self.controller:
        ret = {'value':value, 'key':key, 'origin':origin}
        pprint({'value':value, 'key':key, 'origin':origin})

# ------------------- main: --------------------
ret = None

miville.main()
go()

core = miville.core
api = miville.core.api
me = IPythonController()
view = IPythonView(miville.core, me)


   
