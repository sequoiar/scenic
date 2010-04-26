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
Manages local streamer processes.
"""

from scenic import process
from scenic import sig
from scenic import dialogs
from scenic.internationalization import _

class StreamerManager(object):
    """
    Manages local streamer processes.
    """
    def __init__(self, app):
        self.app = app
        self.sender = None 
        self.receiver = None 
        self.extra_sender = None # extra sender/receiver used only if not synchronized
        self.extra_receiver = None
        
        self.midi_receiver = None
        self.midi_sender = None
        self.state = process.STATE_STOPPED
        self.state_changed_signal = sig.Signal()
        # for stats
        self.session_details = None # either None or a big dict
        self.rtcp_stats = None # either None or a big dict
        self.error_messages = None # either None or a big dict

    def _gather_config_to_stream(self, addr):
        """
        Gathers all settings in a big dict.
        
        Useful for feedback to the user.
        """
        contact_name = addr
        contact = self.app._get_contact_by_addr(addr)
        if contact is not None:
            contact_name = contact["name"]
        
        remote_config = self.app.remote_config # FIXME: should the remote config be passed as a param to this method?
        send_width, send_height = self.app.config.video_capture_size.split("x")
        receive_width, receive_height = remote_config["video"]["capture_size"].split("x")
        
        # MIDI
        midi_send_enabled = self.app.config.midi_send_enabled and remote_config["midi"]["recv_enabled"]
        midi_recv_enabled = self.app.config.midi_recv_enabled and remote_config["midi"]["send_enabled"]
        midi_input_device = self.app.config.midi_input_device
        midi_output_device = self.app.config.midi_output_device
        
        audio_jitterbuffer = self.app.config.audio_jitterbuffer
        if self.app.config.audio_video_synchronized and self.app.config.video_recv_enabled:
            audio_jitterbuffer = self.app.config.video_jitterbuffer
        
        print "remote_config:", remote_config
        
        self.session_details = {
            "peer": {
                "address": addr,
                "name": contact_name,
                },
            # ----------------- send ---------------
            "send": {
                "video": {
                    # Decided by both:
                    "enabled": self.app.config.video_send_enabled and remote_config["video"]["recv_enabled"],
                    
                    # Decided locally:
                    "source": self.app.config.video_source,
                    "device": self.app.config.video_device,
                    "width": int(send_width), # int
                    "height": int(send_height), # int
                    "aspect-ratio": self.app.config.video_aspect_ratio,
                    
                    # Decided by remote peer:
                    "port": remote_config["video"]["port"], 
                    "bitrate": remote_config["video"]["bitrate"], 
                    "codec": remote_config["video"]["codec"], 
                },
                
                "audio": {
                    # Decided by both:
                    "enabled": self.app.config.audio_send_enabled and remote_config["audio"]["recv_enabled"],

                    # Decided locally:
                    "source": self.app.config.audio_source,
                    "vumeter-id": self.app.gui.audio_levels_input_socket_id,
                    "buffer": self.app.config.audio_input_buffer,

                    # Decided by remote peer:
                    "numchannels": remote_config["audio"]["numchannels"],
                    "codec": remote_config["audio"]["codec"],
                    "port": remote_config["audio"]["port"], 
                    "synchronized": remote_config["audio"]["synchronized"],
                }, 
                "midi": {
                    "enabled": midi_send_enabled,
                    "input_device": midi_input_device,
                    "port": remote_config["midi"]["port"]
                }
            },
            # -------------------- recv ------------
            "receive": {
                "video": {
                    # Decided by both:
                    "enabled": self.app.config.video_recv_enabled and remote_config["video"]["send_enabled"],

                    # decided locally:
                    "sink": self.app.config.video_sink,
                    "port": str(self.app.recv_video_port), #decided by the app
                    "codec": self.app.config.video_codec,
                    "deinterlace": self.app.config.video_deinterlace, # bool
                    "window-title": "\"From %s\"" % (contact_name), #TODO: i18n
                    "jitterbuffer": self.app.config.video_jitterbuffer, 
                    "fullscreen": self.app.config.video_fullscreen, # bool
                    "display": self.app.config.video_display,
                    "bitrate": self.app.config.video_bitrate, # float
                    
                    # Decided by remote peer:
                    "aspect-ratio": remote_config["video"]["aspect_ratio"],
                    "width": int(receive_width), # int
                    "height": int(receive_height), # int
                },
                "audio": {
                    # Decided by both:
                    "enabled": self.app.config.audio_recv_enabled and remote_config["audio"]["send_enabled"],
                    
                    # decided locally:
                    "numchannels": self.app.config.audio_channels, # int
                    "vumeter-id": self.app.gui.audio_levels_output_socket_id,
                    "codec": self.app.config.audio_codec, 
                    "port": self.app.recv_audio_port,
                    "sink": self.app.config.audio_sink,
                    "buffer": self.app.config.audio_output_buffer,
                    "synchronized": self.app.config.audio_video_synchronized,
                    "jitterbuffer": audio_jitterbuffer
                },
                "midi": {
                    "enabled": midi_recv_enabled,
                    "jitterbuffer": self.app.config.midi_jitterbuffer,
                    "output_device": midi_output_device,
                    "port": str(self.app.recv_midi_port),
                }
            }
        }
        if self.session_details["send"]["video"]["source"] != "v4l2src":
            self.session_details["send"]["video"]["device"] = None
        if self.session_details["send"]["video"]["codec"] == "theora":
            self.session_details["send"]["video"]["bitrate"] = None
        print(str(self.session_details))

    def _prepare_stats_and_errors_dicts(self):
        # setting up
        self.rtcp_stats = {
            "send": {
                "video": {
                    "packets-lost": 0,
                    "packets-sent": 0,
                    "packet-loss-percent": 0.0,
                    "jitter": 0,
                    "bitrate": None,
                    "connected": False
                },
                "audio": {
                    "packets-lost": 0,
                    "packets-sent": 0,
                    "packet-loss-percent": 0.0,
                    "jitter": 0,
                    "bitrate": None,
                    "connected": False
                }
            },
            "receive": {
                "video": {
                    "connected": False,
                    "bitrate": None 
                },
                "audio": {
                    "connected": False,
                    "bitrate": None 
                }
            }
        }
        self.error_messages = {
            "send": [], # list of strings
            "receive": [], # list of strings
            }
        
    def start(self, host):
        """
        Starts the sender and receiver processes.
        
        @param host: str ip addr
        Raises a RuntimeError if a sesison is already in progress.
        """
        if self.state != process.STATE_STOPPED:
            raise RuntimeError("Cannot start streamers since they are %s." % (self.state)) # the programmer has done something wrong if we're here.
        
        self._gather_config_to_stream(host)
        details = self.session_details
        send_video_enabled = self.session_details["send"]["video"]["enabled"]
        send_audio_enabled = self.session_details["send"]["audio"]["enabled"]
        recv_video_enabled = self.session_details["receive"]["video"]["enabled"]
        recv_audio_enabled = self.session_details["receive"]["audio"]["enabled"]
        midi_recv_enabled = self.session_details["receive"]["midi"]["enabled"]
        midi_send_enabled = self.session_details["send"]["midi"]["enabled"]
        extra_send_enabled = not self.session_details["send"]["audio"]["synchronized"] and send_audio_enabled
        extra_recv_enabled = not self.session_details["receive"]["audio"]["synchronized"] and recv_audio_enabled
        normal_recv_enabled = recv_video_enabled or recv_audio_enabled and not extra_recv_enabled # if the self.sender and self.receiver are enabled
        normal_send_enabled = send_video_enabled or send_audio_enabled and not extra_send_enabled
        if not send_audio_enabled and not send_video_enabled and not midi_send_enabled and not midi_recv_enabled and not recv_audio_enabled and not recv_video_enabled:
            raise RuntimeError("Everything is disabled, but the application is trying to start to stream. This should not happen.") # the programmer has done a mistake if we're here.

        print "send_video_enabled", send_video_enabled
        print "send_audio_enabled", send_audio_enabled
        print "recv_video_enabled", recv_video_enabled
        print "recv_audio_enabled", recv_audio_enabled
        print "midi_recv_enabled", midi_recv_enabled 
        print "midi_send_enabled", midi_send_enabled

        # we store the arguments in lists
        milhouse_send_cmd_common = []
        milhouse_send_cmd_audio = []
        milhouse_send_cmd_video = []
        milhouse_recv_cmd_common = []
        milhouse_recv_cmd_audio = []
        milhouse_recv_cmd_video = []

        # ------------------ send ---------------
        # every element in the lists must be strings since we join them .
        # int elements are converted to str.
        milhouse_send_cmd_common.extend([
            "milhouse", 
            '--sender', 
            '--address', details["peer"]["address"]
            ])

        # send video:
        if send_video_enabled:
            milhouse_send_cmd_video.extend([
                '--videosource', details["send"]["video"]["source"],
                '--videocodec', details["send"]["video"]["codec"],
                '--videoport', str(details["send"]["video"]["port"]),
                '--width', str(details["send"]["video"]["width"]), 
                '--height', str(details["send"]["video"]["height"]),
                '--aspect-ratio', str(details["send"]["video"]["aspect-ratio"]),
                ])
            if details["send"]["video"]["source"] == "v4l2src":
                dev = self.app.parse_v4l2_device_name(details["send"]["video"]["device"])
                if dev is None:
                    print "v4l2 device is not found !!!!"
                else:
                    v4l2_dev_name = dev["name"]
                milhouse_send_cmd_video.extend(["--videodevice", v4l2_dev_name])
            if details["send"]["video"]["codec"] != "theora":
                milhouse_send_cmd_video.extend(['--videobitrate', str(int(details["send"]["video"]["bitrate"] * 1000000))])

        # send audio:
        if send_audio_enabled:
            milhouse_send_cmd_audio.extend([
                '--audiosource', details["send"]["audio"]["source"],
                '--numchannels', str(details["send"]["audio"]["numchannels"]),
                '--audiocodec', details["send"]["audio"]["codec"],
                '--audioport', str(details["send"]["audio"]["port"]),
                '--vumeter-id', str(details["send"]["audio"]["vumeter-id"]),
                '--audio-buffer', str(details["send"]["audio"]["buffer"])
                ])

        # ------------------- recv ----------------
        milhouse_recv_cmd_common.extend([
            "milhouse",
            '--receiver', 
            '--address', details["peer"]["address"]
            ])

        # recv video:
        if recv_video_enabled:
            milhouse_recv_cmd_video.extend([
                '--videosink', details["receive"]["video"]["sink"],
                '--videocodec', details["receive"]["video"]["codec"],
                '--videoport', str(details["receive"]["video"]["port"]),
                '--jitterbuffer', str(details["receive"]["video"]["jitterbuffer"]),
                '--width', str(details["receive"]["video"]["width"]),
                '--height', str(details["receive"]["video"]["height"]),
                '--aspect-ratio', details["receive"]["video"]["aspect-ratio"],
                '--window-title', details["receive"]["video"]["window-title"],
                '--display', details["receive"]["video"]["display"],
                ])
            if details["receive"]["video"]["fullscreen"]:
                milhouse_recv_cmd_video.append('--fullscreen')
            if details["receive"]["video"]["deinterlace"]:
                milhouse_recv_cmd_video.append('--deinterlace')

        # recv audio:
        if recv_audio_enabled:
            milhouse_recv_cmd_audio.extend([
                # audio:
                '--audiosink', details["receive"]["audio"]["sink"],
                '--numchannels', str(details["receive"]["audio"]["numchannels"]),
                '--audiocodec', details["receive"]["audio"]["codec"],
                '--audioport', str(details["receive"]["audio"]["port"]),
                '--vumeter-id', str(details["receive"]["audio"]["vumeter-id"]),
                '--audio-buffer', str(details["receive"]["audio"]["buffer"])
                ])

        self._prepare_stats_and_errors_dicts()
        # every element in the lists must be strings since we join them 
        # TODO: not sync
        # TODO: if both disabled, do not start sender/receiver
        # ---- audio/video receiver ----
        if recv_audio_enabled or recv_video_enabled: # receiver(s)
            milhouse_recv_cmd_final = []
            milhouse_recv_cmd_extra = []
            
            milhouse_recv_cmd_final.extend(milhouse_recv_cmd_common)
            milhouse_recv_cmd_extra.extend(milhouse_recv_cmd_common)
            milhouse_recv_cmd_final.extend(milhouse_recv_cmd_video)
            if extra_recv_enabled:
                milhouse_recv_cmd_extra.extend(milhouse_recv_cmd_audio)
                milhouse_recv_cmd_extra.extend(["--jitterbuffer", str(details["receive"]["audio"]["jitterbuffer"])])
            else:
                milhouse_recv_cmd_final.extend(milhouse_recv_cmd_audio)
            try:
                recv_cmd = " ".join(milhouse_recv_cmd_final)
            except TypeError, e:
                print e
                print milhouse_recv_cmd_final
                raise
            try:
                extra_recv_cmd = " ".join(milhouse_recv_cmd_extra)
            except TypeError, e:
                print e
                print milhouse_recv_cmd_extra
                raise
            if normal_recv_enabled:
                self.receiver = process.ProcessManager(command=recv_cmd, identifier="receiver")
                self.receiver.state_changed_signal.connect(self.on_process_state_changed)
                self.receiver.stdout_line_signal.connect(self.on_receiver_stdout_line)
                self.receiver.stderr_line_signal.connect(self.on_receiver_stderr_line)
            if extra_recv_enabled:
                self.extra_receiver = process.ProcessManager(command=extra_recv_cmd, identifier="extra_receiver")
                self.extra_receiver.state_changed_signal.connect(self.on_process_state_changed)
                self.extra_receiver.stdout_line_signal.connect(self.on_receiver_stdout_line)
                self.extra_receiver.stderr_line_signal.connect(self.on_receiver_stderr_line)
        # ---- audio/video sender ----
        if send_audio_enabled or send_video_enabled: # sender(s)
            milhouse_send_cmd_final = []
            milhouse_send_cmd_extra = []
            
            milhouse_send_cmd_final.extend(milhouse_send_cmd_common)
            milhouse_send_cmd_extra.extend(milhouse_send_cmd_common) # only used if we have to
            milhouse_send_cmd_final.extend(milhouse_send_cmd_video)
            if extra_send_enabled:
                milhouse_send_cmd_extra.extend(milhouse_send_cmd_audio)
            else:
                milhouse_send_cmd_final.extend(milhouse_send_cmd_audio)
            send_cmd = " ".join(milhouse_send_cmd_final)
            extra_send_cmd = " ".join(milhouse_send_cmd_extra)
            if normal_send_enabled:
                self.sender = process.ProcessManager(command=send_cmd, identifier="sender")
                self.sender.state_changed_signal.connect(self.on_process_state_changed)
                self.sender.stdout_line_signal.connect(self.on_sender_stdout_line)
                self.sender.stderr_line_signal.connect(self.on_sender_stderr_line)
            if extra_send_enabled:
                self.extra_sender = process.ProcessManager(command=extra_send_cmd, identifier="extra_sender")
                self.extra_sender.state_changed_signal.connect(self.on_process_state_changed)
                self.extra_sender.stdout_line_signal.connect(self.on_sender_stdout_line)
                self.extra_sender.stderr_line_signal.connect(self.on_sender_stderr_line)
                # FIXME: too much code duplication
        
        if midi_recv_enabled:
            midi_out_device = self.app.parse_midi_device_name(details["receive"]["midi"]["output_device"], is_input=False)
            #TODO: check if is None
            midi_recv_args = [
                "midistream",
                "--address", details["peer"]["address"],
                "--receiving-port", str(details["receive"]["midi"]["port"]),
                "--jitter-buffer", str(details["receive"]["midi"]["jitterbuffer"]),
                "--output-device", str(midi_out_device["number"])
                ]
                #"--verbose",
            midi_recv_command = " ".join(midi_recv_args) 
            self.midi_receiver = process.ProcessManager(command=midi_recv_command, identifier="midi_receiver")
            self.midi_receiver.state_changed_signal.connect(self.on_process_state_changed)
            self.midi_receiver.stdout_line_signal.connect(self.on_midi_stdout_line)
            self.midi_receiver.stderr_line_signal.connect(self.on_midi_stderr_line)
        
        if midi_send_enabled:
            midi_in_device = self.app.parse_midi_device_name(details["send"]["midi"]["input_device"], is_input=True)
            #TODO: check if is None
            midi_send_args = [
                "midistream",
                "--address", details["peer"]["address"],
                "--sending-port", str(details["send"]["midi"]["port"]),
                "--input-device", str(midi_in_device["number"])
                ]
                #"--verbose",
            midi_send_command = " ".join(midi_send_args) 
            self.midi_sender = process.ProcessManager(command=midi_send_command, identifier="midi_sender")
            self.midi_sender.state_changed_signal.connect(self.on_process_state_changed)
            self.midi_sender.stdout_line_signal.connect(self.on_midi_stdout_line)
            self.midi_sender.stderr_line_signal.connect(self.on_midi_stderr_line)
        
        # starting
        self._set_state(process.STATE_STARTING)
        if send_audio_enabled or send_video_enabled:
            print "$", send_cmd
            if normal_send_enabled:
                self.sender.start()
            if extra_send_enabled:
                self.extra_sender.start()
        if recv_audio_enabled or recv_video_enabled:
            print "$", recv_cmd
            if normal_recv_enabled:
                self.receiver.start()
            if extra_recv_enabled:
                self.extra_receiver.start()
        if midi_recv_enabled:
            self.midi_receiver.start()
        if midi_send_enabled:
            self.midi_sender.start()

    def get_command_lines(self):
        """
        Returns a list of all the current processes command lines.
        @rettype: list
        """
        ret = []
        for proc in self.get_all_streamer_process_managers():
            ret.append(proc.command)
        return ret

    def on_midi_stdout_line(self, process_manager, line):
        print process_manager.identifier, line

    def on_midi_stderr_line(self, process_manager, line):
        print process_manager.identifier, line

    def on_receiver_stdout_line(self, process_manager, line):
        """
        Handles a new line from our receiver process' stdout
        """
        if "stream connected" in line:
            if "audio" in line:
                self.rtcp_stats["receive"]["audio"]["connected"] = True
            elif "video" in line:
                self.rtcp_stats["receive"]["video"]["connected"] = True
        elif "BITRATE" in line:
            if "video" in line:
                self.rtcp_stats["receive"]["video"]["bitrate"] = int(line.split(":")[-1])
            elif "audio" in line:
                self.rtcp_stats["receive"]["audio"]["bitrate"] = int(line.split(":")[-1])
        else:
            print "%9s stdout: %s" % (process_manager.identifier, line)

    def on_receiver_stderr_line(self, process_manager, line):
        """
        Handles a new line from our receiver process' stderr
        """
        print "%9s stderr: %s" % (process_manager.identifier, line)
        if "CRITICAL" in line or "ERROR" in line:
            self.error_messages["receive"].append(line)
    
    def on_sender_stdout_line(self, process_manager, line):
        """
        Handles a new line from our receiver process' stdout
        """
        print "%9s stdout: %s" % (process_manager.identifier, line)
        try:
            if "PACKETS-LOST" in line:
                if "video" in line:
                    self.rtcp_stats["send"]["video"]["packets-lost"] = int(line.split(":")[-1])
                elif "audio" in line:
                    self.rtcp_stats["send"]["audio"]["packets-lost"] = int(line.split(":")[-1])
                #self._calculate_packet_loss()
            elif "AVERAGE PACKET-LOSS" in line:
                if "video" in line:
                    self.rtcp_stats["send"]["video"]["packet-loss-percent"] = int(line.split(":")[-1])
                elif "audio" in line:
                    self.rtcp_stats["send"]["audio"]["packet-loss-percent"] = int(line.split(":")[-1])
            elif "PACKETS-SENT" in line:
                if "video" in line:
                    self.rtcp_stats["send"]["video"]["packets-sent"] = int(line.split(":")[-1])
                elif "audio" in line:
                    self.rtcp_stats["send"]["audio"]["packets-sent"] = int(line.split(":")[-1])
                #self._calculate_packet_loss()
            elif "JITTER" in line:
                if "video" in line:
                    self.rtcp_stats["send"]["video"]["jitter"] = int(line.split(":")[-1])
                elif "audio" in line:
                    self.rtcp_stats["send"]["audio"]["jitter"] = int(line.split(":")[-1])
            elif "BITRATE" in line:
                if "video" in line:
                    self.rtcp_stats["send"]["video"]["bitrate"] = int(line.split(":")[-1])
                elif "audio" in line:
                    self.rtcp_stats["send"]["audio"]["bitrate"] = int(line.split(":")[-1])
            elif "connected" in line:
                if "video" in line:
                    self.rtcp_stats["send"]["video"]["connected"] = True
                elif "audio" in line:
                    self.rtcp_stats["send"]["audio"]["connected"] = True
        except ValueError, e:
            print(e)

    def on_sender_stderr_line(self, process_manager, line):
        """
        Handles a new line from our receiver process' stderr
        """
        print "%9s stderr: %s" % (process_manager.identifier, line)
        if "CRITICAL" in line or "ERROR" in line:
            self.error_messages["send"].append(line)

    def is_busy(self):
        """
        Retuns True if a streaming session is in progress.
        """
        return self.state != process.STATE_STOPPED

    def get_all_streamer_process_managers(self):
        """
        Returns all the current streaming process managers for the current session.
        @rtype: list
        """
        ret = []
        for proc in [self.receiver, self.extra_receiver, self.sender, self.extra_sender, self.midi_receiver, self.midi_sender]:
            if proc is not None:
                ret.append(proc)
        return ret

    def on_process_state_changed(self, process_manager, process_state):
        """
        Slot for the ProcessManager.state_changed_signal
        Calls stop() if one of the processes crashed.
        """
        print process_manager, process_state
        if process_state == process.STATE_RUNNING:
            # As soon as one is running, set our state to running
            if self.state == process.STATE_STARTING:
                self._set_state(process.STATE_RUNNING)
        elif process_state == process.STATE_STOPPING:
            pass
        elif process_state == process.STATE_STARTING:
            pass
        elif process_state == process.STATE_STOPPED:
            # As soon as one crashes or is not able to start, stop all streamer processes.
            if self.state in [process.STATE_RUNNING, process.STATE_STARTING]:
                print("A streamer process died. Stopping the local streamer manager.")
                self.stop() # sets self.state to STOPPING
            # Next, if all streamers are dead, we can say this manager is stopped
            if self.state == process.STATE_STOPPING:
                one_is_left = False
                for proc in self.get_all_streamer_process_managers():
                    if process_manager is not proc and proc.state != process.STATE_STOPPED:
                        print("Streamer process %s is not dead, so we are not done stopping. Its state is %s." % (proc, proc.state))
                        one_is_left = True
                if not one_is_left:
                    print "Setting streamers manager to STOPPED"
                    self._set_state(process.STATE_STOPPED)

    def on_stopped(self):
        """
        When the state changes to stopped, 
         * check for errors and display them to the user.
        """
        msg = ""
        details = ""
        show_error_dialog = False
        #TODO: internationalize
        print("All streamers are stopped.")
        print("Error messages for this session: %s" % (self.error_messages))
        if len(self.error_messages["send"]) != 0:
            details += _("Errors from local sender:") + "\n"
            for line in self.error_messages["send"]:
                details += " * " + line + "\n"
            show_error_dialog = True
        if len(self.error_messages["receive"]) != 0:
            details += _("Errors from local receiver:") + "\n"
            for line in self.error_messages["receive"]:
                details += " * " + line + "\n"
            show_error_dialog = True
        if show_error_dialog:
            msg = _("Some errors occured during the audio/video streaming session.")
            dialogs.ErrorDialog.create(msg, parent=self.app.gui.main_window, details=details)
        # TODO: should we set all our process managers to None

        # set all to None:
        self.sender = None
        self.receiver = None
        self.extra_sender = None
        self.extra_receiver = None
        self.midi_receiver = None
        self.midi_sender = None
        print "should all be None:", self.get_all_streamer_process_managers()
    
    def _set_state(self, new_state):
        """
        Handles state changes.
        """
        if self.state != new_state:
            self.state_changed_signal(self, new_state)
            self.state = new_state
            if new_state == process.STATE_STOPPED:
                self.on_stopped()
        else:
            raise RuntimeError("Setting state to %s, which is already the current state." % (self.state))
            
    def stop(self):
        """
        Stops the sender and receiver processes.
        Does not send any message to the remote peer ! This must be done somewhere else.
        """
        #TODO: return a deferred
        # stopping
        if self.state in [process.STATE_RUNNING, process.STATE_STARTING]:
            self._set_state(process.STATE_STOPPING)
            for proc in self.get_all_streamer_process_managers():
                if proc is not None:
                    if proc.state != process.STATE_STOPPED and proc.state != process.STATE_STOPPING:
                        proc.stop()
