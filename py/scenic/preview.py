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
Preview Process management.
"""
from scenic import sig
from scenic import process
from scenic.internationalization import _
from scenic import logger

log = logger.start(name="preview")

class Preview(object):
    """
    Local preview - not live
    """
    def __init__(self, app):
        self.app = app
        self.process_manager = None
        self.state = process.STATE_STOPPED
        self.state_changed_signal = sig.Signal()
        self.error_messages = None # either None or a list
        self.warnings = None # either None or a list
            
    def is_busy(self):
        """
        Retuns True if a preview is in progress.
        """
        return self.state != process.STATE_STOPPED
        
    def _create_command(self):
        """
        Looks in the settings, and returns a bash command to run for the preview.
        The preview is only for video, no sound.
        @rtype: str
        """
        width, height = self.app.config.video_capture_size.split("x")
        aspect_ratio = self.app.config.video_aspect_ratio
        numchannels = self.app.config.audio_channels
        vumeter_id = self.app.gui.audio_levels_input_socket_id
        audio_buffer = self.app.config.audio_input_buffer
        jack_autoconnect = self.app.config.audio_jack_enable_autoconnect
        window_title = _("Local preview")
        x_window_id = None
        if not self.app.config.preview_in_window:
            if self.app.gui.preview_area_x_window_id is None:
                log.error("XID of the preview drawing area is None !")
            else:
                x_window_id = self.app.gui.preview_area_x_window_id
        command = "milhouse --videosource %s --localvideo --window-title \"%s\" --width %s --height %s --aspect-ratio %s" % (self.app.config.video_source, window_title, width, height, aspect_ratio, )
        if self.app.devices["jackd_is_running"] and numchannels > 0: 
            command += " --localaudio --audiosource %s --numchannels %s --vumeter-id %s --audio-buffer %s" % (self.app.config.audio_source, numchannels, vumeter_id, audio_buffer)
            if not jack_autoconnect:
                command += " --disable-jack-autoconnect"
        #else:
        #    warning_message = "You should consider starting jackd."
        if x_window_id is not None:
            command += " --x-window-id %d" % (x_window_id)
        else:
            command += " --videodisplay %s" % (self.app.config.video_display) # xid does not work if DISPLAY is set to an other display.
        if self.app.config.video_source != "videotestsrc":
            dev = self.app.parse_v4l2_device_name(self.app.config.video_device)
            if dev is None:
                log.error("v4l2 device is not found ! %s" % (self.app.config.video_device))
                #FIXME: handle this
            video_device = dev["name"]
            command += " --videodevice %s" % (video_device)
        return command
        
    def start(self):
        log.info("Starting the preview")
        if self.state != process.STATE_STOPPED:
            raise RuntimeError("Cannot start preview since it is %s." % (self.state)) # the programmer has done something wrong if we're here.
        else:
            command = self._create_command()
        self.error_messages = []
        self.warnings = []
        self.process_manager = process.ProcessManager(command=command, identifier="preview")
        self.process_manager.stdout_line_signal.connect(self.on_stdout_line)
        self.process_manager.stderr_line_signal.connect(self.on_stderr_line)
        self.process_manager.state_changed_signal.connect(self.on_process_state_changed)
        self._set_state(process.STATE_STARTING)
        self.process_manager.start()

    def on_stdout_line(self, process_manager, line):
        log.debug(line)
        if "WARNING" in line:
            log.warning(line)
            self.warnings.append(line)

    def on_stderr_line(self, process_manager, line):
        log.debug(line)
        if "CRITICAL" in line or "ERROR" in line:
            self.error_messages.append(line)
        if "WARNING" in line:
            log.warning(line)
            self.warnings.append(line)
        
    def on_process_state_changed(self, process_manager, process_state):
        """
        Slot for the ProcessManager.state_changed_signal
        """
        log.debug("Preview: %s %s" % (process_manager, process_state))
        if process_state == process.STATE_RUNNING:
            # As soon as it is running, set our state to running
            if self.state == process.STATE_STARTING:
                self._set_state(process.STATE_RUNNING)
        elif process_state == process.STATE_STOPPING:
            pass
        elif process_state == process.STATE_STARTING:
            pass
        elif process_state == process.STATE_STOPPED:
            self._set_state(process.STATE_STOPPED)
    
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

    def on_stopped(self):
        show_error_dialog = False
        details = ""
        if len(self.error_messages) != 0:
            show_error_dialog = True
            log.error("Error messages from the preview : %s" % (self.error_messages))
            details += _("Errors from local preview:") + "\n"
            for line in self.error_messages:
                details += " * " + line + "\n"
            if len(self.warnings) != 0:
                details += _("Warnings from local preview:") + "\n"
                for line in self.warnings:
                    details += " * " + line + "\n"
        if show_error_dialog:
            msg = _("Some errors occured while looking at the local preview.")
            self.app.gui.show_error_dialog(msg, details=details)
            
    def stop(self):
        """
        Stops the preview process.
        """
        log.info("Stopping the preview")
        if self.state in [process.STATE_RUNNING, process.STATE_STARTING]:
            self._set_state(process.STATE_STOPPING)
            if self.process_manager is not None:
                if self.process_manager.state != process.STATE_STOPPED and self.process_manager.state != process.STATE_STOPPING:
                    self.process_manager.stop()
        else:
            log.debug("Warning: preview state is %s" % (self.state))
    
