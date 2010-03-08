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
from scenic import internationalization
_ = internationalization._


class Config(saving.ConfigStateSaving):
    """
    Configuration for the application.
    """
    def __init__(self):
        # Default values
        self.negotiation_port = 17446 # receiving TCP (SIC) messages on it.
        self.smtpserver = "smtp.sat.qc.ca"
        self.email_info = "scenic@sat.qc.ca"
        self.audio_source = "jackaudiosrc"
        self.audio_sink = "jackaudiosink"
        self.audio_codec = "raw"
        self.audio_channels = 2
        self.video_source = "v4l2src"
        self.video_device = "/dev/video0"
        self.video_deinterlace = False
        self.video_input = 0
        self.video_standard = "ntsc"
        self.video_sink = "xvimagesink"
        self.video_codec = "mpeg4"
        self.video_display = ":0.0"
        self.video_fullscreen = False
        self.video_capture_size = "640x480"
        self.preview_in_window = False
        #video_window_size = "640x480"
        self.video_aspect_ratio = "4:3" 
        self.confirm_quit = True
        #self.theme = "Darklooks"
        self.video_bitrate = 3.0
        self.video_jitterbuffer = 75
        
        # Done with the configuration entries.
        config_file = 'configuration.json'
        config_dir = os.path.expanduser("~/.scenic")
        config_file_path = os.path.join(config_dir, config_file)
        saving.ConfigStateSaving.__init__(self, config_file_path)

class Application(object):
    """
    Main class of the application.

    The devices attributes is a very interesting dict. See the source code.
    """
    def __init__(self, kiosk_mode=False, fullscreen=False, log_file_name=None):
        self.config = Config()
        self.log_file_name = log_file_name
        self.recv_video_port = None
        self.recv_audio_port = None
        self.remote_config = {} # dict
        self.ports_allocator = ports.PortsAllocator()
        self.address_book = saving.AddressBook()
        self.streamer_manager = StreamerManager(self)
        self.streamer_manager.state_changed_signal.connect(self.on_streamer_state_changed) # XXX
        print("Starting SIC server on port %s" % (self.config.negotiation_port)) 
        self.server = communication.Server(self, self.config.negotiation_port) # XXX
        self.client = communication.Client()
        #self.client.connection_error_signal.connect(self.on_connection_error)
        self.protocol_version = "SIC 0.1"
        self.got_bye = False 
        # starting the GUI:
        internationalization.setup_i18n()
        self.gui = gui.Gui(self, kiosk_mode=kiosk_mode, fullscreen=fullscreen)
        self.devices = {
            "x11_displays": [], # list of dicts
            "cameras": {}, # dict of dicts (only V4L2 cameras for now)
            #"dc_cameras": [], # list of dicts
            "xvideo_is_present": False, # bool
            "jackd_is_running": False,
            "jackd_is_zombie": False,
            "jack_servers": [] # list of dicts
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
            deferred = dialogs.ErrorDialog.create(_("Is another Scenic running? Cannot bind to port %(port)d") % {"port": self.config.negotiation_port}, parent=self.gui.main_window)
            deferred.addCallback(_cb)
            return
        # Devices: JACKD (every 5 seconds)
        self._jackd_watch_task.start(5, now=True)
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
                msg = _("It seems like the xvideo extension is not present. Video display is not possible.")
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
            jack_servers = jackd.jackd_get_infos() # returns a list of dicts
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
        if self.devices["jackd_is_running"] != is_running:
            print("Jackd server changed state: %s" % (jack_servers))
        self.devices["jackd_is_running"] = is_running
        self.devices["jackd_is_zombie"] = is_zombie
        self.devices["jack_servers"] = jack_servers
        self.gui.update_jackd_status()
    
    def before_shutdown(self):
        """
        Last things done before quitting.
        @rettype: L{DeferredList}
        """
        deferred = defer.Deferred()
        print("The application is shutting down.")
        # TODO: stop streamers
        self.save_configuration()
        if self.client.is_connected():
            if not self.got_bye:
                self.send_bye() # returns None
                self.stop_streamers() # returns None
        def _cb(result):
            print "done quitting."
            deferred.callback(True)
        def _later():
            d2 = self.disconnect_client()
            d2.addCallback(_cb)
            print('stopping server')
        reactor.callLater(0.1, _later)
        d1 = self.server.close()
        d2 = self.gui.close_preview_if_running()
        return defer.DeferredList([deferred, d1, d2])
        
    # ------------------------- session occuring -------------
    def has_session(self):
        """
        @rettype: bool
        """
        return self.streamer_manager.is_busy()
    # -------------------- streamer ports -----------------
    def prepare_before_rtp_stream(self):
        #TODO: return a Deferred
        self.gui.close_preview_if_running() # TODO: use its deferred
        self.save_configuration()
        self._allocate_ports()
        
    def cleanup_after_rtp_stream(self):
        self._free_ports()
    
    def _allocate_ports(self):
        # TODO: start_session
        self.recv_video_port = self.ports_allocator.allocate()
        self.recv_audio_port = self.ports_allocator.allocate()

    def _free_ports(self):
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
        """
        handles the INVITE message. 
        Refuses if : 
         * jackd is not running
         * We already just got an INVITE and didn't answer yet.
        """
        self.got_bye = False
        self._check_protocol_version(message)
        
        def _on_contact_request_dialog_response(response):
            """
            User is accepting or declining an offer.
            @param result: Answer to the dialog.
            """
            if response:
                self.send_accept(addr)
            else:
                self.send_refuse_and_disconnect() 
        # check if the contact is in the addressbook
        contact = self._get_contact_by_addr(addr)
        invited_by = addr
        send_to_port = message["please_send_to_port"]

        def _simply_refuse():
            communication.connect_send_and_disconnect(addr, send_to_port, {'msg':'REFUSE', 'sid':0})
        
        if contact is not None:
            invited_by = contact["name"]

        if self.get_last_message_received() == "INVITE" and self.get_last_message_sent() != "REFUSE": # FIXME: does that cover all cases?
            _simply_refuse()
            print("REFUSED an INVITE since we already got one from someone else.")
            return
            
        if self.streamer_manager.is_busy():
            _simply_refuse()
            print("Refused invitation: we are busy.")
            return 
        
        if not self.devices["jackd_is_running"]:
            _simply_refuse()
            dialogs.ErrorDialog.create(_("Refused invitation: jack is not running."), parent=self.gui.main_window)
            return
        
        else:
            self.remote_config = {
                "audio": message["audio"],
                "video": message["video"]
                }
            connected_deferred = self.client.connect(addr, message["please_send_to_port"])
            if contact is not None:
                if contact["auto_accept"]:
                    print("Contact %s is on auto_accept. Accepting." % (invited_by))
                    def _connected_cb(proto):
                        self.send_accept(addr)
                    connected_deferred.addCallback(_connected_cb)
                    return # important
            text = _("<b><big>%(invited_by)s is inviting you.</big></b>\n\nDo you accept the connection?" % {"invited_by": invited_by})
            dialog_deferred = self.gui.show_invited_dialog(text)
            dialog_deferred.addCallback(_on_contact_request_dialog_response)
    
    def _get_contact_by_addr(self, addr):
        """
        Returns a contact dict or None if not in the addressbook.
        """
        ret = None
        for contact in self.address_book.contact_list:
            if contact["address"] == addr:
                ret = contact
                break
        return ret
    
    def handle_cancel(self, message, addr):
        # If had previously sent ACCEPT and receive CANCEL, abort the session.
        if self.get_last_message_sent() == "ACCEPT":
            self.cleanup_after_rtp_stream()
        contact = self._get_contact_by_addr(addr)
        contact_name = ""
        if contact is not None:
            contact_name = contact["name"]
        txt = _("Contact %(name) invited you but cancelled his invitation.") % {"name": contact_name}
        # Turning the reason into readable i18n str.
        if message.has_key("reason"):
            reason = message["reason"]
            if reason == communication.CANCEL_REASON_TIMEOUT:
                txt += "\n\n" + _("The invitation expired.")
            elif reason == communication.CANCEL_REASON_CANCELLED:
                txt += "\n\n" + _("The peer cancelled the invitation.")
        self.client.disconnect()
        self.gui.invited_dialog.hide()
        dialogs.ErrorDialog.create(txt, parent=self.gui.main_window)

    def handle_accept(self, message, addr):
        if self.get_last_message_sent() == "CANCEL":
            self.send_bye() # If got ACCEPT, but had sent CANCEL, send BYE.
        else:
            self._check_protocol_version(message)
            self.got_bye = False
            # TODO: Use session to contain settings and ports
            self.gui.hide_calling_dialog()
            self.remote_config = {
                "audio": message["audio"],
                "video": message["video"]
                }
            if self.streamer_manager.is_busy():
                print("Got ACCEPT but we are busy. This is very strange")
                dialogs.ErrorDialog.create(_("Got an acceptation from a remote peer, but a streaming session is already in progress."), parent=self.gui.main_window)
            else:
                print("Got ACCEPT. Starting streamers as initiator.")
                self.start_streamers(addr)
                self.send_ack()

    def handle_refuse(self):
        """
        Got REFUSE
        """
        self.gui.hide_calling_dialog()
        self._free_ports()
        text = _("The contact refused to stream with you.\n\nIt may be caused by a ongoing session with an other peer or by technical problems.")
        dialogs.ErrorDialog.create(text, parent=self.gui.main_window)

    def handle_ack(self, addr):
        """
        Got ACK
        """
        print("Got ACK. Starting streamers as answerer.")
        self.start_streamers(addr)

    def handle_bye(self):
        """
        Got BYE
        """
        self.got_bye = True
        self.stop_streamers()
        if self.client.is_connected():
            print('disconnecting client and sending BYE')
            self.client.send({"msg":"OK", "sid":0})
            self.disconnect_client()

    def handle_ok(self):
        """
        Got OK
        """
        print("received ok. Everything has an end.")
        print('disconnecting client')
        self.disconnect_client()

    def on_server_receive_command(self, message, addr):
        msg = message["msg"]
        print("Got %s from %s" % (msg, addr))
        # TODO: use prefixedMethods from twisted.
        if msg == "INVITE":
            self.handle_invite(message, addr)
        elif msg == "CANCEL":
            self.handle_cancel(message, addr)
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
            print('WARNING: Unexpected message %s' % (msg))

    # -------------------------- actions on streamer manager --------

    def start_streamers(self, addr):
        self.streamer_manager.start(addr)

    def stop_streamers(self):
        # TODO: return a deferred. 
        self.streamer_manager.stop()

    def on_streamers_stopped(self, addr):
        """
        We call this when all streamers are stopped.
        """
        print("on_streamers_stopped got called")
        self.cleanup_after_rtp_stream()

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
        # not sure why to do it in a call later.
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
            dialogs.ErrorDialog.create(_("Impossible to invite a contact to start streaming, JACK is not running."), parent=self.gui.main_window)
            return
            
        if self.streamer_manager.is_busy():
            dialogs.ErrorDialog.create(_("Impossible to invite a contact to start streaming. A streaming session is already in progress."), parent=self.gui.main_window)
        else:
            #TODO: use the Deferred it will return
            self.prepare_before_rtp_stream()
            msg = {
                "msg":"INVITE",
                "protocol": self.protocol_version,
                "sid":0, 
                "please_send_to_port": self.config.negotiation_port, # FIXME: rename to listening_port
                }
            msg.update(self._get_local_config_message_items())
            contact = self.address_book.selected_contact
            port = self.config.negotiation_port
            ip = contact["address"]
            
            

            def _on_connected(proto):
                self.gui._schedule_inviting_timeout_delayed()
                self.client.send(msg)
                return proto
            def _on_error(reason):
                #FIXME: do we need this error dialog?
                exc_type = type(reason.value)
                if exc_type is error.ConnectionRefusedError:
                    msg = _("Could not invite contact %(name)s. \n\nScenic is not listening on port %(port)d of host %(ip)s.") % {"ip": ip, "name": contact["name"], "port": port}
                elif exc_type is error.ConnectError:
                    msg = _("Could not invite contact %(name)s. \n\nHost %(ip)s is unreachable.") % {"ip": ip, "name": contact["name"]}
                elif exc_type is error.NoRouteError:
                    msg = _("Could not invite contact %(name)s. \n\nHost %(ip)s is unreachable.") % {"ip": ip, "name": contact["name"]}
                else:
                    msg = _("Could not invite contact %(name)s. \n\nError trying to connect to %(ip)s:%(port)s:\n %(reason)s") % {"ip": ip, "name": contact["name"], "port": port, "reason": reason.value}
                print(msg)
                self.gui.hide_calling_dialog()
                dialogs.ErrorDialog.create(msg, parent=self.gui.main_window)
                return None
               
            print("sending %s to %s:%s" % (msg, ip, port))
            deferred = self.client.connect(ip, port)
            deferred.addCallback(_on_connected).addErrback(_on_error)
            self.gui.show_calling_dialog()
            # window will be hidden when we receive ACCEPT or REFUSE, or when we cancel
    
    def send_accept(self, addr):
        # UPDATE config once we accept the invitie
        #TODO: use the Deferred it will return
        self.prepare_before_rtp_stream()
        msg = {
            "msg":"ACCEPT", 
            "protocol": self.protocol_version,
            "sid":0,
            }
        msg.update(self._get_local_config_message_items())
        self.client.send(msg)

    def get_last_message_sent(self):
        return self.client.last_message_sent

    def get_last_message_received(self):
        return self.server.last_message_received
    
    def send_ack(self):
        """
        Sends ACK.
        INVITE, ACCEPT, ACK
        """
        self.client.send({"msg":"ACK", "sid":0})

    def send_bye(self):
        """
        Sends BYE
        BYE stops the streaming on the remote host.
        """
        self.client.send({"msg":"BYE", "sid":0})
    
    def send_cancel_and_disconnect(self, reason=""):
        """
        Sends CANCEL
        CANCEL cancels the invite on the remote host.
        """
        #TODO: add reason argument.
        #CANCEL_REASON_TIMEOUT = "timed out"
        #CANCEL_REASON_CANCELLED = "cancelled"
        if self.client.is_connected():
            self.client.send({"msg":"CANCEL", "reason": reason, "sid":0})
            self.client.disconnect()
        self.cleanup_after_rtp_stream()
    
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
            
    #def on_connection_error(self, err, msg):
    #    """
    #    @param err: Exception message.
    #    @param msg: Legible message.
    #    """
    #    self.gui.hide_calling_dialog()
    #    text = _("Connection error: %(message)s\n%(error)s") % {"error": err, "message": msg}
    #    dialogs.ErrorDialog.create(text, parent=self.gui.main_window)

