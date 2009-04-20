# -*- coding: utf-8 -*-

# Miville
# # # # Copyright (C) 2008 Société des arts technologiques (SAT)
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

# System imports
import os, resource, signal, time, sys, select

# Twisted imports
from twisted.internet import reactor, protocol, defer, abstract
from twisted.python import procutils


# App imports
from miville.protocols import ipcp
from miville.utils import log
from miville.utils.common import get_def_name

log = log.start('info', 1, 0, 'base_gst')

STOPPED = 0
STARTING = 1
RUNNING = 2
CONNECTING = 3
CONNECTED = 4
STREAMSTOPPED = 5
STREAMINIT = 6
STREAMING = 7



streaming_server_process_name = "milhouse"

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

    def __init__(self, mode, port, address): #address='127.0.0.1'
        log.info("GstServer.__init__(mode = %s, port=%d, address=%s)" % (mode, port, address))
        self.process = None
        self.conn = None
        self.state = -1
        self.change_state(STOPPED)
        self.mode = mode
        self.port = port
        self.address = address
        self.commands = []

    def connect(self):
        log.debug("GstServer.connect")
        if self.state == RUNNING:
            self.change_state(CONNECTING)
            deferred = ipcp.connect(self.address, self.port)
            deferred.addCallback(self.connection_ready)
            deferred.addErrback(self.connection_failed)

    def change_state(self, new_state):
         old_state = self.state
         self.state = new_state
         
         all_states = {-1:'NOTHING', STOPPED:'STOPPED', STARTING:'STARTING', RUNNING:'RUNNING', 
                   CONNECTING:'CONNECTING', CONNECTED:'CONNECTED', STREAMSTOPPED: 'STREAMSTOPPED',
                   STREAMINIT:'STREAMINIT', STREAMING:'STREAMING'}
         log.info('GstServer state changed from %s to %s handle: %s ' % (all_states[old_state], all_states[new_state], str(self) ))

    def connection_ready(self, ipcp):
        log.info("CONNECTION ready %s" % str(self))
        msg = 'GstServer.connection_ready: Address: %s, Port: %s, conn %s' % (self.address, self.port, str(ipcp) )
        log.info(msg)
        log.info('CONN: %s %s' % (ipcp, ipcp.__dict__))
        self.conn = ipcp
        self.conn.connectionLost = self.connection_lost
        # Our GST keywords
        self.conn.add_callback(self.gst_log, 'log')
        self.conn.add_callback(self.gst_video_init, 'video_init')
        self.conn.add_callback(self.gst_audio_init, 'audio_init')
        self.conn.add_callback(self.gst_start, 'start')
        self.conn.add_callback(self.gst_audio_init, 'stop')
        self.conn.add_callback(self.gst_success, 'success')
        self.conn.add_callback(self.gst_failure, 'failure')
        self.conn.add_callback(self.gst_rtp, 'rtp')
        
        self.change_state(CONNECTED)
        log.info('GST inter-process link created')
        
    def gst_video_init(self, **args):
        log.info('GST VIDEO INIT acknowledged:  args %s %s' %  (str(args), str(self))  )
        self.change_state(STREAMINIT)

    def gst_start(self,  **args):
        log.info('GST START acknowledged... our args %s  %s' % (str(args), str(self)) )
        self.change_state(STREAMING)
        
    def gst_stop(self,  **args):
        log.info('GST STOP acknowledged ... our args %s  %s' % (str(args), str(self)) )
        self.change_state(STREAMSTOPPED)

    def gst_audio_init(self,  **args):
        log.info('GST AUDIO INIT acknowledged our args %s  %s' % (str(args), str(self)) )
        self.change_state(STREAMINIT)

    def gst_success(self, **args):
        log.info('GST success :-)  args %s %s' %  (str(args), str(self)) )
        self.change_state(STREAMINIT)

    def gst_failure(self, **args):
        log.debug('GST failure :-(  args %s %s' %  (str(args), str(self)) )
        self.change_state(STREAMINIT)

    def gst_log(self, **args):
        log.info('GST log  [%s] %s' %  (str(args), str(self)) )

    def gst_rtp(self, **args):
        log.info('GST RTP stats  [%s] %s' %  (str(args), str(self)) )


    def connection_failed(self, conn):
        msg = 'GstServer.connection_failed: Address: %s, Port: %s %s' % (self.address, self.port, str(self))
        log.info(msg + " "  + str(self))
        ipcp.connection_failed(conn)
        self.kill()

    def connection_lost(self, reason=protocol.connectionDone):
        self.kill()
        self.conn.del_callback('log')
        log.info('Lost the server connection. Reason:\n%s %s' % (reason, str(self) ) )

    def start_process(self):
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
                    log.info(msg)
                    self.process = reactor.spawnProcess(protocol, proc_path, args, os.environ, usePTY=True)
                    log.info("Process spawned pid=" + str(self.process.pid)  + " "+ str(self))
                    
                except Exception, e:
                    log.critical('Cannot start the GST application "%s": %s %s' % (streaming_server_process_name, str(e), str(self) ) )
            else:
                log.critical('Cannot find the GST application: %s %s' % streaming_server_process_name, str(self) )
        else:
            log.error("GstServer.start_process: not in proper state for starting %s" % str(self))

    def kill(self):
        log.debug("GstServer.kill %s" % str(self))
        self.change_state(STOPPED)
#        os.kill(self._process.pid, signal.SIGTERM)
        reactor.callLater(0.5, self.verify_kill)
        try:
            self.process.loseConnection()
        except:
            log.debug('Process %s or connection seems already dead. %s' % str(self.process.pid) ,str(self))
#        self.process = None

    def verify_kill(self):
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
        log.debug('.')   
        if len(self.commands) > 0:
            if self.conn != None:
                if self.state >= CONNECTED :
                    (cmd, args) = self.commands.pop()
                    log.info('\n\n\nGstServer._process_cmd: \n  ' + cmd + "\n  args: " + str(args) + "\n  pid " + str(self.process.pid)  + " " + str(self) + '\n\n')
                    if args != None:
                        self.conn.send_cmd(cmd, *args)
                    else:
                        self.conn.send_cmd(cmd)
        
        if len(self.commands) > 0:
            if self.state > STOPPED:
                reactor.callLater(1, self._process_cmd)
        
        

    def send_cmd(self, cmd, args=None, callback=None, timer=None, timeout=3):
        log.debug('GstServer.send_cmd state: ' + str(self.state) + " cmd: " +  cmd + " args: " + str(args)+ "pid " + str(self.process.pid) + " " + str(self) )
        self.commands.insert(0, (cmd,args) )
        self._process_cmd()



    def del_callback(self, callback=None):
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
        log.debug('GstServer.add_callback ' + str(self) )
        if self.conn:
            self.conn.add_callback(cmd, name)
        else:
            curr_time = time.time()
            if not timer:
                timer = curr_time
            if curr_time - timer < 3:
                delay = 0.001
                delay = 0.5
                reactor.callLater(delay, self.add_callback, cmd, name, timer)
            else:
                log.critical('The GST process cannot be ready to connect (from add_callback).' + str(self))



class GstClient(BaseGst):
    
    def __init__(self):
        log.debug("GstClient __init__  " + str(self))
    
    def setup_gst_client(self, mode, port, address): # self, api, mode, port,address='127.0.0.1'
        log.debug('GstClient.setup_gst_client '+ str(self))
        self._gst = GstServer(mode, port, address)
        self._gst.start_process()
        log.debug('GstClient.setup_gst_client done')
        
 
    def _send_cmd(self, cmd, args=None, callback=None):
        msg = 'GstClient._send_cmd cmd:'+ cmd
        msg += " args: " + str(args)
        msg += ' handle: ' + str(self)
        log.debug(msg)
        self._gst.send_cmd(cmd, args, callback)

    def _add_callback(self, cmd, name=None):
        log.debug('GstClient._add_callback'+ str(self))
        self._gst.add_callback(cmd, name)

    def _del_callback(self, callback=None):
        log.debug('GstClient._del_callback'+ str(self))
        self._gst.del_callback(callback)

    def stop_process(self):
        log.debug('GstClient.stop_process'+ str(self))
        if self._gst.state > 0:
            self._gst.kill()


class GstProcessProtocol(protocol.ProcessProtocol):
    
    # add command stack
    # add state info
    
    def __init__(self, server):
        log.debug('GstProcessProtocol.__init__: %s' % str(server))
        self.server = server

    def connectionMade(self):
        log.info('GstProcessProtocol.connectionMade: GST process started.')
        reactor.callLater(5, self.check_process)
        

    def check_process(self):
        if self.server.state < RUNNING:
            log.critical('GstProcessProtocol.check_process: The GST process cannot be ready to connect.')
            self.server.kill()
            
    def outReceived(self, data):
        log.debug('GstProcessProtocol.outReceived server state: %d, data %s' % (self.server.state, str(data) ) )
        log.info('GstProcessProtocol.outReceived: "%s" from %s' % (data, str(self)) )
        if self.server.state < RUNNING:
            lines = data.split('\n')
            for line in lines:
                if line.strip() == 'READY':
                    self.server.change_state(RUNNING)
                    self.server.connect()
                    break    

    def processEnded(self, status):
        self.server.change_state(STOPPED)
        log.info('GstProcessProtocol.processEnded: GST process ended. Message: %s' % status)







