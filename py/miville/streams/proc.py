#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Miville is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.
"""
Tools for deferreds, delayed calls and process management.
"""
import os # for environ
from twisted.internet import protocol
from twisted.internet import reactor
from twisted.python import procutils
from miville.utils import sig
from miville.utils import log

log = log.start("debug", 1, 0, "streams.proc")

STATE_IDLE = "IDLE" 
STATE_STARTING = "STARTING"
STATE_RUNNING = "RUNNING" # success
STATE_STOPPING = "STOPPING"
STATE_STOPPED = "STOPPED" # success
STATE_ERROR = "ERROR"

class ManagedProcessError(Exception):
    """
    Raised by ProcessManager
    """
    pass

class TextLinesLogger(object):
    """
    Logs lines of text.
    """
    def __init__(self, maxsize=0, prefix=""):
        self.lines = []
        self.maxsize = maxsize
        self.prefix = prefix

    def append(self, line):
        self.lines.append(line.strip())
        if self.maxsize != 0:
            if len(self.lines) > self.maxsize:
                self.lines.pop(0)
    
    def get_text(self):
        """
        Returns the whole text logged.
        """
        ret = ""
        for line in self.lines:
            ret += self.prefix + line + "\n"
        return ret
    
    def clear(self):
        """
        Empties the lines of text
        """
        self.lines = []

class ManagedProcessProtocol(protocol.ProcessProtocol):
    """
    Process managed by a ProcessManager.
 
    Its stdin/stdout streams are logged.    
    """
    def __init__(self, manager):
        """
        :param manager: ProcessManager instance.
        """
        self.manager = manager

    def connectionMade(self):
        """
        Called once the process is started.
        """
        self.manager._on_connection_made()

    def outReceived(self, data):
        """
        Called when text is received from the managed process stdout
        Twisted will not splitlines, it gives an arbitrary amount of
        data at a time. This way, our manager only gets one line at 
        a time.
        """
        for line in data.splitlines():
            if line != "":
                self.manager._on_out_received(line)

    def errReceived(self, data):
        """
        Called when text is received from the managed process stderr
        """
        for line in data.splitlines().strip():
            if line != "":
                self.manager._on_err_received(data)

    def processEnded(self, status):
        """
        Called when the managed process has exited.
        status is probably a twisted.internet.error.ProcessTerminated
        "A process has ended with a probable error condition: process ended by signal 1"
        
        This is called when all the file descriptors associated with the child 
        process have been closed and the process has been reaped. This means it 
        is the last callback which will be made onto a ProcessProtocol. 
        The status parameter has the same meaning as it does for processExited.
        """
        self.manager._on_process_ended(status)
    
    def inConnectionLost(self, data):
        log.debug("stdin pipe has closed." + str(data))

    def outConnectionLost(self, data):
        log.debug("stdout pipe has closed." + str(data))
    
    def errConnectionLost(self, data):
        log.debug("stderr pipe has closed." + str(data))

    def processExited(self, reason):
        """
        This is called when the child process has been reaped, and receives 
        information about the process' exit status. The status is passed in the form 
        of a Failure instance, created with a .value that either holds a ProcessDone 
        object if the process terminated normally (it died of natural causes instead 
        of receiving a signal, and if the exit code was 0), or a ProcessTerminated 
        object (with an .exitCode attribute) if something went wrong.
        """
        log.debug("process has exited " + str(reason))
    
class ProcessManager(object):
    """
    Starts one  ManagedProcessProtocol.
    
    You should create one ProcessManager each time you start a process, and delete it after.
    """
    # constants
    # do not mix these process states with the streams states ! they are 2 diff. things.
    # default that can be overriden in children classes.
    
    def __init__(self, name="default", log_max_size=100, command=None, verbose=False, process_protocol_class=ManagedProcessProtocol, env=None):
        """
        :param command: list or args. The first item is the name of the name of the executable.
        :param name: Any string, for printing infos. Does not need to be unique.
        Might raise a ManagedProcessError
        """
        self.name = name
        self.command = list(command)
        if command is None:
            raise ManagedProcessError("You must provide a command to be run.")
        else:
            try:
                self.command[0] = procutils.which(self.command[0])[0]
            except IndexError:
                raise ManagedProcessError("Could not find path of executable %s." % (self.command[0]))
        self.process_protocol_class = process_protocol_class
        if env is None:
            env = {}
        self.env = env
        self.state = STATE_IDLE
        self.stdout_logger = TextLinesLogger(maxsize=log_max_size, prefix=self.name)
        self.stderr_logger = TextLinesLogger(maxsize=log_max_size, prefix=self.name)
        self.verbose = verbose
        self._process_protocol = None
        self._process_transport = None
        self.state_changed_signal = sig.Signal() # FIXME: this is only used in a test so far
        self.exitted_itself_signal = sig.Signal()

    def set_state(self, new_state):
        if self.state != new_state:
            self.state = new_state
            self.state_changed_signal(self.state)

    def start(self):
        """
        Start the managed process
        """
        self.stdout_logger.clear()
        self.stderr_logger.clear()
        self._process_protocol = ManagedProcessProtocol(self)
        log.debug("Running command %s" % (str(self.command)))
        try:
            proc_path = self.command[0]
            args = self.command
            environ = {}
            for key in ['HOME', 'DISPLAY', 'PATH']: # passing a few env vars
                if os.environ.has_key(key):
                    environ[key] = os.environ[key]
            for key, val in self.env.iteritems():
                environ[key] = val # override
            self.set_state(STATE_STARTING)
            log.debug("Starting process (%s) %s %s" % (self.name, self.command, environ))
            self._process_transport = reactor.spawnProcess(self._process_protocol, proc_path, args, environ, usePTY=True)
        except TypeError, e:
            self.set_state(STATE_ERROR)
            raise
        else:
            log.debug("Process is started.")

    def _on_connection_made(self):
        if STATE_STARTING:
            self.set_state(STATE_RUNNING)
        else:
            self.set_state(STATE_ERROR)
        
    def _on_out_received(self, data):
        log.debug("stdout(%s): %s" % (self.name, data))
        self.stdout_logger.append(data.strip())
        # TODO: parse output to check if ok.

    def _on_err_received(self, data):
        log.debug("stderr(%s): %s" % (self.name, data))
        self.stderr_logger.append(data.strip())
        # TODO: parse output to check if ok.

    def stop(self):
        """
        Stops the managed process
        """
        if self.state == STATE_RUNNING:
            self.set_state(STATE_STOPPING)
            self._process_transport.loseConnection()
        else:
            msg = "Cannot stop a process that is \"%s\" state." % (self.state)
            self.set_state(STATE_ERROR)
            raise ManagedProcessError(msg)

    def _on_process_ended(self, reason):
        #log.warning("%s process ended. Reason: \n%s State:%s" % (self.name, str(reason), self.state))
        if self.state == STATE_STARTING:
            self.set_state(STATE_ERROR)
        elif self.state == STATE_RUNNING:
            """ Don't error out if we exitted with exit code 0 (for now) """
            if str(reason).find('exit code 0') != -1:
                log.warning('PROCESS EXITTED OF ITS OWN ACCORD, CLEANLY')
                self.exitted_itself_signal()
            else:
                self.set_state(STATE_ERROR)
        if self.state == STATE_STOPPING:
            self.set_state(STATE_STOPPED)
        if self.verbose:
            print("%s process ended. Reason: \n%s" % (self.name, str(reason)))
