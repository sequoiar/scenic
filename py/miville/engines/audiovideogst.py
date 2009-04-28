# -*- coding: utf-8 -*-

# Miville
# Copyright (C) 2008 Société des arts technologiques (SAT)
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


# Twisted imports
from twisted.internet import reactor, protocol, defer

# App imports
from miville.engines.base_gst import GstClient
from miville.utils import log
from miville.protocols.ipcp import parse 
from miville.errors import *
from miville.utils.common import PortNumberGenerator


gst_ipcp_port_gen = PortNumberGenerator(9000, 1)
gst_av_port_gen = PortNumberGenerator(10000, 10)

log = log.start('debug', 1, 0, 'videoGst')

class AudioVideoGst(GstClient):
         
    def apply_settings(self, listener, mode, group_name, stream_name, parameters ):
       self.mode = mode
       self.group_name = group_name
       self.stream_name = stream_name
       self.commands = []
       
       gst_parameters = []
       for k,v in parameters.iteritems():
           if k not in ['engine','gst_address','gst_port']:
               gst_parameters.append( (k, v) )      
       log.debug('AudioVideoGst.apply_settings group:' + str(group_name) + ' stream: ' + str(stream_name) + ' params: ' + str(gst_parameters))
       gst_address =  '127.0.0.1' # parameters['gst_address']
       gst_port =  gst_ipcp_port_gen.generate_new_port() # parameters['gst_port']
              
       self.setup_gst_client(mode, gst_port, gst_address)
       if stream_name.upper().startswith('VIDEO'):
           self._send_command('video_init', gst_parameters)
       elif stream_name.upper().startswith('AUDIO'):
           self._send_command('audio_init', gst_parameters)
       else:
          raise StreamsError, 'Engine AudioVideoGst does not support "%s" parameters' %  stream_name 

    def _send_command(self, cmd, params):
        self._send_cmd(cmd, params)
        tupple_of_fine_stuff = (cmd, params)
        self.commands.append(tupple_of_fine_stuff)

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
        