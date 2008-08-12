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
from twisted.internet import reactor
from twisted.protocols.basic import LineReceiver

# App imports
from protocols import icpc
from streams.stream import AudioStream, Stream
from utils import log

log = log.start('info', 1, 0, 'audioGst')

class AudioGst(AudioStream):
    """Class streams->audio->gst.AudioGst
    """
    
    def __init__(self, port, address='127.0.0.1'):
        AudioStream.__init__(self)
        self.s_port = s_port
        self.address = address
        if not hasattr(Stream, 'gst'):
            deferred = icpc.connect(address, port)
            deferred.addCallback(self.connectionReady)
            
    def connectionReady(self, gst):
        Stream.gst = gst
        log.info('GST inter-process link created')
            
    def _unhandled_message(self, path, types, data, addr):
        log.info('Unhandled OSC message comming from %s:%s with this address: %s' % (addr[0], addr[1], path))
        
    def _container(self, path, types, data, addr):
        if data == 'ACK':
            self.callbacks['container'] = 1
        elif:
            self.callbacks['container'] = 0
        
    def _send_message(self, name, value=None):
        message = OscMessage()
        message.setAddress('/audio/%s' % name)
        message.append(value)
        Stream.osc.send_message(self.address, self.s_port, message)
        self.callbacks[name] = 0
#        reactor.callLater(0.5, self.check_ack, name)
#        
#    def check_ack(self, name):
#        if self.callbacks[name]:
#            log.warning('Did not receive the ACK for %s' % name)
#            self.callbacks[name] = 0
            
        
    def get_attr(self, name):
        """        
        name: string
        """
        self._send_message(name)        
    
    def get_attrs(self):
        """function get_attrs
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def set_attr(self, name, value):
        """
        name: string
        value: 
        """
        self._send_message(name, value)        
    
    def set_attrs(self):
        """function set_attrs
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def start_sending(self, address):
        """function start_sending
        
        address: string
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def stop_sending(self):
        """function stop_sending
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def start_receving(self):
        """function start_receving
        
        returns 
        """
        return None # should raise NotImplementedError()
    
    def stop_receving(self):
        """function stop_receving
        
        returns 
        """
        return None # should raise NotImplementedError()
    

