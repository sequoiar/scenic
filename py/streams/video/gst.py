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
from errors import *

log = log.start('debug', 1, 0, 'videoGst')

class AudioVideoGst(GstClient):
         
    def apply_settings(self, listener, mode, stream_name, parameters ):
       
       gst_parameters = []
       
       for k,v in parameters.iteritems():
           if k not in ['engine','gst_address','gst_port']:
               gst_parameters.append( (k, v) )      
       
       log.debug('AudioVideoGst.apply_settings ' + str(gst_parameters))
       gst_address = parameters['gst_address']
       gst_port = parameters['gst_port']
       
       self.setup_gst_client(mode, gst_port, gst_address)
       if stream_name.upper().startswith('VIDEO'):
           self._send_cmd('video_init', gst_parameters)
       elif stream_name.upper().startswith('AUDIO'):
           self._send_cmd('audio_init', gst_parameters)
       else:
          raise StreamsError, 'Engine AudioVideoGst does not support "%s" parameters' %  stream_name 

    def start_streaming(self):
        """function start_sending
         address: string
        """
        self._send_cmd('start')

    def stop_streaming(self):
        """function stop_sending
        """
        self._send_cmd('stop')
        self.stop_process()
        