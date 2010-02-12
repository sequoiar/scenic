#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.

"""
Main application classes.

Negotiation is done as follow:
------------------------------
 * {"msg":"INVITE", "videoport":10000, "audioport":11000, "sid":0, "please_send_to_port":999}
   * Each peer ask for ports to send to, and of media settings as well. "video": [{"port":10000, "codec":"mpeg4", "bitrate":3000000}]
 * {"msg":"ACCEPT", "videoport":10000, "audioport":11000, "sid":0}
 * {"msg":"REFUSE", "sid":0}
 * {"msg":"CANCEL", "sid":0}
 * {"msg":"ACK", "sid":0}
 * {"msg":"BYE", "sid":0}
 * {"msg":"OK", "sid":0}

"""
import os
import gtk # for dialog responses. TODO: remove
from twisted.internet import defer
from twisted.internet import error
from twisted.internet import task
from twisted.internet import reactor
from scenic import communication
from scenic import saving
from scenic import process # just for constants
from scenic.streamer import StreamerManager
from scenic import dialogs
from scenic import ports
from scenic.devices import jackd
from scenic.devices import x11
from scenic.devices import cameras
from scenic import gui
_ = gui._ # gettext

class Config(saving.ConfigStateSaving):
    """
    Class attributes are default.
    """
    # Default values
    negotiation_port = 17446 # receiving TCP (SIC) messages on it.
    smtpserver = "smtp.sat.qc.ca"
    email_info = "scenic@sat.qc.ca"
    audio_source = "jackaudiosrc"
    audio_sink = "jackaudiosink"
    audio_codec = "raw"
    audio_channels = 2
    video_source = "v4l2src"
    video_device = "/dev/video0"
    video_deinterlace = False
    video_input = 0
    video_standard = "ntsc"
    video_sink = "xvimagesink"
    video_codec = "mpeg4"
    video_display = ":0.0"
    video_fullscreen = False
    video_capture_size = "640x480"
    #video_window_size = "640x480"
    video_aspect_ratio = "4:3" 
    confirm_quit = True
    theme = "Darklooks"
    #video_bitrate = 3000000
    video_bitrate = 3.0
    video_jitterbuffer = 75

    def __init__(self):
        config_file = 'scenic.cfg'
        config_dir = os.environ['HOME'] + '/.scenic'
        config_file_path = os.path.join(config_dir, config_file)
        saving.ConfigStateSaving.__init__(self, config_file_path)

class Application(object):
    """
    Main class of the application.

    The devices attributes is a very interesting dict. See the source code.
    """
    def __init__(self, kiosk_mode=False, fullscreen=False):
        self.config = Config()
        self.recv_video_port = None
        self.recv_audio_port = None
        self.remote_audio_config = {} # dict
        self.remote_video_config = {} # dict
        self.ports_allocator = ports.PortsAllocator()
        self.address_book = saving.AddressBook()
        self.streamer_manager = StreamerManager(self)
        self._has_session = False
        self.streamer_manager.state_changed_signal.connect(self.on_streamer_state_changed) # XXX
        print("Starting SIC server on port %s" % (self.config.negotiation_port)) 
        self.server = communication.Server(self, self.config.negotiation_port) # XXX
        self.client = communication.Client(self.on_connection_error) # XXX
        self.protocol_version = "SIC 0.1"
        self.got_bye = False 
        # starting the GUI:
        self.gui = gui.Gui(self, kiosk_mode=kiosk_mode, fullscreen=fullscreen)
        self.devices = {
            "x11_displays": [], # list of dicts
            "cameras": {}, # dict of dicts (only V4L2 cameras for now)
            #"dc_cameras": [], # list of dicts
            "xvideo_is_present": False, # bool
            "jackd_is_running": False 
            }
        self._jackd_watch_task = task.LoopingCall(self._poll_jackd)
        reactor.callLater(0, self._start_the_application)

    def _start_the_application(self):
        """
        Should be called only once.
        (once Twisted's reactor is running)
        """
        reactor.addSystemEventTrigger("before", "shutdown", self.before_shutdown)
        try:
            self.server.start_listening()
        except error.CannotListenError, e:
            def _cb(result):
                reactor.stop()
            print("Cannot start SIC server. %s" % (e))
            deferred = dialogs.ErrorDialog.create("Is another Scenic running? Cannot bind to port %d" % (self.config.negotiation_port), parent=self.gui.main_window)
            deferred.addCallback(_cb)
            return
        # Devices: JACKD
        self._jackd_watch_task.start(10, now=True)
        # Devices: X11 and XV
        def _callback(result):
            self.gui.update_widgets_with_saved_config()
        deferred_list = defer.DeferredList([
            self.poll_x11_devices(), 
            self.poll_xvideo_extension(),
            self.poll_camera_devices()
            ])
        deferred_list.addCallback(_callback)

    def poll_x11_devices(self):
        """
        Called once at startup, and then the GUI can call it.
        Calls gui.update_x11_devices.
        @rettype: Deferred
        """
        deferred = x11.list_x11_displays(verbose=False)
        def _callback(x11_displays):
            self.devices["x11_displays"] = x11_displays
            print("displays: %s" % (x11_displays))
            self.gui.update_x11_devices()
        deferred.addCallback(_callback)
        return deferred


    def poll_camera_devices(self):
        """
        Called once at startup, and then the GUI can call it.
        Calls gui.update_camera_devices.
        For now, we only take into account V4L2 cameras.
        @rettype: Deferred
        """
        deferred = cameras.list_cameras()
        def _callback(cameras):
            self.devices["cameras"] = cameras
            print("cameras: %s" % (cameras))
            self.gui.update_camera_devices()
        deferred.addCallback(_callback)
        return deferred

    def poll_xvideo_extension(self):
        """
        Called once at startup, and then the GUI can call it.
        @rettype: Deferred
        """
        deferred = x11.xvideo_extension_is_present()
        def _callback(xvideo_is_present):
            self.devices["xvideo_is_present"] = xvideo_is_present
            if not xvideo_is_present:
                msg = _("It seems like the xvideo extension is not present our your X11 display ! It is likely to be impossible to receive a video stream.")
                print(msg)
                dialogs.ErrorDialog.create(msg, parent=self.gui.main_window)
        deferred.addCallback(_callback)
        return deferred
        
    def _poll_jackd(self):
        """
        Checks if the jackd default audio server is running.
        Called every n seconds.
        """
        is_running = False
        is_zombie = False
        try:
            jack_servers = jackd.jackd_get_infos() # returns a list a dict such as :
        except jackd.JackFrozenError, e:
            print e 
            msg = _("The JACK audio server seems frozen ! \n%s") % (e)
            print(msg)
            #dialogs.ErrorDialog.create(msg, parent=self.gui.main_window)
            is_zombie = True
        else:
            #print "jackd servers:", jack_servers
            if len(jack_servers) == 0:
                is_running = False
            else:
                is_running = True
        self.devices["jackd_is_running"] = is_running
        self.gui.update_jackd_status(is_running, is_zombie)
    
    def before_shutdown(self):
        """
        Last things done before quitting.
        """
        print("The application is shutting down.")
        # TODO: stop streamers
        if self.client.is_connected():
            if not self.got_bye:
                self.send_bye()
                self.stop_streamers()
            self.disconnect_client()
        print('stopping server')
        self.server.close()
    # ------------------------- session occuring -------------
    def has_session(self):
        """
        @rettype: bool
        """
        return self._has_session
    # -------------------- streamer ports -----------------

    def allocate_ports(self):
        # TODO: start_session
        self.recv_video_port = self.ports_allocator.allocate()
        self.recv_audio_port = self.ports_allocator.allocate()

    def free_ports(self):
        # TODO: stop_session
        for port in [self.recv_video_port, self.recv_audio_port]:
            try:
                self.ports_allocator.free(port)
            except ports.PortsAllocatorError, e:
                print(e)

    def save_configuration(self):
        """
        Saves the configuration to a file.
        Reads the widget value prior to do it.
        """
        self.gui._gather_configuration() # need to get the value of the configuration widgets.
        self.config.save()
        self.address_book.save() # addressbook values are already stored.
    # --------------------------- network receives ------------

    def _check_protocol_version(self, message):
        """
        Checks if the remote peer's SIC protocol matches.
        @param message: dict messages received in an INVITE or ACCEPT SIC message. 
        @rettype: bool
        """
        # TODO: break if not compatible in a next release.
        if message["protocol"] != self.protocol_version:
            print("WARNING: Remote peer uses %s and we use %s." % (message["protocol"], self.protocol_version))
            return False
        else:
            return True

    def handle_invite(self, message, addr):
        self.got_bye = False
        self._check_protocol_version(message)
        
        def _on_contact_request_dialog_response(response):
            """
            User is accetping or declining an offer.
            @param result: Answer to the dialog.
            """
            if response == gtk.RESPONSE_OK:
                self.send_accept(addr)
            elif response == gtk.RESPONSE_CANCEL or gtk.RESPONSE_DELETE_EVENT:
                self.send_refuse_and_disconnect() 
            else:
                pass
            return True

        if self.streamer_manager.is_busy():
            send_to_port = message["please_send_to_port"]
            communication.connect_send_and_disconnect(addr, send_to_port, {'msg':'REFUSE', 'sid':0}) #FIXME: where do we get the port number from?
            print("Refused invitation: we are busy.")
        elif not self.devices["jackd_is_running"]:
            send_to_port = message["please_send_to_port"]
            communication.connect_send_and_disconnect(addr, send_to_port, {'msg':'REFUSE', 'sid':0}) #FIXME: where do we get the port number from?
            dialogs.ErrorDialog.create("Refused invitation: jack is not running.", parent=self.gui.main_window)
        else:
            self.remote_audio_config = message["audio"]
            self.remote_video_config = message["video"]
            self.client.connect(addr, message["please_send_to_port"])
            # TODO: if a contact in the addressbook has this address, displays it!
            text = _("<b><big>%s is inviting you.</big></b>\n\nDo you accept the connection?" % addr)
            self.gui.show_invited_dialog(text, _on_contact_request_dialog_response)

    def handle_cancel(self):
        self.client.disconnect()
        self.gui.invited_dialog.hide()
        dialogs.ErrorDialog.create("Remote peer cancelled invitation.", parent=self.gui.main_window)

    def handle_accept(self, message, addr):
        self._check_protocol_version(message)
        self.gui._unschedule_offerer_invite_timeout()
        self.got_bye = False
        # TODO: Use session to contain settings and ports
        self.gui.hide_calling_dialog("accept")
        self.remote_audio_config = message["audio"]
        self.remote_video_config = message["video"]
        if self.streamer_manager.is_busy():
            dialogs.ErrorDialog.create("A streaming session is already in progress.", parent=self.gui.main_window)
        else:
            print("Got ACCEPT. Starting streamers as initiator.")
            self.start_streamers(addr)
            self.send_ack()

    def handle_refuse(self):
        self.gui._unschedule_offerer_invite_timeout()
        self.free_ports()
        self.gui.hide_calling_dialog("refuse")

    def handle_ack(self, addr):
        print("Got ACK. Starting streamers as answerer.")
        self.start_streamers(addr)

    def handle_bye(self):
        self.got_bye = True
        self.stop_streamers()
        if self.client.is_connected():
            print('disconnecting client and sending BYE')
            self.client.send({"msg":"OK", "sid":0})
            self.disconnect_client()

    def handle_ok(self):
        print("received ok. Everything has an end.")
        print('disconnecting client')
        self.disconnect_client()

    def on_server_receive_command(self, message, addr):
        # XXX
        msg = message["msg"]
        print("Got %s from %s" % (msg, addr))
        
        if msg == "INVITE":
            self.handle_invite(message, addr)
        elif msg == "CANCEL":
            self.handle_cancel()
        elif msg == "ACCEPT":
            self.handle_accept(message, addr)
        elif msg == "REFUSE":
            self.handle_refuse()
        elif msg == "ACK":
            self.handle_ack(addr)
        elif msg == "BYE":
            self.handle_bye()
        elif msg == "OK":
            self.handle_ok()
        else:
            print ('WARNING: Unexpected message %s' % (msg))

    # -------------------------- actions on streamer manager --------

    def start_streamers(self, addr):
        self._has_session = True
        self.streamer_manager.start(addr, self.config)

    def stop_streamers(self):
        self.streamer_manager.stop()

    def on_streamers_stopped(self, addr):
        """
        We call this when all streamers are stopped.
        """
        print("on_streamers_stopped got called")
        self._has_session = False
        self.free_ports()

    # ---------------------- sending messages -----------
        
    def disconnect_client(self):
        """
        Disconnects the SIC sender.
        @rettype: L{Deferred}
        """
        def _cb(result, d1):
            d1.callback(True)
        def _cl(d1):
            if self.client.is_connected():
                d2 = self.client.disconnect()
                d2.addCallback(_cb, d1)
            else:
                d1.callback(True)
        if self.client.is_connected():
            d = defer.Deferred()
            reactor.callLater(0, _cl, d)
            return d
        else: 
            return defer.succeed(True)
    
    def _get_local_config_message_items(self):
        """
        Returns a dict with keys 'audio' and 'video' to send to remote peer.
        @rettype: dict
        """
        return {
            "video": {
                "codec": self.config.video_codec,
                "bitrate": self.config.video_bitrate, # float Mbps
                "port": self.recv_video_port,
                "aspect_ratio": self.config.video_aspect_ratio, 
                "capture_size": self.config.video_capture_size 
                },
            "audio": {
                "codec": self.config.audio_codec,
                "numchannels": self.config.audio_channels,
                "port": self.recv_audio_port
                }
            }

    def send_invite(self):
        if not self.devices["jackd_is_running"]:
            dialogs.ErrorDialog.create("Impossible to invite a contact to start streaming, jackd is not running.", parent=self.gui.main_window)
            return
            
        self.allocate_ports()
        if self.streamer_manager.is_busy():
            dialogs.ErrorDialog.create("Impossible to invite a contact to start streaming. A streaming session is already in progress.", parent=self.gui.main_window)
        else:
            # UPDATE when initiating session
            self.gui._gather_configuration()
        msg = {
            "msg":"INVITE",
            "protocol": self.protocol_version,
            "sid":0, 
            "please_send_to_port": self.config.negotiation_port, # FIXME: rename to listening_port
            }
        msg.update(self._get_local_config_message_items())
        contact = self.address_book.selected_contact
        port = contact["port"]
        ip = contact["address"]

        def _on_connected(proto):
            self.gui._schedule_offerer_invite_timeout()
            self.client.send(msg)
            return proto
        def _on_error(reason):
            print ("error trying to connect to %s:%s : %s" % (ip, port, reason))
            self.gui.calling_dialog.hide()
            return None
           
        print ("sending %s to %s:%s" % (msg, ip, port))
        deferred = self.client.connect(ip, port)
        deferred.addCallback(_on_connected).addErrback(_on_error)
        self.gui.calling_dialog.show()
        # window will be hidden when we receive ACCEPT or REFUSE, or when we cancel
    
    def send_accept(self, addr):
        # UPDATE config once we accept the invitie
        self.gui._gather_configuration()
        self.allocate_ports()
        msg = {
            "msg":"ACCEPT", 
            "protocol": self.protocol_version,
            "sid":0,
            }
        msg.update(self._get_local_config_message_items())
        self.client.send(msg)
        
    def send_ack(self):
        self.client.send({"msg":"ACK", "sid":0})

    def send_bye(self):
        """
        Sends BYE
        BYE stops the streaming on the remote host.
        """
        self.client.send({"msg":"BYE", "sid":0})
    
    def send_cancel_and_disconnect(self):
        """
        Sends CANCEL
        CANCEL cancels the invite on the remote host.
        """
        self.client.send({"msg":"CANCEL", "sid":0})
        self.client.disconnect()
    
    def send_refuse_and_disconnect(self):
        """
        Sends REFUSE 
        REFUSE tells the offerer we can't have a session.
        """
        self.client.send({"msg":"REFUSE", "sid":0})
        self.client.disconnect()

    # ------------------- streaming events handlers ----------------
    
    def on_streamer_state_changed(self, streamer, new_state):
        """
        Slot for scenic.streamer.StreamerManager.state_changed_signal
        """
        if new_state in [process.STATE_STOPPED]:
            if not self.got_bye:
                # got_bye means our peer sent us a BYE, so we shouldn't send one back 
                print("Local StreamerManager stopped. Sending BYE")
                self.send_bye()
            
    def on_connection_error(self, err, msg):
        # XXX
        self.gui.hide_calling_dialog(msg)
        text = _("%s: %s") % (str(err), str(msg))
        dialogs.ErrorDialog.create(text, parent=self.gui.main_window)
