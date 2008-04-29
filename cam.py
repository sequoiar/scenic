#!/usr/bin/env python

import sys, os
import pygtk, gtk, gobject
import pygst
pygst.require("0.10")
import gst

class GTK_Main:
    """Get dv from a camera over firewire and output its video and sound."""

    def __init__(self):
        """Create gtk window and widgets, as well as the gst pipeline."""

        window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        window.set_title("dv-Player")
        window.set_default_size(500, 400)
        window.connect("destroy", gtk.main_quit, "WM destroy")
        vbox = gtk.VBox()
        window.add(vbox)
        hbox = gtk.HBox()
        vbox.pack_start(hbox, False)
        self.button = gtk.Button("Start")
        hbox.pack_start(self.button, False)
        self.button.connect("clicked", self.start_stop)
        self.movie_window = gtk.DrawingArea()
        vbox.add(self.movie_window)
        window.show_all()
        
        """ Create pipeline. All the examples I've seen for DV use
            gst.parse_launch() but I would be interested to try it by
            instantiating all the elements seperately.
        """

        self.pipeline = gst.parse_launch("dv1394src ! dvdemux name=demux \
                                          demux. ! queue ! dvdec ! \
                                          xvimagesink sync=false demux. ! \
                                          queue ! audioconvert ! \
                                          alsasink sync=false")

        """ Create bus. From the documentation:
            A bus is a simple system that takes care of forwarding messages 
            from the pipeline threads to an application in its own thread 
            context. The advantage of a bus is that an application does not 
            need to be thread-aware in order to use GStreamer, even though 
            GStreamer  itself is heavily threaded.
        """ 
    
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.enable_sync_message_emission()
        bus.connect('message', self.on_message)
        bus.connect('sync-message::element', self.on_sync_message)
        
    def start_stop(self, w):
        """Respond to start/stop button being toggled."""
        if self.button.get_label() == "Start":
            self.button.set_label("Stop")
            self.pipeline.set_state(gst.STATE_PLAYING)
        else:
            self.pipeline.set_state(gst.STATE_NULL)
            self.button.set_label("Start")
                        
    def on_message(self, bus, message):
        """Have pipeline respond to msgs from bus."""
        t = message.type
        if t == gst.MESSAGE_EOS:
            self.pipeline.set_state(gst.STATE_NULL)
            self.button.set_label("Start")
        elif t == gst.MESSAGE_ERROR:
            err, debug = message.parse_error()
            print "Error: %s" % err, debug
            self.pipeline.set_state(gst.STATE_NULL)
            self.button.set_label("Start")
    
    def on_sync_message(self, bus, message):
        if message.structure is None:
            return
        message_name = message.structure.get_name()
        if message_name == 'prepare-xwindow-id':
            imagesink = message.src
            imagesink.set_property('force-aspect-ratio', True)
            imagesink.set_xwindow_id(self.movie_window.window.xid)


GTK_Main()
gtk.gdk.threads_init()
gtk.main()

