#!/usr/bin/env python
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
This modules controls the milhouse processes according to the 
settings that miville has set up. 

Everything that should be used by a programmer from the miville.engines packages 
is in this engines.audiovideogst module. 
"""
# system imports
from datetime import datetime

# Twisted imports
from twisted.internet import defer
from twisted.internet import reactor
from twisted.internet import protocol 

# App imports
from miville.engines.base_gst import GstClient
from miville.utils import log
from miville.protocols.ipcp import parse 
from miville.errors import StreamsError
from miville.engines.base_gst import GstError
from miville.utils.common import PortNumberGenerator
from miville.utils import stack
from  miville.engines import base_gst # for constants.

gst_ipcp_port_gen = PortNumberGenerator(9000, 1)
gst_av_port_gen = PortNumberGenerator(10000, 10)

log = log.start('info', 1, 0, 'audiovideogst')

class RingBuffer(object):
    """
    FIFO buffer for RTP statistics and log messages

    Keeps "size" number of strings.

    TODO: get rid of this buggy class that can lead to data loss. See the append method.
    """
    # TODO
    def __init__(self, size):
        """
        Creates a list of size elements that are None.
        """
        self.data = [None for i in xrange(size)]

    def append(self, x):
        """
        Removes the first element in the list and adds the provided element at the end of the list.
        FIXME: some data loss can occur here !!!
        """
        # FIXME
        self.data.pop(0)
        self.data.append(x)

    def get(self):
        """
        Returns the whole list of items in the buffer.
        """
        return self.data

class AudioVideoGst(GstClient):
    """
    Interface to the milhouse process. All the IPCP commands are here.
    
    GstClient is a simple class which starts a milhouse process and communicates with is using IPCP.
    Here, all the actual IPCP commands used to control milhouse are defined. 
    """
    def __init__(self, mode, group_name):
        """
        :param mode: XXX ?
        :param group_name: I think this is for when you want the audio to be in sync or not with the video.
        """
        self.mode = mode
        self.group_name = group_name
        self.stream_names = []
        self.commands = []
        self.args = None
        self.proc_path = None
        self.pid = None
        self.rtp_stats = {}
        self.logger = RingBuffer(20)
        self.output_logger = RingBuffer(20)
        self.version = ""
        self.gst_address =  '127.0.0.1'
        self.gst_port = gst_ipcp_port_gen.generate_new_port()
        self.acknowledgments = [] # list of (timestamp, string) tuples
        callbacks = {'log'       :self.gst_log,
                     'video_init':self.gst_video_init, 
                     'audio_init':self.gst_audio_init, 
                     'start'     :self.gst_start, 
                     'stop'      :self.gst_audio_init,
                     'success'   :self.gst_success,
                     'failure'   :self.gst_failure,
                     'rtp'       :self.gst_rtp 
                     }       
        try:
            self.setup_gst_client(mode, self.gst_port, self.gst_address, callbacks, self.gst_state_change_callback, self.gst_proc_output)
        except GstError, e:
            log.error(e.message)
            raise
        #self._send_command('loglevel', 10)
        
    def apply_stream_settings(self, stream_name, parameters ):
        """
        Sends commands to the milhouse process applying the setting.
        
        VERY IMPORTANT METHOD !
        """
        self.stream_names.append(stream_name)
        gst_parameters = []
        non_gst_params = ['engine']
        for k,v in parameters.iteritems():
            if k not in non_gst_params:
                gst_parameters.append( (k, v) )      
            log.info('AudioVideoGst.apply_stream_settings stream: ' + str(stream_name) + ' params: ' + str(gst_parameters))
        if stream_name.upper().startswith('VIDEO'):
           self._send_command('video_init', gst_parameters)
        elif stream_name.upper().startswith('AUDIO'):
           self._send_command('audio_init', gst_parameters)
        else:
           raise StreamsError, 'Engine AudioVideoGst does not support "%s" parameters' %  stream_name 
      
    def get_status(self):
        """
        Returns status (string?)
        """
        s = self.gst_server.get_status()
        return s
    
    def get_version_str(self):
        """
        Returns version of the IPCP protocol??? 
        """
        s = self.gst_server.version_str
        return s
        
    def _send_command(self, cmd, params = None):
        """
        Wraps GstClient._send_cmd. Sends a IPCP command.
        """
        self._send_cmd(cmd, params)
        tupple_of_fine_stuff = (cmd, params) # FIXME Change this variable name
        self.commands.append(tupple_of_fine_stuff)

    def gst_video_init(self, **args):
        # TODO: explicit arguments !
        timestamp = datetime.now()
        info = 'video_init  "%s"' % str(args)
        self.acknowledgments.append((timestamp, info))
        log.debug('%s %s' %  (info, str(self)) )        
        self.gst_server.change_state(base_gst.STREAMINIT)

    def gst_start(self,  **args):
        # TODO: explicit arguments !
        timestamp = datetime.now()
        info = 'start  "%s"' % str(args)
        self.acknowledgments.append((timestamp, info))
        log.debug('%s %s' %  (info, str(self)) )
        self.gst_server.change_state(base_gst.STREAMING)
        
    def gst_stop(self,  **args):
        # TODO: explicit arguments !
        timestamp = datetime.now()
        info = 'stop  "%s"' % str(args)
        self.acknowledgments.append((timestamp, info) )
        log.debug('%s %s' %  (info, str(self)) )
        self.gst_server.change_state(base_gst.STREAMSTOPPED)

    def gst_audio_init(self,  **args):
        # TODO: explicit arguments !
        timestamp = datetime.now()
        info = 'audio_init  "%s"' % str(args)
        self.acknowledgments.append((timestamp, info))
        log.debug('%s %s' %  (info, str(self)) )
        self.gst_server.change_state(base_gst.STREAMINIT)

    def gst_success(self, **args):
        # TODO: explicit arguments !
        timestamp = datetime.now()
        info = 'Success :-)  "%s"' % str(args)
        self.acknowledgments.append((timestamp, info))
        log.debug('%s %s' %  (info, str(self)) )
        self.gst_server.change_state(base_gst.STREAMING)

    def gst_failure(self, **args):
        # TODO: explicit arguments !
        timestamp = datetime.now()
        info = 'failure :-(  "%s"' % str(args)
        self.acknowledgments.append((timestamp, info))
        log.debug('%s %s' %  (info, str(self)) )
        self.gst_server.change_state(base_gst.FAILURE)

    def gst_log(self, **args):
        # TODO: explicit arguments !
        log_message = (str(args))
        src = str(self)
        info = 'GST log pid %s [%s] %s' %  (self.pid, log_message, src)  
        log.debug(info ) 
        self.logger.append(log_message)

    def gst_rtp(self, **args):
        # TODO: explicit arguments !
        if args.has_key('stats'):
            rtp_string = args['stats']
            tokens = rtp_string.split(':')
            if len(tokens) == 3:
                stream = tokens[0]
                stat = tokens[1]
                value = tokens[2]
                self.set_rtp_stats(stream, stat, value)
    
    def gst_proc_output(self, log_message):
        src = str(self)
        info = 'GST output: pid %s [%s] %s' %  (self.pid, log_message, src)  
        log.debug(info ) 
        self.output_logger.append(log_message)
    
    def gst_state_change_callback(self, old_state, new_state, old_str, new_str):
        timestamp = datetime.now()
        info = 'GST state change  "%s" to "%s"' % (old_str, new_str)
        self.acknowledgments.append((timestamp, info))
    
    def set_rtp_stats(self, stream, stat, value):
        timestamp = datetime.now()
        data = (timestamp, value)
        if not self.rtp_stats.has_key(stream):
            self.rtp_stats[stream] = {}
        if not self.rtp_stats[stream].has_key(stat):
            self.rtp_stats[stream][stat] = RingBuffer(10)
            
        self.rtp_stats[stream][stat].append( data )
        
    def start_streaming(self):
        """
        Sends the "start" message to milhouse so that it starts streaming. 
        Milhouse must be already running.
        """
        log.debug('audiovideogst.start_streaming()')
        self._send_command('start')
        #print '\nAudioVideoGst.start_streaming() stack :'
        #stack.print_stack() # XXX this is very verbose !

    def stop_streaming(self):
        """
        Sends the "stop" command to milhouse.
        """
        self._send_command('stop')
        self.stop_process()

        
