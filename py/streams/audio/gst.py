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

# App imports
from protocols.osc_protocol import Osc, OscMessage
from streams.stream import AudioStream, Stream


class AudioGst(AudioStream):
    """Class streams->audio->gst.AudioGst
    """
    
    def __init__(self, s_port, r_port, address='127.0.0.1'):
        AudioStream.__init__(self)
        self.s_port = s_port
        self.address = address
        if not hasattr(Stream, 'osc'):
            Stream.osc = Osc()
            reactor.listenUDP(r_port, Stream.osc)
        
    def _send_message(self, name, value=None):
        message = OscMessage()
        message.setAddress('/audio/%s' % name)
        message.append(value)
        Stream.osc.send_message(self.address, self.s_port, message)
        
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
    

