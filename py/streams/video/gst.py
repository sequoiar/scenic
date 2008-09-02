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
from streams.stream import VideoStream, Stream
from streams.gst_client import GstClient
from utils import log

log = log.start('info', 1, 0, 'videoGst')

class VideoGst(VideoStream, GstClient):
    """Class streams->video->gst.VideoGst
    """
    
    def __init__(self, port, address='127.0.0.1', core=None):
        VideoStream.__init__(self, core)
        GstClient.__init__(self, port, address)
            
    def get_attr(self, name):
        """        
        name: string
        """
        return getattr(self, name)
    
    def set_attr(self, name, value):
        """
        name: string
        value: 
        """
        if hasattr(self, name):
            setattr(self, name, value)
            return True, name, value
        return False, name, value
    
    def start_sending(self, address):
        """function start_sending
        address: string
        """
        attrs = [(attr, value) for attr, value in self.__dict__.items() if attr[0] != "_"]
        self._send_cmd('start_video', self.sending_started, ('address', address), *attrs)
        
    def sending_started(self, caps):
        self._del_callback()
        if caps.isdigit():
            self._core.notify(None, caps, 'video_sending_started')
        else:
            self._core.notify(None, 1, 'video_sending_started')
#            log.info('SHOULD SEND CAPS VIA TCP HERE!')
   
    def stop_sending(self):
        """function stop_sending
        """
        self._send_cmd('stop_video', self.sending_stopped)
    
    def sending_stopped(self, state):
        self._del_callback()
        self._core.notify(None, state, 'video_sending_stopped')
   
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
    

