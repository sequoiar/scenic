#!/usr/bin/env python

# Twisted imports
from twisted.internet import reactor, protocol
from twisted.plugin import getPlugins
#from ui import iui
from ui import iui, cli


def find_ui():
    """
    Find all the difference user interfaces available
    """
    for uinterface in getPlugins(iui.IUI, cli): 
        print uinterface  
    print "fin"

def main():
    find_ui()
    reactor.run()


if __name__ == '__main__':
    main()