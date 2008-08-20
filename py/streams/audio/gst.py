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
#from twisted.protocols.basic import LineReceiver

# App imports
#from protocols import ipcp
from streams.stream import AudioStream, Stream
from streams.gst_client import GstClient
from utils import log

log = log.start('info', 1, 0, 'audioGst')

class AudioGst(AudioStream, GstClient):
    """Class streams->audio->gst.AudioGst
    """
    
    def __init__(self, port, address='127.0.0.1'):
        AudioStream.__init__(self)
        GstClient.__init__(self, port, address)
        
        # Add callback for commands coming from GST
#        Stream.gst.add_callback()
            
        
#    @defer.inlineCallbacks
    def get_attr(self, name):
        """        
        name: string
        """
        return getattr(self, name)
#        self._send_cmd('audio_get', name)
#        answer = self._send_cmd('audio_get', name)
#        def got_answer(result):
#            return result
#        answer.addCallback(got_answer)
#        return answer              
    
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
        if hasattr(self, name):
            setattr(self, name, value)
            return True, name, value
        return False, name, value
    
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
    

