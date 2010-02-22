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

class StreamerManager(object):
    """
    Manages local streamer processes.
    """
    def __init__(self, app):
        self.app = app
        # commands
        self.milhouse_recv_cmd = None
        self.milhouse_send_cmd = None
        self.sender = None
        self.receiver = None
        self.state = process.STATE_STOPPED
        self.state_changed_signal = sig.Signal()
        # for stats
        #TODO: add ports?
        self.session_details = None # either None or a big dict

    def _gather_config_to_stream(self, addr):
        """
        Gathers all settings in a big dict.
        
        Useful for feedback to the user.
        """
        contact_name = addr
        contact = self.app._get_contact_by_addr(addr)
        if contact is not None:
            contact_name = contact["name"]
        
        remote_config = self.app.remote_config
        send_width, send_height = self.app.config.video_capture_size.split("x")
        receive_width, receive_height = remote_config["video"]["capture_size"].split("x")
        self.session_details = {
            "peer": {
                "address": addr,
                "name": contact_name,
                },
            # ----------------- send ---------------
            "send": {
                "video": {
                    "source": self.app.config.video_source,
                    "device": self.app.config.video_device,
                    "bitrate": self.app.config.video_bitrate,
                    "codec": remote_config["video"]["codec"],
                    "width": int(send_width), # int
                    "height": int(send_height), # int
                    "aspect-ratio": self.app.config.video_aspect_ratio,
                    "port": self.app.remote_config["video"]["port"], 
                },
                "audio": {
                    "source": self.app.config.audio_source,
                    "numchannels": self.app.config.audio_channels,
                    "codec": self.app.config.audio_codec,
                    "port": self.app.remote_config["audio"]["port"], 
                }
            },
            # -------------------- recv ------------
            "receive": {
                "video": {
                    "sink": self.app.config.video_sink,
                    "codec": self.app.config.video_codec,
                    "width": int(receive_width), # int
                    "height": int(receive_height), # int
                    "deinterlace": self.app.config.video_deinterlace, # bool
                    "aspect-ratio": remote_config["video"]["aspect_ratio"],
                    "port": str(self.app.recv_video_port), #decided by the app
                    "window-title": "\"From %s\"" % (contact_name), #TODO: i18n
                    "jitterbuffer": self.app.config.video_jitterbuffer, 
                    "fullscreen": self.app.config.video_fullscreen, # bool
                    "bitrate": remote_config["video"]["bitrate"], # int
                },
                "audio": {
                    "numchannels": self.app.config.audio_channels, # int
                    "codec": self.app.config.audio_codec, 
                    "port": self.app.recv_audio_port,
                    "sink": self.app.config.audio_sink
                }
            }
        }
        if self.session_details["send"]["video"]["source"] != "v4l2src":
            self.session_details["send"]["video"]["device"] = None
        if self.session_details["send"]["video"]["codec"] == "theora":
            self.session_details["send"]["video"]["bitrate"] = None
        
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

        # ------------------ send ---------------
        self.milhouse_send_cmd = [
            "milhouse", 
            '--sender', 
            '--address', details["peer"]["address"],
            '--videosource', details["send"]["video"]["source"],
            '--videocodec', details["send"]["video"]["codec"],
            '--videoport', str(details["send"]["video"]["port"]),
            '--width', str(details["send"]["video"]["width"]),
            '--height', str(details["send"]["video"]["height"]),
            '--aspect-ratio', str(details["send"]["video"]["aspect-ratio"]),
            '--audiosource', details["send"]["audio"]["source"],
            '--numchannels', str(details["send"]["audio"]["numchannels"]),
            '--audiocodec', details["send"]["audio"]["codec"],
            '--audioport', str(details["send"]["audio"]["port"]),
            ]
        if details["send"]["video"]["source"] == "v4l2src":
            self.milhouse_send_cmd.extend(["--videodevice", details["send"]["video"]["device"]])
        if details["send"]["video"]["codec"] != "theora":
            self.milhouse_send_cmd.extend(['--videobitrate', str(int(details["send"]["video"]["bitrate"] * 1000000))])

        # ------------------- recv ----------------
        self.milhouse_recv_cmd = [
            "milhouse",
            '--receiver', 
            '--address', details["peer"]["address"],
            '--videosink', details["receive"]["video"]["sink"],
            '--videocodec', details["receive"]["video"]["codec"],
            '--videoport', str(details["receive"]["video"]["port"]),
            '--jitterbuffer', str(details["receive"]["video"]["jitterbuffer"]),
            '--width', str(details["receive"]["video"]["width"]),
            '--height', str(details["receive"]["video"]["height"]),
            '--aspect-ratio', details["receive"]["video"]["aspect-ratio"],
            '--audiosink', details["receive"]["audio"]["sink"],
            '--numchannels', str(details["receive"]["audio"]["numchannels"]),
            '--audiocodec', details["receive"]["audio"]["codec"],
            '--audioport', str(details["receive"]["audio"]["port"]),
            '--window-title', details["receive"]["video"]["window-title"],
            ]
        if details["receive"]["video"]["fullscreen"]:
            self.milhouse_recv_cmd.append('--fullscreen')
        if details["receive"]["video"]["deinterlace"]:
            self.milhouse_recv_cmd.append('--deinterlace')

        # setting up
        recv_cmd = " ".join(self.milhouse_recv_cmd)
        self.receiver = process.ProcessManager(command=recv_cmd, identifier="receiver")
        self.receiver.state_changed_signal.connect(self.on_process_state_changed)
        send_cmd = " ".join(self.milhouse_send_cmd)
        self.sender = process.ProcessManager(command=send_cmd, identifier="sender")
        self.sender.state_changed_signal.connect(self.on_process_state_changed)
        # starting
        self._set_state(process.STATE_STARTING)
        print "$", send_cmd
        self.sender.start()
        print "$", recv_cmd
        self.receiver.start()

    def is_busy(self):
        """
        Retuns True if a streaming session is in progress.
        """
        return self.state != process.STATE_STOPPED

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
                for proc in [self.sender, self.receiver]:
                    if process_manager is not proc and proc.state != process.STATE_STOPPED:
                        print("Streamer process %s is not dead, so we are not done stopping. Its state is %s." % (proc, proc.state))
                        one_is_left = True
                if not one_is_left:
                    print "Setting streamers manager to STOPPED"
                    self._set_state(process.STATE_STOPPED)
    
    def _set_state(self, new_state):
        """
        Handles state changes.
        """
        if self.state != new_state:
            self.state_changed_signal(self, new_state)
            self.state = new_state
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
            for proc in [self.sender, self.receiver]:
                if proc is not None:
                    if proc.state != process.STATE_STOPPED and proc.state != process.STATE_STOPPING:
                        proc.stop()
