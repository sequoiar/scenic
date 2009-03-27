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

from utils import log
from errors import *
import pprint
import audiovideogst

log = log.start('debug', 1, 0, 'gstchannel')

def _create_stream_engines( listener, mode, procs_params):
    """
    Returns list of new stream engines. 
    
    AudioVideoGst instances.
    """
    engines = []
    for group_name, sync_group in procs_params.iteritems():
        log.debug(" sync group: " + group_name) 
        engine = None
        for stream_name, stream_params in sync_group.iteritems():
            log.debug("  stream: " + stream_name)
            engine_name = stream_params['engine']
            if engine == None:
                # engine is a AudioVideoGst instance
                if engine_name.upper() == 'GST':
                    engine =  audiovideogst.AudioVideoGst()
                else:
                    raise StreamsError, 'Engine "%s" is not supported' %  engine_name

            engine.apply_settings(listener, mode, stream_name, stream_params)
        engines.append(engine)
    return engines


class GstChannel(object):
    """
    Allows to send and receive setting information
    via the com_chan protocol, once we are joined to
    a remote contact
    """
    def __init__(self):
        log.debug('GstChannel.__init__: ' + str(self))
        self.com_chan = None
        self.contact = None
        self.api = None
        self.caller = None
        self.remote_addr = None    
        
        self.receiver_procs_params = None
        self.sender_procs_params = None
        self.receiver_engines = None
        self.sender_engines = None
        
    def on_remote_message(self, key, args=None):
        """
        Called by the com_chan whenever a message is received from the remote contact 
        through the com_chan with the "pinger" key.
        
        :param key: string defining the action/message
        :param *args: list of arguments
        
        The key and *args can be one of the following...
        """
        caller = None
        log.debug('GstChannel.on_remote_message: %s : %s' %  (key, str(args) )  )
            
        if key == "remote_gst_params":
            self.api.notify(caller, "Got remote params", "info" )
            rx_params = args[0]
            tx_params = args[1]
            self.start_local_gst_processes(rx_params, tx_params )
            
        else:
            log.error("Unknown key in settings channel: " + key +  " args: %s"  % str(args) )
    
    def _start_stream_engines(self, engines):
        """
        Let us start streaming
        """
        for engine in engines:
            log.debug('GstChannel._start_stream_engines: ' + str(engine))
            engine.start_streaming()
            
    def stop_streaming(self):
        pass # TODO 
        
    def start_local_gst_processes(self, rx_params, tx_params):
        """
        creates processses and send init messages
        """
        log.debug("GstChannel.start_local_gst_processes")
        self.receiver_procs_params = rx_params
        self.sender_procs_params = tx_params
        log.debug("   RX params :" + str(rx_params))
        log.debug("   TX params :" + str(tx_params))

        log.debug("   Initialize RECEIVING PROCESSES:")
        log.debug( pprint.pformat(self.receiver_procs_params)) 
        self.receiver_engines = _create_stream_engines(self.api, 'receive', self.receiver_procs_params)
        self._start_stream_engines(self.receiver_engines)
        
        log.debug("   GstChannel.on_remote_message: received gst_receiver_params: %s" % str(self.sender_procs_params) )
        log.debug("   Initialize SENDING PROCESSES:")
        log.debug( pprint.pformat(self.sender_procs_params)) 
        self.sender_engines = _create_stream_engines(self.api, 'send', self.sender_procs_params)
        self._start_stream_engines(self.sender_engines)        
        
            
    def send_message(self, key, args_list=[]):
        """
        Sends a message to the current remote contact through the com_chan with key "network_test".
        :param args: list
        """
        log.debug("GstChannel._send_message %s. %r" % (key, args_list))
        try:
            # list with key as 0th element
            self.com_chan.callRemote('Gst', key, args_list)
        except AttributeError, e:
            log.error("Could not send message to remote: " + e.message)
