#!/usr/bin/env python
from twisted.trial import unittest
import os
import sys
import subprocess
import pygtk
pygtk.require('2.0')
import warnings

# HACK to raise an exception when we get a GtkWarning
def customwarn(message, category, filename, lineno, file=None, line=None):
        """
        Override warnings.showwarning to avoid importing gtk when it fails to
        open the display.
        """
        sys.stdout.write(warnings.formatwarning(message, category, filename, lineno))
        if "could not open display" in message:
            raise ImportError("Could not open display")

warnings.showwarning = customwarn

no_gtk = False
try:
    import gtk
except ImportError, e:
    print ('Got ImportError "%s"' % str(e))
    no_gtk = True   # we'll skip the test in this case

class TestVumeter(unittest.TestCase):
    def test_vumeter(self):
        """
            Makes a gtk window, gives socket id for the meter program
            to use for its plug which contains the vumeter.
        """
        self.plug_added = False
        self.plug_removed = False
        window = gtk.Window()
        window.set_default_size(80, 480)
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
    elif no_gtk:
        test_vumeter.skip = "Gtk could not open display, cannot run test"

