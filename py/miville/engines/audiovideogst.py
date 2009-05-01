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
"""
Everything that should be used by a programmer from the miville.engines packages 
is in this engines.audiovideogst module. 

This modules controls the milhouse processes according to the 
settings that miville has set up. 
"""

# Twisted imports
from twisted.internet import reactor, protocol, defer

# App imports
from miville.engines.base_gst import GstClient
from miville.utils import log
from miville.protocols.ipcp import parse 
from miville.errors import *
from miville.utils.common import PortNumberGenerator
import miville.engines.base_gst
from datetime import datetime

gst_ipcp_port_gen = PortNumberGenerator(9000, 1)
gst_av_port_gen = PortNumberGenerator(10000, 10)

log = log.start('debug', 1, 0, 'videoGst')

class RingBuffer:
    """
    For RTP statistics and log messages

    Keeps "size" number of strings.
    """
    def __init__(self, size):
        self.data = [None for i in xrange(size)]

    def append(self, x):
        self.data.pop(0)
        self.data.append(x)

    def get(self):
        return self.data

class AudioVideoGst(GstClient):
    """
    Interface to the milhouse process.
    """
    def apply_settings(self, listener, mode, group_name, stream_name, parameters ):
       self.mode = mode
       self.group_name = group_name
       self.stream_name = stream_name
       self.commands = []
       
       self.args = None
       self.proc_path = None
       self.pid = None
       self.rtp_stats = {}
       self.logger = RingBuffer(20)
            
       gst_parameters = []
       for k,v in parameters.iteritems():
           if k not in ['engine','gst_address','gst_port']:
               gst_parameters.append( (k, v) )      
       log.debug('AudioVideoGst.apply_settings group:' + str(group_name) + ' stream: ' + str(stream_name) + ' params: ' + str(gst_parameters))
       gst_address =  '127.0.0.1' # parameters['gst_address']
       gst_port =  gst_ipcp_port_gen.generate_new_port() # parameters['gst_port']
       
       callbacks = {'log'       :self.gst_log,
                    'video_init':self.gst_video_init, 
                    'audio_init':self.gst_audio_init, 
                    'start'     :self.gst_start, 
                    'stop'      :self.gst_audio_init,
                    'success'   :self.gst_success,
                    'failure'   :self.gst_failure,
                    'rtp'       :self.gst_rtp 
                    }
              
       self.setup_gst_client(mode, gst_port, gst_address, callbacks)
       if stream_name.upper().startswith('VIDEO'):
           self._send_command('video_init', gst_parameters)
       elif stream_name.upper().startswith('AUDIO'):
           self._send_command('audio_init', gst_parameters)
       else:
          raise StreamsError, 'Engine AudioVideoGst does not support "%s" parameters' %  stream_name 

    def get_status(self):
        s = self.gst_server.get_status()
        return s
        
    def _send_command(self, cmd, params = None):
        self._send_cmd(cmd, params)
        tupple_of_fine_stuff = (cmd, params)
        self.commands.append(tupple_of_fine_stuff)

    def gst_video_init(self, **args):
        log.info('GST VIDEO INIT acknowledged:  args %s %s' %  ( str(args), str(self))  )
        self.gst_server.change_state(miville.engines.base_gst.STREAMINIT)

    def gst_start(self,  **args):
        log.info('GST START acknowledged... our args %s  %s' % (str(args), str(self)) )
        self.gst_server.change_state(miville.engines.base_gst.STREAMING)
        
    def gst_stop(self,  **args):
        log.info('GST STOP acknowledged ... our args %s  %s' % (str(args), str(self)) )
        self.gst_server.change_state(miville.engines.base_gst.STREAMSTOPPED)

    def gst_audio_init(self,  **args):
        log.info('GST AUDIO INIT acknowledged our args %s  %s' % (str(args), str(self)) )
        self.gst_server.change_state(miville.engines.base_gst.STREAMINIT)

    def gst_success(self, **args):
        log.info('GST success :-)  args %s %s' %  (str(args), str(self)) )
        self.gst_server.change_state(miville.engines.base_gst.STREAMING)

    def gst_failure(self, **args):
        log.debug('GST failure :-(  args %s %s' %  (str(args), str(self)) )
        self.gst_server.change_state(miville.engines.base_gst.FAILURE)

    def gst_log(self, **args):
        log_message = (str(args))
        src = str(self)
        info = 'GST log pid %s [%s] %s' %  (self.pid, log_message, src)  
        log.info(info ) 
        self.logger.append(log_message)

    def gst_rtp(self, **args):
        if args.has_key('stats'):
            rtp_string = args['stats']
            tokens = rtp_string.split(':')
            if len(tokens) == 3:
                stream = tokens[0]
                stat = tokens[1]
                value = tokens[2]
                self.set_rtp_stats(stream, stat, value)
    
    def set_rtp_stats(self, stream, stat, value):
        timestamp = datetime.now()
        data = (timestamp, value)
        if not self.rtp_stats.has_key(stream):
            self.rtp_stats[stream] = {}
        if not self.rtp_stats[stream].has_key(stat):
            self.rtp_stats[stream][stat] = RingBuffer(10)
            
        self.rtp_stats[stream][stat].append( data )
        
    def start_streaming(self):
        """function start_sending
         address: string
        """
        self._send_command('start')

    def stop_streaming(self):
        """function stop_sending
        """
        self._send_command('stop')
        self.stop_process()
        
