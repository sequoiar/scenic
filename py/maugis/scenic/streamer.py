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
            self.stop_streamers()
        Starts the sender and receiver processes.
        @param host: str ip addr
        @param config: gui.Config object
        
        Raises a RuntimeError if a sesison is already in progress.
        """
        #FIXME: uses self.app.*_port variables to know which ports to use.
        if self.state != process.STATE_STOPPED:
            raise RuntimeError("Cannot start streamers since they are %s." % (self.state))
            # FIXME: catch this and run ;-)
        
        self.milhouse_recv_cmd = [
            "milhouse",
            '--receiver', 
            '--address', str(host),
            '--videosink', config.video_sink,
            '--audiosink', config.audio_sink,
            '--videocodec', config.video_codec,
            '--audiocodec', config.audio_codec,
            '--videoport', str(self.app.recv_video_port),
            '--audioport', str(self.app.recv_audio_port) 
            ]
        self.milhouse_send_cmd = [
            "milhouse", 
            '--sender', 
            '--address', str(host),
            '--videosource', config.video_source,
            '--videocodec', config.video_codec,
            '--videobitrate', str(config.video_bitrate),
            '--audiosource', config.audio_source,
            '--audiocodec', config.audio_codec,
            '--videoport', str(self.app.send_video_port),
            '--audioport', str(self.app.send_audio_port)]
        if config.video_source == "v4l2src":
            self.milhouse_send_cmd.extend(["--videodevice", config.video_device])
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
        # stopping
        if self.state in [process.STATE_RUNNING, process.STATE_STARTING]:
            self._set_state(process.STATE_STOPPING)
            for proc in [self.sender, self.receiver]:
                if proc is not None:
                    if proc.state != process.STATE_STOPPED and proc.state != process.STATE_STOPPING:
                        proc.stop()
