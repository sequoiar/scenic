#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
Milhouse process management and communication.

This module contains the bas process protocol, server and client for the milhouse process.

See miville.protocols.ipcp for the implementation of the IPCP protocol used to communicate with milhouse.
Milhouse is the process that manages the GST (Gstreamer) pipeline.
"""

# System imports
import os
import resource
import signal
import time
import sys
import select

# Twisted imports
from twisted.internet import reactor
from twisted.internet import protocol
from twisted.internet import defer
from twisted.internet import abstract
from twisted.python import procutils

# App imports
from miville.protocols import ipcp
from miville.utils import log
from miville.utils.common import get_def_name

log = log.start('info', 1, 0, 'base_gst')

# states
STOPPED = 0
STARTING = 1
RUNNING = 2
CONNECTING = 3
CONNECTED = 4
STREAMSTOPPED = 5
STREAMINIT = 6
STREAMING = 7
FAILURE = 100

# name of the process to start
streaming_server_process_name = "milhouse"

class GstError(Exception):
    """
    Any error that is critical and makes it impossible to stream using GST
    """
    pass

class BaseGst(object):
    """
    Attributes for milhouse IPCP controls.
    """
    def get_attr(self, name):
        """
        name: string
        """
        return getattr(self, name)

    def get_attrs(self):
        return [(attr, value) for attr, value in self.__dict__.items() if attr[0] != "_"]

    def set_attr(self, name, value):
        """
        name: string
        value:
        """
        if hasattr(self, name):
            setattr(self, name, value)
            return True, name, value
        return False, name, value

class GstServer(object):
    """
    Class that starts a milhouse process.

    Instanciated by a GstClient instance. 

    The process attribute stores a ProcessProtocol instance. I think.
    """
    def __init__(self, mode, port, address, callback_dict, state_change_callback, proc_output_callback):
        self.port = port
        self.address = address
        self.mode = mode
        log.debug("GstServer.__init__(mode = %s, port=%d, address=%s)" % (self.mode, self.port, self.address))
        self.process = None
        self.conn = None
        self.state = -1
        self.state_change_callback = state_change_callback
        self.change_state(STOPPED)
        self.commands = []
        self.callback_dict = callback_dict
        self.proc_output_callback = proc_output_callback
        self.version_str = None

    def connect(self):
        """
        Connects to milhouse using IPCP TCP inter-process protocol.
        """
        # TODO: return deferred
        log.debug("GstServer.connect")
        if self.state == RUNNING:
            self.change_state(CONNECTING)
            deferred = ipcp.connect(self.address, self.port)
            deferred.addCallback(self.connection_ready)
            deferred.addErrback(self.connection_failed)

    def get_state_str(self, state):
        """
        Returns state as a string for a state int.
        """
        all_states = {-1:'NOTHING', STOPPED:'STOPPED', STARTING:'STARTING', RUNNING:'RUNNING', 
                   CONNECTING:'CONNECTING', CONNECTED:'CONNECTED', STREAMSTOPPED: 'STREAMSTOPPED',
                   STREAMINIT:'STREAMINIT', STREAMING:'STREAMING', FAILURE:'FAILURE'}
        s = all_states[state]
        return s
    
    def get_status(self):
        """
        Returns state as a string.
        """
        s = self.get_state_str(self.state)
        return s
        
    def change_state(self, new_state):
        """
        Changes the state
        """
        old_state = self.state
        self.state = new_state
        old_str = self.get_state_str(old_state)
        new_str = self.get_state_str(new_state)
        log.debug('GstServer state changed from %s to %s handle: %s ' % ( old_str, new_str, str(self) ))
        self.state_change_callback(old_state, new_state, old_str, new_str)

    def connection_ready(self, ipcp):
        """
        Called when the IPCP connection is up between this and the milhouse process.
        """
        log.debug("CONNECTION ready %s" % str(self))
        msg = 'GstServer.connection_ready: Address: %s, Port: %s, conn %s' % (self.address, self.port, str(ipcp) )
        log.debug(msg)
        log.debug('CONN: %s %s' % (ipcp, ipcp.__dict__))
        self.conn = ipcp
        self.conn.connectionLost = self.connection_lost

        for key, callback  in self.callback_dict.iteritems():
            self.conn.add_callback(callback, key)
          
        self.change_state(CONNECTED)
        log.debug('GST inter-process link created')
    
    def connection_failed(self, conn):
        """
        Called when the IPCP connection failed.
        """
        msg = 'GstServer.connection_failed: Address: %s, Port: %s %s' % (self.address, self.port, str(self))
        log.debug(msg + " "  + str(self))
        ipcp.connection_failed(conn)
        self.kill()

    def connection_lost(self, reason=protocol.connectionDone):
        """
        Called when the IPCP connection has been lost.
        """
        self.kill()
        self.conn.del_callback('log')
        log.debug('Lost the server connection. Reason:\n%s %s' % (reason, str(self) ) )

    def process_output_received(self, line):
        """
        Called when a new line is received from milhouse stdout.
        """
        #log.info('GstServer.process_output_received: %s' % line)
        self.proc_output_callback(line)

    def start_process(self):
        """
        Starts the milhouse process.

        Might throw a GstError.
        Uses reactor.spawnProcess with a GstProcessProtocol instance. 
        Stores the process in self.process.
        """
        log.info('<<<<<<<<<<< STARTING MILHOUSE PROCESS >>>>>>>>>>>')
        log.debug("GstServer.start_process... %s " % str(self) )
        #if self.state < RUNNING: self.state = RUNNING    # Uncomment this line to start the GST process "by hand"
        if self.state < STARTING:
            self.change_state(STARTING)
            path = procutils.which(streaming_server_process_name)
            if path:
                if self.mode == 'send':
                    mode_arg = "-s"
                else:
                    mode_arg = "-r"
                try:
                    protocol = GstProcessProtocol(self)
                    args = [path[0], mode_arg, "--serverport", str(self.port)]
                    proc_path = path[0]

                    msg = "Start process> path: %s, protocol: %s args: %s %s" % (proc_path, protocol, str(args), str(self))
                    # print msg
                    log.debug(msg)
                    self.process = reactor.spawnProcess(protocol, proc_path, args, os.environ, usePTY=True)
                    log.debug("Process spawned pid=" + str(self.process.pid)  + " "+ str(self))
                    return proc_path, args, self.process.pid
                except Exception, e:
                    msg = 'Cannot start the GST application "%s": %s %s' % (streaming_server_process_name, str(e), str(self))
                    log.critical(msg)
                    raise GstError(msg)
            else:
                msg = 'Cannot find the GST application: %s' % (streaming_server_process_name)
                log.critical(msg)
                raise GstError(msg)
        else:
            msg = "GstServer.start_process: not in proper state for starting %s" % str(self)
            log.error(msg)
            raise GstError(msg)

    def kill(self):
        """
        Kills the milhouse process.
        
        Uses ProcessProtocol.loseConnection()
        """
        log.debug("GstServer.kill %s" % str(self))
        self.change_state(STOPPED)
#        os.kill(self._process.pid, signal.SIGTERM)
        reactor.callLater(0.5, self._verify_kill)
        try:
            self.process.loseConnection()
        except:
            log.debug('Process %s or connection seems already dead. %s' % str(self.process.pid) ,str(self))
#        self.process = None

    def _verify_kill(self):
        """
        Kills the process using os.kill with SIGKILL (kill -9)
        """
        try:
            status = os.waitpid(self.process.pid, 0)
        except:
            pass
            #log.debug('Process %s already dead %s %s' % (str(self.process.pid) ,  str(self)) )
        else:
            if status[1] != 0:
                pid = self.process.pid
                log.debug("Killing pid: %s %s" % (str(pid)) )
                try:
                    os.kill(pid, signal.SIGKILL)
                except OSError:
                    pass # no such process

    def _process_cmd(self): 
        """
        Sends one command at a time to milhouse using IPCP.
        """
        #log.debug('.')   
        if len(self.commands) > 0:
            if self.conn != None:
                if self.state >= CONNECTED :
                    (cmd, args) = self.commands.pop()
                    log.debug('GstServer._process_cmd:  ' + cmd + "  args: " + str(args) + "  pid " + str(self.process.pid)  + " " + str(self) + '\n\n')
                    if args != None:
                        if isinstance(args, (tuple, list)):
                            self.conn.send_cmd(cmd, *args)
                        else:
                            self.conn.send_cmd(cmd, args)
                    else:
                        self.conn.send_cmd(cmd)      
        if len(self.commands) > 0:
            if self.state > STOPPED:
                reactor.callLater(1, self._process_cmd)

    def send_cmd(self, cmd, args=None, callback=None, timer=None, timeout=3):
        """
        Sends a IPCP command to milhouse
        """
        log.debug('GstServer.send_cmd state: ' + str(self.state) + " cmd: " +  cmd + " args: " + str(args)+ "pid " + str(self.process.pid) + " " + str(self) )
        self.commands.insert(0, (cmd,args) )
        self._process_cmd()

    def del_callback(self, callback=None):
        """
        Deletes a callback to a IPCP message received from the process.
        (i am not sure about this one.)
        """
        log.debug('GstServer.del_callback ' + str(self))
        if self.state == CONNECTED:
            if callback:
                self.conn.del_callback(callback)
            else:
                try:
                    self.conn.del_callback(get_def_name())
                except:
                    log.debug("No callback to delete. (coming from: %s). %s" % (get_def_name(), str(self)))

    def add_callback(self, cmd, name=None, timer=None):
        """
        Adds a callback to a IPCP message received from the process.
        (i am not sure about this one.)
        """
        log.debug('GstServer.add_callback ' + str(self) )
        if self.conn:
            self.conn.add_callback(cmd, name)
        else:
            curr_time = time.time()
            if not timer:
                timer = curr_time
            if curr_time - timer < 3:
                #delay = 0.001
                delay = 0.5
                reactor.callLater(delay, self.add_callback, cmd, name, timer)
            else:
                log.critical('add_callback: The GST process is not ready to connect.' + str(self))

class GstClient(BaseGst):
    """
    Start a GstServer (milhouse process) and communicates with it using the IPCP protocol. 

    The Inter-process Communication protocol is based on TCP lines of ASCII characters.
    
    This class could be deleted, I think, and we could use the GstServer directly.
    """
    def __init__(self):
        log.debug("GstClient __init__  " + str(self))
        
        # FIXME : how come there was no attributes in __init__ ???!?:%^&
        self.gst_server = None
        self.version_str = ''
    
    def setup_gst_client(self, mode, port, address, callbacks_dict, state_change_callback, proc_output_callback): 
        """
        Starts a GstServer instance. (which holds a milhouse process and communicates with it using IPCP)

        Should throw an error if no success.
        """
        log.debug('GstClient.setup_gst_client '+ str(self))
        self.gst_server = GstServer(mode, port, address, callbacks_dict, state_change_callback, proc_output_callback)
        try:
            self.proc_path, self.args, self.pid = self.gst_server.start_process()
        except GstError, e: 
            log.error('Could not start GST process: ' + e.message)
            raise GstError('Could not start GST process. ' + str(e.message))
        self.version_str = self.gst_server.version_str
        log.debug('GstClient.setup_gst_client done')
        
    def _send_cmd(self, cmd, args=None, callback=None):
        """
        Sends a command to the GstServer using IPCP.
        """
        msg = 'GstClient._send_cmd cmd:'+ cmd
        msg += " args: " + str(args)
        msg += ' handle: ' + str(self)
        log.debug(msg)
        self.gst_server.send_cmd(cmd, args, callback)

    def _add_callback(self, cmd, name=None):
        """
        Adds a callback for incoming IPCP messages.
        (i am not sure about this one)
        """
        log.debug('GstClient._add_callback'+ str(self))
        self.gst_server.add_callback(cmd, name)

    def _del_callback(self, callback=None):
        """
        Deletes a callback for incoming IPCP messages.
        (i am not sure about this one)
        """
        log.debug('GstClient._del_callback'+ str(self))
        self.gst_server.del_callback(callback)

    def stop_process(self):
        """
        Stops the milhouse process calling the GstServer.kill() method.
        """
        log.debug('GstClient.stop_process'+ str(self))
        if self.gst_server.state > 0:
            self.gst_server.kill()

class GstProcessProtocol(protocol.ProcessProtocol):
    """
    Manages a milhouse process and its stdin/stdout streams.
    """
    def __init__(self, server):
        log.debug('GstProcessProtocol.__init__: %s' % str(server))
        self.server = server

    def connectionMade(self):
        """
        Called once the process is started.
        """
        log.debug('GstProcessProtocol.connectionMade: GST process started.')
        reactor.callLater(5, self.check_process)

    def check_process(self):
        """
        After 5 seconds, checks if milhouse is still running.
        Kills it otherwise.
        """
        if self.server.state < RUNNING:
            log.critical('GstProcessProtocol.check_process: The GST process state is not RUNNING.')
            self.server.kill()
            
    def outReceived(self, data):
        """
        Called when text is received from milhouse process stdout
        """
        log.debug('GstProcessProtocol.outReceived server state: %d, data %s' % (self.server.state, str(data) ) )
        lines = data.split('\n')
        for line in lines:
            self.server.process_output_received(line)
        if self.server.state < RUNNING:
            for line in lines:
                if line.strip().startswith("Ver:"):
                    self.server.version_str = line.split("Ver:")[1] 
                if line.strip() == 'READY':
                    self.server.change_state(RUNNING)
                    self.server.connect()
                    break
        else:
            print lines

    def processEnded(self, status):
        """
        Called when the milhouse process has exited.
        """
        self.server.change_state(STOPPED)
        log.debug('GstProcessProtocol.processEnded: GST process ended. Message: %s' % status)

