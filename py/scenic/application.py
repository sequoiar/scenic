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

Summary of events
=================
 - At startup, the config file is read.
 - Next, we need to disable the interactivity of widgets
 - We then set the widget's values, and make them interactive again.
 - Some widgets do things when they are changed. Some toggle the sensitivity (gray out) of some other widgets, whereas some other will call external processes to change video and audio devices properties. 
 - When the user decides to start a streaming session, the value of all widgets is read, and we save those values in the config file. 
 - Next, the offerer connects to the answerer and sends it a dict of its configuration options, serialized in JSON. 
 - If the answerer accepts, he sends back its options. Each peer decides which port he listens to for each service. (audio, video, MIDI streams) 
 - Next, the streamer manager store a summary of both peer's options in a large dict. That's where we check which processes we will need to start. 
 - The streamer manager starts the processes. 
 - Some processes' output might be checked for error messages, which can be shown to the user in error dialogs.
 - As soon as one process dies or the user wants to stop the streaming session, we kill all streamer processes and send "BYE" to the other peer. The other peer also stops all its streamer processes.
 - When a session is in progress, many widgets are grayed out. It is not the case when there is no session in progress.
 - When we quit, the state of each widget is saved to the config file.

The preview
===========
The preview works a little like the streamer manager, but is simpler since it does not involve a remote peer. It is a process that is started. When it dies, we toggle the start of the start/stop button. 

Negotiation sequence
====================
 - {"msg":"INVITE", "videoport":10000, "audioport":11000, "sid":0, "please_send_to_port":999}
  - Each peer ask for ports to send to, and of media settings as well. "video": [{"port":10000, "codec":"mpeg4", "bitrate":3000000}]
 - {"msg":"ACCEPT", "videoport":10000, "audioport":11000, "sid":0}
 - {"msg":"REFUSE", "sid":0}
 - {"msg":"CANCEL", "sid":0}
 - {"msg":"ACK", "sid":0}
 - {"msg":"BYE", "sid":0}
 - {"msg":"OK", "sid":0}

Devices names
=============
Identifying the devices is a difficult task. The users prefers to see the name of the device, not its number. That's what we show to the user and keep in the state saving. That makes it easier to identify them when there number changes. 

For example, a given V4L2 video device can be mounted as /dev/video0 once and as /dev/video1 at an other time. Same for MIDI devices. 

But what if we have two devices with same name? Here are two examples:

MIDI example
------------
 - M Audio Delta 1010LT MIDI (2)
 - USB Oxygen 8 v2 MIDI 1 (3)
 - USB Oxygen 8 v2 MIDI 1 (5)

V4L2 example
------------
 - BT878 video (Osprey 210/220/230 (/dev/video0)
 - BT878 video (Osprey 210/220/230 (/dev/video1)
 - UVC Camera (046d:0990) (/dev/video2)

It's nice to show the device number/identifier to the user. In the worst case, the user can test the device to see if it's the right one or not. 

So, our choice is to store both the name of the device and its number in the combo box widget and in the state saving. When we load the device name and number from the config file, we first check for the device with that name and number. If it does not exist, we try to find the first device with that name that we can find. If it does not exist, it defaults to the first choice in the list of devices of that kind. 
"""
import os
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
from scenic.devices import midi
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
        # ----------- MISC --------------
        self.email_info = "scenic@sat.qc.ca" 
        # ----------- AUDIO --------------
        self.audio_send_enabled = True
        self.audio_recv_enabled = True
        self.audio_video_synchronized = True # we configure what we receive
        self.audio_source = "jackaudiosrc"
        self.audio_sink = "jackaudiosink"
        self.audio_codec = "raw"
        self.audio_channels = 2
        self.audio_input_buffer = 15
        self.audio_output_buffer = 15
        self.audio_jitterbuffer = 75
        # ------------- VIDEO -------------
        self.video_send_enabled = True
        self.video_recv_enabled = True
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
        # ----------- MIDI ----------------
        self.midi_recv_enabled = False
        self.midi_send_enabled = False
        self.midi_input_device = "" # ID and name
        self.midi_output_device = "" # ID and name
        self.midi_jitterbuffer = 10 # ms
        
        # Done with the configuration entries.
        config_file = 'configuration.json'
        config_dir = os.path.expanduser("~/.scenic")
        config_file_path = os.path.join(config_dir, config_file)
        saving.ConfigStateSaving.__init__(self, config_file_path)

def _format_device_name_and_identifier(name, identifier):
    """
    Formats a device name to show it to the user and save it to the state saving.
    
    If you change the format here, change the parsing in the device name parsing method.
    See _parse_device_name_and_identifier.
    @param name: Name of the device.
    @param identifier: Identifier of the device.
    @rtype: str
    """
    #@param midi_device_dict: Dict of MIDI device info, as given by the MIDI device driver.
    #@type midi_device_dict: dict
    #@rtype: str
    return "%s (%s)" % (name, identifier)

def _parse_device_name_and_identifier(formatted_name):
    """
    Splits a device name and identifier.

    See _format_device_name_and_identifier.
    @param formatted_name: Name and identifier as shown to the user.
    @rtype: tuple
    @return: Name and identifier of the device. Both strings.
    """
    tokens = formatted_name.split("(") # split tokens
    number = tokens[-1].split(")")[0] # last token without closing parenthesis
    name = "(".join(tokens[0 : -1]).strip() # all tokens except last
    return name, number

class Application(object):
    """
    Main class of the application.

    The devices attributes is a very interesting dict. See the source code.
    """
    def __init__(self, kiosk_mode=False, fullscreen=False, log_file_name=None, enable_debug=False):
        self.config = Config()
        self.enable_debug = enable_debug
        self.log_file_name = log_file_name
        self.recv_video_port = None
        self.recv_audio_port = None
        self.recv_midi_port = None
        self.remote_config = {} # dict
        self.ports_allocator = ports.PortsAllocator()
        self.address_book = saving.AddressBook()
        self.streamer_manager = StreamerManager(self)
        self.streamer_manager.state_changed_signal.connect(self.on_streamer_state_changed) # XXX
        self._is_negotiating = False
        print("Starting SIC server on port %s" % (self.config.negotiation_port)) 
        self.server = communication.Server(self, self.config.negotiation_port) # XXX
        self.client = communication.Client()
        self.client.connection_error_signal.connect(self.on_connection_error)
        self.protocol_version = "SIC 0.1"
        self.got_bye = False 
        # starting the GUI:
        internationalization.setup_i18n()
        self.gui = gui.Gui(self, kiosk_mode=kiosk_mode, fullscreen=fullscreen, enable_debug=self.enable_debug)
        self.devices = {
            "x11_displays": [], # list of dicts
            "cameras": {}, # dict of dicts (only V4L2 cameras for now)
            #"dc_cameras": [], # list of dicts
            "xvideo_is_present": False, # bool
            "jackd_is_running": False,
            "jackd_is_zombie": False,
            "jack_servers": [], # list of dicts
            "midi_input_devices": [],
            "midi_output_devices": [],
            }
        self._jackd_watch_task = task.LoopingCall(self._poll_jackd)
        reactor.callLater(0, self._start_the_application)
    
    def format_midi_device_name(self, midi_device_dict):
        """
        Formats a MIDI device name to show it to the user and save it to the state saving.
        @param midi_device_dict: Dict of MIDI device info, as given by the MIDI device driver.
        @type midi_device_dict: dict
        @rtype: str
        """
        return _format_device_name_and_identifier(midi_device_dict["name"], str(midi_device_dict["number"]))
    
    def on_connection_error(self, err, mess):
        """
        Called by the communication.Client in case of an error.
        """
        self._is_negotiating = False #important

    def format_v4l2_device_name(self, device_dict):
        print "formatting v4l2 device name", device_dict
        return _format_device_name_and_identifier(device_dict["card"], device_dict["name"])

    def parse_v4l2_device_name(self, formatted_name):
        ret = None
        name, identifier = _parse_device_name_and_identifier(formatted_name)
        key = "cameras"
        # try to find a device that matches both name and identifier
        for dev in self.devices[key].values():
            if dev["card"] == name and dev["name"] == identifier:
                ret = dev
        if ret is None:
            # try to find a device that matches only the name
            for dev in self.devices[key].values():
                if dev["card"] == name:
                    ret = dev
        return ret
    
    def parse_midi_device_name(self, formatted_name, is_input=False):
        """
        Parses a MIDI device name shown to the user, and return the device's number, or None if it is not found.
        
        It will not be found in the system if it doesn't exist anymore.        
        See format_midi_device_name.

        @param formatted_name: Name of the device, as given by the format_midi_device_name method.
        @type formatted_name: str
        @param is_input: True if it's an input device, False for an output device.
        @type is_input: bool
        @rtype: dict
        """
        ret = None
        name, number = _parse_device_name_and_identifier(formatted_name)
        if is_input:
            key = "midi_input_devices"
        else:
            key = "midi_output_devices"
        # try to find a device that matches both name and number
        for dev in self.devices[key]:
            if dev["name"] == name and dev["number"] == int(number):
                ret = dev
        # try to find a device that matches only the name
        if ret is None:
            for dev in self.devices[key]:
                if dev["name"] == name:
                    ret = dev
        return ret

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
            self.poll_camera_devices() # we need to do it once more, to update the list of possible image size according to the selected video device
        deferred_list = defer.DeferredList([
            self.poll_x11_devices(), 
            self.poll_xvideo_extension(),
            self.poll_camera_devices(), 
            self.poll_midi_devices()
            ])
        deferred_list.addCallback(_callback)

    def poll_midi_devices(self):
        """
        Called once at startup, and then the GUI can call it.
        @rtype: L{Deferred}
        """
        deferred = midi.list_midi_devices()
        def _callback(midi_devices):
            input_devices = []
            output_devices = []
            for device in midi_devices:
                if device["is_input"]:
                    input_devices.append(device)
                else:
                    output_devices.append(device)
            self.devices["midi_input_devices"] = input_devices
            self.devices["midi_output_devices"] = output_devices
            print("MIDI inputs: %s" % (input_devices))
            print("MIDI outputs: %s" % (output_devices))
            self.gui.update_midi_devices()
        deferred.addCallback(_callback)
        return deferred

    def poll_x11_devices(self):
        """
        Called once at startup, and then the GUI can call it.
        Calls gui.update_x11_devices.
        @rtype: Deferred
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
        @rtype: Deferred
        """
        deferred = cameras.list_cameras()
        toggle_size_sensitivity = self.gui.video_capture_size_widget.get_property("sensitive")
        def _callback(cameras):
            self.devices["cameras"] = cameras
            print("cameras: %s" % (cameras))
            self.gui.update_camera_devices()
            if toggle_size_sensitivity:
                self.gui.video_capture_size_widget.set_sensitive(True)
            print("Done polling cameras. Setting video_capture_size widget sensitive to true.")
            return cameras
        def _errback(reason):
            if toggle_size_sensitivity:
                self.gui.video_capture_size_widget.set_sensitive(True)
            print("Setting video_capture_size widget sensitive to true")
            return reason
        if toggle_size_sensitivity:
            self.gui.video_capture_size_widget.set_sensitive(False)
        deferred.addCallback(_callback)
        print("Setting video_capture_size widget sensitive to false")
        deferred.addErrback(_errback)
        return deferred

    def poll_xvideo_extension(self):
        """
        Called once at startup, and then the GUI can call it.
        @rtype: Deferred
        """
        deferred = x11.xvideo_extension_is_present()
        def _callback(xvideo_is_present):
            self.devices["xvideo_is_present"] = xvideo_is_present
            if not xvideo_is_present:
                msg = _("It seems like the xvideo extension is not present. Video display is not possible.")
                print(msg)
                dialogs.ErrorDialog.create(msg, parent=self.gui.main_window)
            return xvideo_is_present
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
        @rtype: L{DeferredList}
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
        Checks if we are currently streaming with a peer or not.
        @rtype: bool
        """
        return self.streamer_manager.is_busy()

    def has_negotiation_in_progress(self):
        """
        Checks if we are currently negotiating  with a peer or not.
        @rtype: bool
        """
        return self._is_negotiating
    
    # -------------------- streamer ports -----------------
    def prepare_before_rtp_stream(self):
        #TODO: return a Deferred
        self.gui.close_preview_if_running() # TODO: use its deferred
        self.save_configuration()
        self._allocate_ports()
        
    def cleanup_after_rtp_stream(self):
        #FIXME: is this useful at all?
        self._free_ports()
    
    def _allocate_ports(self):
        # TODO: start_session
        self.recv_video_port = self.ports_allocator.allocate()
        self.recv_audio_port = self.ports_allocator.allocate()
        self.recv_midi_port = self.ports_allocator.allocate()

    def _free_ports(self):
        # TODO: stop_session
        for port in [self.recv_video_port, self.recv_audio_port, self.recv_midi_port]:
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
        @rtype: bool
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
            self._is_negotiating = False
        
        if contact is not None:
            invited_by = contact["name"]
        if self.has_negotiation_in_progress():
            print "REFUSING an INVITE, since we are already negotiating with some peer."
            _simply_refuse() # TODO: add reason
            return
        self._is_negotiating = True
        
        def _check_cb(result):
            if not result:
                _simply_refuse() # TODO: add reason param: Technical problems.
            else:
                # FIXME: the copy of dict should be more straightforward.
                self.remote_config = {
                    "audio": message["audio"],
                    "video": message["video"],
                    "midi": message["midi"]
                    }
                connected_deferred = self.client.connect(addr, message["please_send_to_port"])
                if contact is not None and contact["auto_accept"]:
                    print("Contact %s is on auto_accept. Accepting." % (invited_by))
                    def _connected_cb(proto):
                        self.send_accept(addr)
                    connected_deferred.addCallback(_connected_cb)
                else:
                    text = _("<b><big>%(invited_by)s is inviting you.</big></b>\n\nDo you accept?" % {"invited_by": invited_by})
                    dialog_deferred = self.gui.show_invited_dialog(text)
                    dialog_deferred.addCallback(_on_contact_request_dialog_response)
        
        flight_check_deferred = self.check_if_ready_to_stream(role="answerer")
        flight_check_deferred.addCallback(_check_cb)
    
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
        self._is_negotiating = False
        # If had previously sent ACCEPT and receive CANCEL, abort the session.
        if self.get_last_message_sent() == "ACCEPT":
            self.cleanup_after_rtp_stream()
        contact = self._get_contact_by_addr(addr)
        contact_name = ""
        if contact is not None:
            contact_name = contact["name"]
        txt = _("Contact %(name)s invited you but cancelled his invitation.") % {"name": contact_name}
        # Turning the reason into readable i18n str.
        if message.has_key("reason"):
            reason = message["reason"]
            if reason == communication.CANCEL_REASON_CANCELLED:
                txt += "\n\n" + _("The peer cancelled the invitation.")
        self.client.disconnect()
        self.gui.invited_dialog.hide()
        dialogs.ErrorDialog.create(txt, parent=self.gui.main_window)

    def handle_accept(self, message, addr):
        self._is_negotiating = False
        if self.get_last_message_sent() == "CANCEL":
            self.send_bye() # If got ACCEPT, but had sent CANCEL, send BYE.
        else:
            self._check_protocol_version(message)
            self.got_bye = False
            # TODO: Use session to contain settings and ports
            self.gui.hide_calling_dialog()
            # FIXME: the copy of dict should be more straightforward
            self.remote_config = {
                "audio": message["audio"],
                "video": message["video"],
                "midi": message["midi"]
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
        self._is_negotiating = False
        self.gui.hide_calling_dialog()
        self._free_ports()
        text = _("The contact refused to stream with you.\n\nIt may be caused by a ongoing session with an other peer or by technical problems.")
        dialogs.ErrorDialog.create(text, parent=self.gui.main_window)

    def handle_ack(self, addr):
        """
        Got ACK
        """
        self._is_negotiating = False
        print("Got ACK. Starting streamers as answerer.")
        self.start_streamers(addr)

    def handle_bye(self):
        """
        Got BYE
        """
        self._is_negotiating = False
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
        self._is_negotiating = False
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

    def _check_if_all_disabled(self):
        """
        Checks if al the streams are disabled.
        @rettype: bool
        """
        send_video = self.remote_config["video"]["recv_enabled"] and self.config.video_send_enabled
        send_audio = self.remote_config["audio"]["recv_enabled"] and self.config.audio_send_enabled
        recv_audio = self.remote_config["audio"]["send_enabled"] and self.config.audio_recv_enabled
        recv_video = self.remote_config["video"]["send_enabled"] and self.config.video_recv_enabled
        recv_midi = self.remote_config["midi"]["send_enabled"] and self.config.midi_recv_enabled
        send_midi = self.remote_config["midi"]["recv_enabled"] and self.config.midi_send_enabled
        return not send_video and not send_audio and not recv_audio and not recv_video and not send_midi and not recv_midi
    
    def start_streamers(self, addr):
        if self._check_if_all_disabled():
            error_msg = _("Cannot start streaming if all the streams are disabled.")
            dialogs.ErrorDialog.create(error_msg, parent=self.gui.main_window)
            self.send_bye()
            self.stop_streamers()
        else:
            self.streamer_manager.start(addr)
        self._is_negotiating = False

    def stop_streamers(self):
        # TODO: return a deferred. 
        self.streamer_manager.stop()
        self._is_negotiating = False

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
        @rtype: L{Deferred}
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
        Returns a dict with keys 'audio', 'midi' and 'video' to send to remote peer.
        @rtype: dict
        """
        return {
            "video": {
                "send_enabled": self.config.video_send_enabled,
                "recv_enabled": self.config.video_recv_enabled,
                "codec": self.config.video_codec,
                "bitrate": self.config.video_bitrate, # float Mbps
                "port": self.recv_video_port,
                "aspect_ratio": self.config.video_aspect_ratio, 
                "capture_size": self.config.video_capture_size 
                },
            "audio": {
                "synchronized": self.config.audio_video_synchronized,
                "send_enabled": self.config.audio_send_enabled,
                "recv_enabled": self.config.audio_recv_enabled,
                "codec": self.config.audio_codec,
                "numchannels": self.config.audio_channels,
                "port": self.recv_audio_port
                },
            "midi": {
                "port": self.recv_midi_port,
                "recv_enabled": self.config.midi_recv_enabled,
                "send_enabled": self.config.midi_send_enabled
                }
            }

    def check_if_ready_to_stream(self, role="offerer"):
        """
        Does the flight check, checking if ready to stream.
        
        Checks if ready to stream. 
        Will pop up error dialog if there are errors.
        Calls the deferred with a result that is True of False.
        @rtype: L{Deferred}
        @param role: Either "offerer" or "answerer".
        """
        #TODO: poll X11 devices
        #TODO: poll xv extension
        deferred = defer.Deferred()
        def _callback(result):
            # callback for the deferred list created below.
            # calls the deferred's callback
            if role == "offerer":
                error_msg = _("Impossible to invite a contact to start streaming.")
            elif role == "answerer":
                error_msg = _("Impossible to accept an invitation to stream.")
            else:
                raise RuntimeError("Invalid role value : %s" % (role))
            
            x11_displays = [display["name"] for display in self.devices["x11_displays"]]
            midi_input_devices = [device["name"] for device in self.devices["midi_input_devices"]]
            midi_output_devices = [device["name"] for device in self.devices["midi_output_devices"]]
            cameras = self.devices["cameras"].keys()
                
            if self.config.video_display not in x11_displays: #TODO: do not test if not receiving video
                dialogs.ErrorDialog.create(error_msg + "\n\n" + _("The X11 display %(display)s disappeared!") % {"display": self.config.video_display}, parent=self.gui.main_window) # not very likely to happen !
                return deferred.callback(False)
            
            elif self.config.video_source == "v4l2src" and self.parse_v4l2_device_name(self.config.video_device) is None: #TODO: do not test if not sending video
                dialogs.ErrorDialog.create(error_msg + "\n\n" + _("The video source %(camera)s disappeared!") % {"camera": self.config.video_source}, parent=self.gui.main_window) 
                return deferred.callback(False)
                
            elif not self.devices["jackd_is_running"] and self.config.audio_send_enabled or self.config.audio_recv_enabled:
                # TODO: Actually poll jackd right now.
                dialogs.ErrorDialog.create(error_msg + "\n\n" + _("JACK is not running."), parent=self.gui.main_window)
                return deferred.callback(False)
            
            elif self.streamer_manager.is_busy():
                dialogs.ErrorDialog.create(error_msg + "\n\n" + _("A streaming session is already in progress."), parent=self.gui.main_window)
                deferred.callback(False)
            
            elif self.config.midi_recv_enabled and self.parse_midi_device_name(self.config.midi_output_device, is_input=False) is None:
                dialogs.ErrorDialog.create(error_msg + "\n\n" + _("The MIDI output device %(device)s disappeared!") % {"device": self.config.midi_output_device}, parent=self.gui.main_window)
                deferred.callback(False)
            
            elif self.config.midi_send_enabled and self.parse_midi_device_name(self.config.midi_input_device, is_input=True) is None:
                dialogs.ErrorDialog.create(error_msg + "\n\n" + _("The MIDI input device %(device)s disappeared!") % {"device": self.config.midi_input_device}, parent=self.gui.main_window)
                deferred.callback(False)
            
            # "cameras": {}, # dict of dicts (only V4L2 cameras for now)
            elif not self.devices["xvideo_is_present"] and self.config.video_recv_enabled:
                dialogs.ErrorDialog.create(error_msg + "\n\n" + _("The X video extension is not present."), parent=self.gui.main_window)
                deferred.callback(False)
            
            else:
                deferred.callback(True)
        
        deferred_list = defer.DeferredList([
            self.poll_x11_devices(), 
            self.poll_midi_devices(), 
            self.poll_xvideo_extension(),
            self.poll_camera_devices()
            ])
        self._poll_jackd() # does not return a deferred for now... called in a looping call.
        deferred_list.addCallback(_callback)
        return deferred

    def send_invite(self):
        """
        Does the flight check. If OK, send an INVITE.
        """
        self._is_negotiating = True
        contact = self.address_book.get_currently_selected_contact()
        if contact is None:
            dialogs.ErrorDialog.create(_("You must select a contact to invite."), parent=self.gui.main_window)
            return  # important
        else:
            ip = contact["address"]
            
        def _check_cb(result):
            #TODO: use the Deferred it will return
            if result:
                self.prepare_before_rtp_stream()
                msg = {
                    "msg":"INVITE",
                    "protocol": self.protocol_version,
                    "sid":0, 
                    "please_send_to_port": self.config.negotiation_port, # FIXME: rename to listening_port
                    }
                msg.update(self._get_local_config_message_items())
                port = self.config.negotiation_port
                
                def _on_connected(proto):
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
                    self._is_negotiating = False
                    return None
                   
                print("sending %s to %s:%s" % (msg, ip, port))
                deferred = self.client.connect(ip, port)
                deferred.addCallback(_on_connected).addErrback(_on_error)
                self.gui.show_calling_dialog()
                # window will be hidden when we receive ACCEPT or REFUSE, or when we cancel
            else:
                print("Cannot send INVITE.")

        check_deferred = self.check_if_ready_to_stream(role="offerer")
        check_deferred.addCallback(_check_cb)
    
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
        self._is_negotiating = False

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
        self._is_negotiating = False
    
    def send_cancel_and_disconnect(self, reason=""):
        """
        Sends CANCEL
        CANCEL cancels the invite on the remote host.
        """
        #TODO: add reason argument.
        #CANCEL_REASON_CANCELLED = "cancelled"
        if self.client.is_connected():
            self.client.send({"msg":"CANCEL", "reason": reason, "sid":0})
            self.client.disconnect()
        self.cleanup_after_rtp_stream()
        self._is_negotiating = False
    
    def send_refuse_and_disconnect(self):
        """
        Sends REFUSE 
        REFUSE tells the offerer we can't have a session.
        """
        self.client.send({"msg":"REFUSE", "sid":0})
        self.client.disconnect()
        self._is_negotiating = False

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
        elif new_state == process.STATE_RUNNING:
            self.gui.write_info_in_debug_tab()
            
    #def on_connection_error(self, err, msg):
    #    """
    #    @param err: Exception message.
    #    @param msg: Legible message.
    #    """
    #    self.gui.hide_calling_dialog()
    #    text = _("Connection error: %(message)s\n%(error)s") % {"error": err, "message": msg}
    #    dialogs.ErrorDialog.create(text, parent=self.gui.main_window)

