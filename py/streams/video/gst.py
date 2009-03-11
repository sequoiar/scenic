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
from protocols.ipcp import parse 

log = log.start('debug', 1, 0, 'videoGst')

class VideoGst(GstClient):
    """Class streams->video->gst.VideoGst
    """
    
    def apply_settings(self, listener, mode, stream_port , codec,  bitrate, source, address, gst_port, gst_address ):    
        self.listener = listener
        self.codec = codec
        self.stream_port = stream_port
        self.gst_port = gst_port
        self.address = address
        self.source = source
        self.bitrate = bitrate
        self.setup_gst_client(mode, self.gst_port, gst_address)
         
        attrs = []
        if codec:
            att  = ('codec', self.codec)
            attrs.append(att)
        if stream_port:    
            att = ('port', self.stream_port)
            attrs.append(att)
        if address:
            att = ('address', self.address)
            attrs.append(att)
        if bitrate:
            att = ('bitrate', bitrate)
            attrs.append(att)
        if source:
            att = ('source', self.source)
            attrs.append(att)
        self._send_cmd('video_init', attrs)

 
    def start_streaming(self):
        """function start_sending
         address: string
        """
        self._send_cmd('start')

    def notify(self, state, message):
        log.info("Notify")
        self.listener.notify(state, message)
        
    def sending_started(self):
        self._del_callback()
                
    def stop_streaming(self):
        """function stop_sending
        """
        self._send_cmd('stop')
        self.stop_process()
    
    def sending_stopped(self, state):
        self._del_callback()
        self._notify(None, state, 'video_sending_stopped')
   
    def start_receiving(self, channel):
        """function start_receiving"""
        attrs = self.get_attrs()
        self._send_cmd('start', attrs)
    
    def stop_receiving(self, state):
        """function stop_receiving    
        """
        self._del_callback()
        self._notify(self, state, 'video_receiving_stopped')
    

