#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Preview Process management.
"""
from scenic import sig
from scenic import process
from scenic.internationalization import _

class Preview(object):
    """
    Local preview - not live
    """
    def __init__(self, app):
        self.app = app
        self.process_manager = None #process.ProcessManager(identifier="preview")
        self.state = process.STATE_STOPPED
        self.state_changed_signal = sig.Signal()
            
    def is_busy(self):
        """
        Retuns True if a preview is in progress.
        """
        return self.state != process.STATE_STOPPED
        
    def _create_command(self):
        """
        Looks in the settings, and returns a bash command to run for the preview.
        Note that the preview doesn't use the display option, which sets the X11 DISPLAY.
        The preview is only for video, no sound.
        @rettype: str
        """
        width, height = self.app.config.video_capture_size.split("x")
        aspect_ratio = self.app.config.video_aspect_ratio
        window_title = _("Local preview")
        x_window_id = None
        if not self.app.config.preview_in_window:
            if self.app.gui.preview_area_x_window_id is None:
                print("WARNING: XID of the preview drawing area is None !")
            else:
                x_window_id = self.app.gui.preview_area_x_window_id
        command = "milhouse --videosource %s --localvideo --window-title \"%s\" --width %s --height %s --aspect-ratio %s" % (self.app.config.video_source, window_title, width, height, aspect_ratio)
        if x_window_id is not None:
            command += " --x-window-id %d" % (x_window_id)
        else:
            command += " --display %s" % (self.app.config.video_display) # xid does not work if DISPLAY is set to an other display.
        if self.app.config.video_source != "videotestsrc":
            command += " --videodevice %s" % (self.app.config.video_device)
        return command
        
    def start(self):
        print("Starting the preview")
        if self.state != process.STATE_STOPPED:
            raise RuntimeError("Cannot start preview since it is %s." % (self.state)) # the programmer has done something wrong if we're here.
        else:
            command = self._create_command()
        self.process_manager = process.ProcessManager(command=command, identifier="preview")
        self.process_manager.stdout_line_signal.connect(self.on_stdout_line)
        self.process_manager.stderr_line_signal.connect(self.on_stderr_line)
        self.process_manager.state_changed_signal.connect(self.on_process_state_changed)
        self._set_state(process.STATE_STARTING)
        self.process_manager.start()

    def on_stdout_line(self, process_manager, line):
        print line

    def on_stderr_line(self, process_manager, line):
        print line        
        
    def on_process_state_changed(self, process_manager, process_state):
        """
        Slot for the ProcessManager.state_changed_signal
        """
        print "Preview:", process_manager, process_state
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
        else:
            raise RuntimeError("Setting state to %s, which is already the current state." % (self.state))
            
    def stop(self):
        """
        Stops the preview process.
        """
        print("Stopping the preview")
        if self.state in [process.STATE_RUNNING, process.STATE_STARTING]:
            self._set_state(process.STATE_STOPPING)
            if self.process_manager is not None:
                if self.process_manager.state != process.STATE_STOPPED and self.process_manager.state != process.STATE_STOPPING:
                    print('stopping the preview process')
                    self.process_manager.stop()
        else:
            print("Warning: preview state is %s" % (self.state))
    
