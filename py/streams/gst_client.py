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

log = log.start('debug', 1, 0, 'gst')


INIT = 0
START = 1
PROC = 2
CONN = 3

class BaseGst(object):            
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
    def __init__(self, mode, port, address='127.0.0.1'):
        self.mode = mode
        self.port = port
        self.address = address
        self.process = None
        self.conn = None
        self.state = INIT

    def connect(self):
        deferred = ipcp.connect(self.address, self.port)
        deferred.addCallback(self.connection_ready)
        deferred.addErrback(self.connection_failed)
            
    def connection_ready(self, conn):
        self.conn = conn
        self.conn.connectionLost = self.connection_lost
        self.conn.add_callback(self.gst_log, 'log')
        self.conn.add_callback(self.gst_levels, 'levels')
        self.state = CONN
        log.info('GST inter-process link created')
        
    def connection_failed(self, conn):
        ipcp.connection_failed(conn)
        self.kill()
        log.debug('Address: %s | Port: %s' % self.address, self.port)
#        Stream.gst_state = 0
#        log.info('Trying to reconnect...')
#        self.connect()

    def connection_lost(self, reason=protocol.connectionDone):
        self.kill()
        self.conn.del_callback('log')
        log.info('Lost the server connection. Reason:\n%s' % reason)
#        log.info('Trying to reconnect...')
#        self.connect()

    def start_process(self):
        self.state = 2    # Uncomment this line to start the GST process "by hand"
        if self.state < 1:
            self.state = START
            gst_app = 'mainTester'
            path = procutils.which(gst_app)
#            path = ['/home/etienne/workspace/miville/trunk/public/py/protocols/ipcp_server.py']
            if path:
                if self.mode == 'send':
                    mode_arg = "1"
                else:
                    mode_arg = "0"
                try:
                    print 'STARTING PROCESS'
                    self.process = reactor.spawnProcess(GstProcessProtocol(self), path[0], [path[0], mode_arg, str(self.port)], usePTY=True)
                except:
                    log.critical('Cannot start the GST application: %s' % gst_app)
            else:
                log.critical('Cannot find the GST application: %s' % gst_app)
        elif self.state == 2:
            self.connect()

    def kill(self):
        self.state = INIT
#        os.kill(self._process.pid, signal.SIGTERM)
        reactor.callLater(0.5, self.verify_kill)
        try:
            self.process.loseConnection()
        except:
            log.debug('Process %s or connection seem already dead.' % self.process.pid)
#        self.process = None

    def verify_kill(self):
        try:
            status = os.waitpid(self.process.pid, 0)            
        except:
            log.debug('Process %s already kill' % self.process.pid)
        else:
            if status[1] != 0:
                os.kill(self.process.pid, signal.SIGKILL)

    def send_cmd(self, cmd, args=None, callback=None, timer=None, timeout=3):
        if self.state < 3:
            curr_time = time.time()
            if not timer:
                timer = curr_time
                self.start_process()
            if curr_time - timer < timeout:
                reactor.callLater(0.001, self.send_cmd, cmd, args, callback, timer)
            else:
                log.critical('The GST process cannot be ready to connect (from send).')
        else:
            if callback:
                log.debug('Callback: %s' % repr(callback))
                self.conn.add_callback(*callback)
            if args:
                self.conn.send_cmd(cmd, *args)
            else:
                self.conn.send_cmd(cmd)

    def del_callback(self, callback=None):
        if self.state == CONN:
            if callback:
                self.conn.del_callback(callback)
            else:
                try:
                    self.conn.del_callback(get_def_name())
                except:
                    log.debug("No callback to delete. (coming from: %s)." % get_def_name())

    def gst_levels(self, values):
        log.info(values)

    def gst_log(self, level, msg):
        msg = 'From GST: %s' % msg.partition(': ')[2].strip()
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
            
        


class GstClient(BaseGst):    
    def __init__(self, mode, port, address='127.0.0.1'):
        self._mode = mode
        if not hasattr(Stream, 'gst_' + mode):
            setattr(Stream, 'gst_' + mode, GstServer(mode, port, address))
        self._gst = getattr(Stream, 'gst_' + mode)

    def _send_cmd(self, cmd, args=None, callback=None):
        self._gst.send_cmd(cmd, args, callback)

    def _del_callback(self, callback=None):
        self._gst.del_callback(callback)
                    
    def stop_process(self):
        if self._gst.state > 0:
            self._gst.kill()
                        

            
            
class GstProcessProtocol(protocol.ProcessProtocol):
    def __init__(self, server):
        self.server = server
        
    def connectionMade(self):
        log.info('GST process started.')
        reactor.callLater(2, self.check_process)

    def check_process(self):
        if self.server.state < 2:
            log.critical('The GST process cannot be ready to connect.')
            self.server.kill()

    def outReceived(self, data):
        if self.server.state < 2:
            lines = data.split('\n')
            log.debug('DATA FROM CHILD: %s' % lines)
            for line in lines:
                if line.strip() == 'READY':
                    self.server.state = PROC
                    self.server.connect()
                    break
           
    def processEnded(self, status):
        self.server.state = INIT
        log.info('GST process ended. Message: %s' % status)            
            
           
            
            
            
            
            
