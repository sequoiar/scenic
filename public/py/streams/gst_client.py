# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Sropulpof is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sropulpof.  If not, see <http:#www.gnu.org/licenses/>.

# System imports
import os
import signal
import time

# Twisted imports
from twisted.internet import reactor, protocol, defer
from twisted.python import procutils

# App imports
from protocols import ipcp
from streams.stream import AudioStream, Stream
from utils import log, get_def_name

log = log.start('debug', 1, 0, 'gstClient')


INIT = 0
PROC = 1
CONN = 2

class GstClient(object):    
    def __init__(self, mode, port, address='127.0.0.1'):
        if not hasattr(Stream, 'gst_' + mode):
            self._gst_port = port
            self._gst_address = address
            self._mode = mode
            setattr(Stream, 'gst_' + mode, True)
            self.gst = None
            self.state(INIT)
            
    def connect(self):
        deferred = ipcp.connect(self._gst_address, self._gst_port)
        deferred.addCallback(self.connection_ready)
        deferred.addErrback(self.connection_failed)
            
    def connection_ready(self, gst):
        setattr(Stream, 'gst_' + self._mode, gst)
        setattr(Stream, 'gst_' + self._mode + '.connectionLost', self.connection_lost)
        self.gst = getattr(Stream, 'gst_' + self._mode)
        self.gst.add_callback(self.gst_log, 'log')
        self.state(CONN)
        log.info('GST inter-process link created')
        
    def connection_failed(self, gst):
        ipcp.connection_failed(gst)
        self._process.kill()
        log.debug('Address: %s | Port: %s' % self._gst_address, self._gst_port)
#        Stream.gst_state = 0
#        log.info('Trying to reconnect...')
#        self.connect()

    def connection_lost(self, reason=protocol.connectionDone):
        self._process.kill()
        self.gst.del_callback('log')
        log.info('Lost the server connection. Reason:\n%s' % reason)
#        log.info('Trying to reconnect...')
#        self.connect()

    def _send_cmd(self, cmd, args, callback=None, timer=None, timeout=3):
        if self.state() < 2:
            curr_time = time.time()
            if not timer:
                timer = curr_time
                self._process = self.start_process()
            if curr_time - timer < timeout:
                reactor.callLater(0.001, self._send_cmd, cmd, args, callback, timer)
            else:
                log.critical('The GST process cannot be ready to connect (from send).')
        else:
            if callback:
                log.debug('Callback: %s' % repr(callback))
                self.gst.add_callback(*callback)
            self.gst.send_cmd(cmd, *args)

    def _del_callback(self, callback=None):
        if self.gst:
            if callback:
                self.gst.del_callback(callback)
            else:
                try:
                    self.gst.del_callback(get_def_name())
                except:
                    log.debug("No callback to delete. (coming from: %s)." % get_def_name())
                    
    def start_process(self):
        if self.state() < 1:
            gst_app = 'mainTester'
            path = procutils.which(gst_app)
#            path = ['/home/etienne/workspace/miville/trunk/public/py/protocols/ipcp_server.py']
            if path:
                if self._mode == 'send':
                    mode_arg = "1"
                else:
                    mode_arg = "0"
                try:
                    self._process = reactor.spawnProcess(GSTProcessProtocol(self), path[0], [path[0], mode_arg, str(self._gst_port)], usePTY=True)
                except:
                    log.critical('Cannot start the GST application: %s' % gst_app)
            else:
                log.critical('Cannot find the GST application: %s' % gst_app)
        else:
            self.connect()
                        
    def state(self, new_state=None):
        if new_state is None:
            return getattr(Stream, 'gst_state_' + self._mode)
        else:
            setattr(Stream, 'gst_state_' + self._mode, new_state)
            
    def gst_log(self, level, msg):
        msg = 'From GST: %s' % msg
        if level == 10:
            log.debug(msg)
        elif level == 30:
            log.warning(msg)
        elif level == 40:
            log.error(msg)
        elif level == 50:
            log.critical(msg)
        else:
            log.info(msg)
            
            
class GSTProcessProtocol(protocol.ProcessProtocol):
    def __init__(self, client):
        self.timer = 0
        self.client = client
        
    def connectionMade(self):
        self.timer = time.time()
        log.info('GST process started.')
        reactor.callLater(2, self.check_process)

    def check_process(self):
        if self.client.state() < 1:
            log.critical('The GST process cannot be ready to connect.')
            self.kill()

    def outReceived(self, data):
        if self.client.state() < 1:
            lines = data.split('\n')
            log.debug('DATA FROM CHILD: %s' % lines)
            for line in lines:
                if line.strip() == 'READY':
                    self.client.state(PROC)
                    self.client.connect()
                    break
#        self.transport.loseConnection()
           
    def processEnded(self, status):
        log.info('GST process ended. Message: %s' % status)            
            
    def kill(self):
        self.client.state(INIT)
#        os.kill(self.transport.pid, signal.SIGTERM)
        pid = self.transport.pid
        reactor.callLater(0.5, self.verify_kill, pid)
        self.transport.loseConnection()

    def verify_kill(self, pid):            
        try:
            if os.waitpid(pid, 0)[1] != 0:
                os.kill(self.transport.pid, signal.SIGKILL)
        except:
            log.debug('Process %s already kill' % pid)
           
            
            
            
            
            
            
