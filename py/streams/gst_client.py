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

# Twisted imports
from twisted.internet import reactor, protocol, defer

# App imports
from protocols import ipcp
from streams.stream import AudioStream, Stream
from utils import log

log = log.start('info', 1, 0, 'gstClient')


class GstClient(object):    
    def __init__(self, port, address='127.0.0.1'):
        self._gst_port = port
        self._gst_address = address
        if not hasattr(Stream, 'gst') or not Stream.gst_state:
            self.connect()
            
    def connect(self):
        deferred = ipcp.connect(self._gst_address, self._gst_port)
        deferred.addCallback(self.connection_ready)
        deferred.addErrback(self.connection_failed)
            
    def connection_ready(self, gst):
        Stream.gst = gst
        Stream.gst.connectionLost = self.connection_lost
        Stream.gst_state = 1
        log.info('GST inter-process link created')
        
    def connection_failed(self, gst):
        ipcp.connection_failed(gst)
        Stream.gst_state = 0
#        log.info('Trying to reconnect...')
#        self.connect()

    def connection_lost(self, reason=protocol.connectionDone):        
        Stream.gst_state = 0
        log.info('Lost the server connection. Reason:\n%' % reason)
#        log.info('Trying to reconnect...')
#        self.connect()

    def _send_cmd(self, cmd, callback=None, *args):
        if not Stream.gst_state:
            self.connect()  # Should add verification if the GST process is running
            reactor.callLater(0.5, self._send_cmd, cmd, *args)
        else:
            if callback:
                Stream.gst.add_callback(callback)
            Stream.gst.send_cmd(cmd, *args)

    def _del_callback(self, callback):
        if hasattr(Stream, 'gst'):
            Stream.gst.del_callback(callback)
                    
            
            
            
            
            
            
            
            
            
            
            
            
            
            
