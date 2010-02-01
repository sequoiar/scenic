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
        
    def start(self, host, config):
        """
        Starts the sender and receiver processes.
        @param host: str ip addr
        @param config: gui.Config object
        """
        if self.state != process.STATE_STOPPED:
            raise RuntimeError("Cannot start streamers since they are %s." % (self.state))
            # FIXME: catch this and run ;-)
        
        self.milhouse_recv_cmd = [
            config.streamer_command,
            '--receiver', 
            '--address', str(host),
            '--videosink', config.video_output,
            '--audiosink', config.audio_output,
            '--videocodec', config.video_codec,
            '--audiocodec', config.audio_codec,
            '--videoport', str(config.recv_video_port),
            '--audioport', str(config.recv_audio_port) 
            ]
        self.milhouse_send_cmd = [
            config.streamer_command, 
            '--sender', 
            '--address', str(host),
            '--videosource', config.video_input,
            '--videocodec', config.video_codec,
            '--videobitrate', config.video_bitrate,
            '--audiosource', config.audio_input,
            '--audiocodec', config.audio_codec,
            '--videoport', str(config.send_video_port),
            '--audioport', str(config.send_audio_port)]
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

    def on_process_state_changed(self, process_manager, new_state):
        """
        Slot for the ProcessManager.state_changed_signal
        """
        print process_manager, new_state
        if new_state == process.STATE_RUNNING:
            # As soon as one is running, set our state to running
            if self.state == process.STATE_STARTING:
                self._set_state(process.STATE_RUNNING)
        if new_state == process.STATE_STOPPED:
            # As soon as one crashes or is not able to start, stop all streamer processes.
            if self.state in [process.STATE_RUNNING, process.STATE_STARTING]:
                print("A streamer process died. Stopping the local streamer manager.")
                self.stop()
            # If all streamers are dead, we can say this manager is stopped
            one_is_left = False
            for proc in [self.sender, self.receiver]:
                if process_manager is not proc and proc.state != process.STATE_STOPPED:
                    print("Streamer process %s is not dead, so we are not done stopping" % (proc))
                    one_is_left = True
            if not one_is_left:
                self._set_state(process.STATE_STOPPED)
    
    def _set_state(self, new_state):
        """
        Handles state changes.
        """
        if self.state != new_state:
            self.state_changed_signal(self, new_state)
        self.state = new_state
            
    def stop(self):
        """
        Stops the sender and receiver processes.
        Does not send any message to the remote peer ! This must be done somewhere else.
        """
        # stopping
        if self.state in [process.STATE_RUNNING, process.STATE_STARTING]:
            self._set_state(process.STATE_STOPPING)
            if self.sender is not None:
                self.sender.stop()
            if self.receiver is not None:
                self.receiver.stop()
