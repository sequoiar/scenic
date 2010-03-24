#!/usr/bin/env python
from twisted.trial import unittest
import sys
import subprocess
import pygtk
pygtk.require('2.0')
import gtk

class TestVumeter(unittest.TestCase):
    def test_vumeter(self):
        """
            Makes a gtk window, gives socket id for the meter program
            to use for its plug which contains the vumeter.
        """
        window = gtk.Window()
        window.show()

        socket = gtk.Socket()
        socket.show()
        window.add(socket)

        print "Socket ID=", socket.get_id()
        window.connect("destroy", gtk.main_quit)

        def plugged_event(widget):
            print "I (", widget, ") have just had a plug inserted!"

        socket.connect("plug-added", plugged_event)
        socket.connect("plug-removed", gtk.main_quit)

        # redirect stderr to stdout
        print "opening process"
        command = 'meter %ld' % (socket.get_id())
        proc = subprocess.Popen([command, '"to stdout"'], 
                shell=True, 
                stderr=subprocess.STDOUT,
                stdout=subprocess.PIPE)
        #stdout_value = proc.communicate()[0]
        #print stdout_value
        gtk.main()

