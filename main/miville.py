#!/usr/bin/env python

# Twisted imports
from twisted.internet import reactor

import ui

# this is the base class mediator, implemented
# as a template pattern. The main controller should inherit from this.
class Mediator:
    def __init__(self):
        pass

    def colleague_changed(self, colleague, event):
        """Template pattern interface method, intended to be called directly"""
        self._colleague_changed(colleague, event)

    def _colleague_changed(self, colleague, event):
        """Template pattern polymorphic method, intended to be inherited"""
    

# this is the base class colleague, implemented
# as a template pattern. The input classes should inherit from this.
class Colleague:
    def __init__(self, mediator):
        self.mediator = mediator

    def changed(self, colleague, event):
        """Template pattern inteface method, intended to be called directly"""
        self._changed(colleague, event)

    def _changed(self, colleague, event):
        """Template pattern interface method, intended to be inherited"""
        self.mediator.colleague_changed(colleague, event)


class ColleagueExample(Colleague):
    """This takes part in the Mediator/Colleague pattern"""

    def __init__(self, mediator, *_args, **_kwargs):
        # somewhat wacky initialization of parent control
        Colleague.__init__(self, mediator)

    def on_event(self, event):
        self.changed(self, event)


# main frame of our application
class MediatorExample(Mediator):
    """The MainFrame of the application"""
    
    def __init__(self):
        Mediator.__init__(self)

    def _colleague_changed(self, colleague, event, *args, **kargs):
        """This method gets called when when of the colleagues has changed.
        This is the central 'clearing' house for all changes and orchestrates
        these changes among the cooperating controls"""

        if event[0] != '_':
            try:
                callback = getattr(self, event)
            except:
                print "The callback %s doesn't exist." % (event)
            if callback:          
                 callback(colleague, *args, **kargs)
       

# main frame of our application
class MivilleMediator(Mediator):
    """The MainFrame of the application"""
    
    def __init__(self):
        Mediator.__init__(self)
        all_ui = ui.find_ui()
        ui.cli.start()


    def _colleague_changed(self, colleague, event, *args, **kargs):
        """This method gets called when when of the colleagues has changed.
        This is the central 'clearing' house for all changes and orchestrates
        these changes among the cooperating controls"""

        if event[0] != '_':
            try:
                callback = getattr(self, event)
            except:
                print "The callback %s doesn't exist." % (event)
            if callback:          
                 callback(colleague, *args, **kargs)
       


class LocalStream:
    """That represent all the settings of a stream to send """

    def __init__(self):
        self.a_codec = ""
        self.v_codec = ""
        self.address = ""
    
    def connect(self, address=""):
        print "Connecting to %s" % (address)
    
    def status(self):
        return (self.a_codec, self.v_codec, self.address)


def main():
    all_ui = ui.find_ui()
    print all_ui
#    for each_ui in all_ui:
#        each_ui.start()
    ui.cli.start()
    reactor.run()


if __name__ == '__main__':
    med = MivilleMediator()
    reactor.run()
    

