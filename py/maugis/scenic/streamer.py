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

from scenic import process

class ProcessManager(object):
    """
    PROCESS manager.
    """
    def __init__(self, app):
        self.app = app
        self.config = self.app.config
        self.video_port = self.config.video_port #TODO: different for each session
        self.audio_port = self.config.audio_port # TODO
        # commands
        self.milhouse_recv_cmd = None
        self.milhouse_send_cmd = None

        self.sender = None
        self.receiver = None
        
    def start(self, host):
        # TODO: check if already streamers running.
        self.milhouse_recv_cmd = [
            self.config.streamer_command,
            '--receiver', 
            '--address', str(host),
            '--videosink', self.config.video_output,
            '--audiosink', self.config.audio_output,
            '--videocodec', self.config.video_codec,
            '--audiocodec', self.config.audio_codec,
            '--videoport', str(self.video_port),
            '--audioport', str(self.audio_port) 
            ]
        self.milhouse_send_cmd = [
            self.config.streamer_command, 
            '--sender', 
            '--address', str(host),
            '--videosource', self.config.video_input,
            '--videocodec', self.config.video_codec,
            '--videobitrate', self.config.video_bitrate,
            '--audiosource', self.config.audio_input,
            '--audiocodec', self.config.audio_codec,
            '--videoport', str(self.video_port),
            '--audioport', str(self.audio_port)]
        # setting up
        recv_cmd = " ".join(self.milhouse_recv_cmd)
        self.receiver = process.ProcessManager(command=recv_cmd, identifier="receiver")
        self.receiver.state_changed_signal.connect(self.on_process_state_changed)
        send_cmd = " ".join(self.milhouse_send_cmd)
        self.sender = process.ProcessManager(command=send_cmd, identifier="sender")
        self.sender.state_changed_signal.connect(self.on_process_state_changed)
        # starting
        print "$", send_cmd
        self.sender.start()
        print "$", recv_cmd
        self.receiver.start()

    def on_process_state_changed(self, process_manager, new_state):
        print process_manager, new_state

    def stop(self):
        # stopping
        if self.sender is not None:
            self.sender.stop()
        if self.receiver is not None:
            self.receiver.stop()
