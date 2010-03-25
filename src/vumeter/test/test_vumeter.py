#!/usr/bin/env python
from twisted.trial import unittest
import os
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
        self.plug_added = False
        self.plug_removed = False
        window = gtk.Window()
        window.show()

        socket = gtk.Socket()
        socket.show()
        window.add(socket)

        print "Socket ID=", socket.get_id()
        window.connect("destroy", gtk.main_quit)

        def on_plug_added(widget):
            print "I (", widget, ") have just had a plug inserted!"
            self.plug_added = True
        def on_plug_removed(widget):
            """ 
                This would return True if we wanted to reuse the 
                socket.
            """
            print "I (", widget, ") have just had a plug removed!"
            self.plug_removed = True
            gtk.main_quit()

        socket.connect("plug-added", on_plug_added)
        socket.connect("plug-removed", on_plug_removed)

        # redirect stderr to stdout
        print "opening process"
        command = 'meter %ld' % (socket.get_id())
        proc = subprocess.Popen([command, '"to stdout"'], 
                shell=True)
        gtk.main()
        self.failUnless(self.plug_added)
        self.failUnless(self.plug_removed)

    if "DISPLAY" not in os.environ:
        test_vumeter.skip = "No DISPLAY set, cannot run test"

