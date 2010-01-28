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

import gobject
import subprocess

class ProcessManager(object):
    """
    PROCESS manager.
    """
    def __init__(self, app):
        self.app = app
        self.config = self.app.config
        self.video_port = self.config.video_port
        self.audio_port = self.config.audio_port
        # receiver
        self.milhouse_recv_cmd = None
        self.milhouse_recv_pid = None
        # files
        self.milhouse_recv_input = None
        self.milhouse_recv_output = None
        self.milhouse_recv_error = None
        # File description watcher
        self.watched_milhouse_recv_id = None
        self.milhouse_recv_timeout = None # call_later
        # sender
        self.milhouse_send_cmd = None
        self.milhouse_send_subproc = None
        self.milhouse_send_pid = None
        self.milhouse_send_timeout = None
        
    def start(self, host, bandwidth):
        base = 30
        divider = base / bandwidth
        # First, start the milhouse_recv process, milhouse_send needs a remote running propulseart --receive to work
        self.milhouse_recv_cmd = [
            self.config.streamer_command,
            '--receiver', 
            '--address', host,
            '--videosink', self.config.video_output,
            '--audiosink', self.config.audio_output,
            '--videocodec', self.config.video_codec,
            '--audiocodec', self.config.audio_codec,
            '--videoport', str(self.video_port),
            '--audioport', str(self.audio_port) 
            ]
        print "milhouse_recv_cmd: ", self.milhouse_recv_cmd
        
        # local function declaration:
        def _env_sequence():
            return [key + '=' + value for key, value in os.environ.items()]
        
        self.milhouse_recv_pid, self.milhouse_recv_input, self.milhouse_recv_output, self.milhouse_recv_error = gobject.spawn_async(
            self.milhouse_recv_cmd,
            envp = _env_sequence(),
            working_directory = os.environ['PWD'],
            flags = gobject.SPAWN_SEARCH_PATH,
            standard_input = False,
            standard_output = True,
            standard_error = True)
        self.watched_milhouse_recv_id = gobject.io_add_watch(
            self.milhouse_recv_output,
            gobject.IO_HUP,
            self.watch_milhouse_recv)
        
        self.milhouse_send_cmd = [
            self.config.streamer_command, 
            '--sender', 
            '--address', host,
            '--videosource', self.config.video_input,
            '--videocodec', self.config.video_codec,
            '--videobitrate', self.config.video_bitrate,
            '--audiosource', self.config.audio_input,
            '--audiocodec', self.config.audio_codec,
            '--videoport', str(self.video_port),
            '--audioport', str(self.audio_port)]

        print "milhouse_send_cmd: ", self.milhouse_send_cmd
        self.milhouse_send_subproc = subprocess.Popen(self.milhouse_send_cmd, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        print "milhouse_send_cmd launched "
        self.milhouse_send_pid = self.milhouse_send_subproc.pid

    def watch_milhouse_recv(self, *args):
        print "watch_milhouse_recv"
        self.milhouse_recv_timeout = gobject.timeout_add(5000, self.stop)
        return False

    def watch_milhouse_send(self, *args):
        print "watch_milhouse_send"
        self.milhouse_send_timeout = gobject.timeout_add(5000, self.stop)
        return False

    def stop(self):
        print "stop: ", 
        if hasattr(self, "watched_milhouse_recv_id"):
            print "watch"
            try:
                gobject.source_remove(self.watched_milhouse_recv_id)
            except TypeError:
                pass
        if hasattr(self, "timeout"):
            print "timeout"
            gobject.source_remove(self.timeout)
        try:
            print "killing milhouse_recv: ", self.milhouse_recv_pid
            os.kill(self.milhouse_recv_pid, signal.SIGTERM)
            # not waiting for child process !!
        except:
            pass
        try:
            print "killing milhouse_send_pid: ", self.milhouse_send_pid
            os.kill(self.milhouse_send_pid, signal.SIGTERM)
            print "send: before os.wait()"
            os.wait()
            print "send: after os.wait()"
        except:
            pass
        self.app.on_stop_milhouse_send()

