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
Communication channel for the streaming/settings infos between two miville contacts.
"""
import sys
from twisted.internet import reactor
import pprint

from miville.utils import log
from miville.utils.commands import find_command
from miville.errors import *
from miville.engines import audiovideogst
from miville.engines.base_gst import GstError
import miville.settings

log = log.start('debug', 1, 0, 'gstchannel')

def _check_for_milhouse():
    try:
        tmp = find_command('milhouse')
    except:
        log.error("milhouse command not found. Please install milhouse. See the documentation.")
    else:
        log.info("Succesfully found the milhouse command.")

reactor.callLater(0, _check_for_milhouse)

# Notitification for stream process state change (1 notif per streaming session)
#  timeout: tbd 5 sec 
#  info: state, contact name, error message
#        states are: stopped, starting, started
#        

_gst_channels_dict = {}
_api = None


def set_api(api):
    global _api
    _api = api
    
# this is totally retard (see WTF !!@)
# def create_channel(engine_name):
#     log.debug('engines.create_channel: ' + str(engine_name) )
#     engine_name = str(engine_name)
#     if engine_name.upper() == 'GST':
#         chan = GstChannel()
#         return chan
#     print 'Don\'t make me work.'
#     raise StreamsError, 'Engine "%s" has no communication channel' %  engine_name

  
def on_com_chan_connected(connection_handle, role="client"):
    """
    Called when a new connection with a contact is made.
    
    Called from the Connector class in its com_chan_started_client or com_chan_started_server method.

    registers the com_chan callback for settings transferts.
    
    :param connection_handle: connectors.Connection object.
    :param role: string "client" or "server"
    We actually do not care if this miville is a com_chan client or server.
    """
    global _api
    global _gst_channels_dict

    log.debug("settings.on_com_chan_connected")
    
    contact = connection_handle.contact
    
    #chan = create_channel('Gst') WTF !!@
    # self.com_chan = None
    # self.contact = None
    # self.api = None
    # self.caller = None
    # self.remote_addr = None    
    chan = GstChannel()
    chan.contact = contact
    chan.com_chan = connection_handle.com_chan
    chan.api = _api
    chan.remote_addr = contact.address
    chan.send_message('SEND_ME_MY_IP', [contact.address])
        
    callback = chan.on_remote_message
    chan.com_chan.add(callback, 'Gst')
    
    _gst_channels_dict[chan.contact.name] = chan
    log.debug("settings.on_com_chan_connected: settings_chans: " + str(_gst_channels_dict))
    

def on_com_chan_disconnected(connection_handle):
    """
    Called when a connection is stopped
    """
    global _gst_channels_dict
    try:
        del _gst_channels_dict[connection_handle.contact.name]
        log.debug("settings.on_com_chan_disconnected: settings_chans: " + str(_gst_channels_dict))
    except KeyError, e:
        log.error("error in on_com_chan_disconnected : KeyError " + e.message)        

def get_all_gst_channels():
    return _gst_channels_dict
    
def get_gst_channel_for_contact(contact):
    return _gst_channels_dict[contact]

def split_gst_parameters(global_setting, address):
    """
    Walks the global setting to harvest all the necessary
    gst parameters that will make their way to milhouse.

    This is called in A's miville. (the initiator)
    This is a VERY IMPORTANT function.

    :param global_setting: GlobalSetting object 
    :param address: ip addr string. The ip of B. 
    """
    receiver_procs = {}
    sender_procs = {}
    log.debug("split_gst_parameters: global setting %s" % global_setting.name)
    log.debug("split_gst_parameters: address of B : %s" % str(address))
    for id, group in global_setting.stream_subgroups.iteritems():
        if group.enabled:
            log.debug("split_gst_parameters group: %s " % group.name)
            # procs is used to select between rx and tx process groups
            procs = receiver_procs
            s = group.mode.upper()
            if s.startswith('SEND'):
                procs = sender_procs
                
            for stream in group.media_streams:
                if stream.enabled:
                    log.debug("split_gst_parameters stream: %s " % stream.name)
                    proc_params = None
                    if not procs.has_key(stream.sync_group):
                        procs[stream.sync_group]  = {}
                    proc_params = procs[stream.sync_group]
                    proc_params = procs[stream.sync_group]
                    # proc_params now points to a valid dict.
                    # get params from media stream
                    media_setting = miville.settings.Settings.get_media_setting_from_id(stream.setting)
                    if media_setting.settings.has_key('engine'):
                        log.debug("split_gst_parameters media setting: %s " % media_setting.name)
                        engine_name =  media_setting.settings['engine']
                        if engine_name.upper().startswith('GST'):
                            params = {}
                            params['port'] = stream.port
                            for k,v in media_setting.settings.iteritems():
                                    params[k] = media_setting.settings[k]
                            params['address'] = address
                            proc_params[stream.name]= params
    return receiver_procs, sender_procs

def _create_stream_engines( listener, mode, procs_params):
    """
    This is where contact B starts streaming to A when A decides it is time to start streaming.

    Uses the communication channel.

    Returns list of new stream engines.    
    AudioVideoGst instances.
    
    VERY IMPORTANT METHOD !
    """
    log.info("gstchannel._create_stream_engines")
    engines = []
    for group_name, sync_group in procs_params.iteritems():
        log.info(" sync group [" + group_name + "]") 
        engine = None
        #for stream_name, stream_params in sync_group.iteritems():
    	stream_names = sync_group.keys()
        stream_names.sort()
    	stream_names.reverse() # we want reverse lexical order for our streams (i.e video before audeo)
        for stream_name in stream_names:
            stream_params = sync_group[stream_name]
            log.info("  stream: " + stream_name)
            engine_name = stream_params['engine']
            if engine == None:
                # engine is a AudioVideoGst instance
                if engine_name.upper() == 'GST':
                    try:
                        engine =  audiovideogst.AudioVideoGst(mode, group_name)
                    except GstError, e:
                        log.error(e.message)
                        raise
                else:
                    raise StreamsError, 'Engine "%s" is not supported' % (engine_name)
            # sends the command to the new Milhouse process.
            engine.apply_stream_settings(stream_name, stream_params)
        engines.append(engine)
    return engines

REMOTE_STREAMING_CMD = "start_streaming"
STOP_RECEIVERS_CMD   = "stop_receivers"
RECEIVERS_STOPPED    = "receivers_stopped"
STOP_SENDERS_CMD     = "stop_senders"

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
        self.my_addr = None    
        
        self.receiver_procs_params = None
        self.sender_procs_params = None
        self.receiver_engines = None
        self.sender_engines = None

    def start_streaming(self, global_setting, contact):
        """
        Host "A" starts the audio/video/data streaming between two miville programs. 
    
        this is where the arguments to the milhouse processes are exchanged. 
        A stream can be of audio or video type. 
        
        first, the settings are browsed and sorted between receiver an sender
        processes (local and remote), according to the streams type and
        sync groups. Then the settings are sent to the engines (pobably GST)
        
        Also, here is called initiate_streaming, which sends a message to 
        B, telling him to (sudo) start to transmit.
        """
        # TODO first making sure the stream is off...
        # self.stop_streaming()
        address = contact.address
        receiver_procs_params, sender_procs_params = split_gst_parameters(global_setting, address)
        connection_basic = self.com_chan.owner
        local_ip = self.my_addr
        # local_ip = None
        # try:
        #     if connection_basic.localhost is not None:
        #         # BasicClient instance
        #         protocol_obj = connection_basic.connection
        #         print 'protocol_obj:', protocol_obj
        #         local_ip = protocol_obj.transport.getHost().host
        #         protocol_obj.localhost = local_ip
        #         connection_basic.localhost = local_ip
        #         print 'local_ip:', local_ip
        # except AttributeError, e:
        #     print e.message
        #     print sys.exc_info()
        # #OLD: local_ip  = connection_basic.localhost

        # print('---------------- gstchannel.start_streaming -------------------')
        log.debug('gstchannel.start_streaming: local_ip: %s, remote_ip: %s' % (local_ip, address))
        # print("AALEXXX ::: address of A: (me) " + str(local_ip))
        # print("AALEXXX ::: address of B: " + str(address))
        # print('-----------------------------------')
        # on brr when brr connects and tzing starts:
        #AALEXXX ::: remote_address: None
        #AALEXXX ::: address: 10.10.10.66

        # XXX FIXME HACK ! aalex was here
        #ORIGINAL form hugo : remote_sender_procs_params, remote_receiver_procs_params = split_gst_parameters(global_setting, remote_address)
        remote_sender_procs_params, remote_receiver_procs_params = split_gst_parameters(global_setting, local_ip)
        #remote_sender_procs_params, remote_receiver_procs_params = split_gst_parameters(global_setting, address)
        # send settings to remote miville
        self.initiate_streaming(receiver_procs_params, sender_procs_params, remote_receiver_procs_params, remote_sender_procs_params, contact.address, contact)

    def initiate_streaming(self, rx_params, tx_params, rx_remote_params, tx_remote_params, contact_addr, contact):
        """
        A initiates the streaming.
        
        Sends a message to B with REMOTE_STREAMING_CMD key.

        Sends it our setting name, as seen from here.
        """
        self.receiver_procs_params = rx_params
        self.sender_procs_params = tx_params
        setting_name = self.api.settings.get_global_setting_from_id(self.contact.setting).name
        try:
            self.start_local_gst_processes(self.receiver_procs_params,  self.sender_procs_params, setting_name)
        except GstError, e:
            self.notify_error(e)
            #raise
        else:
            self.send_message(REMOTE_STREAMING_CMD, [rx_remote_params, tx_remote_params, contact_addr, setting_name])

    def stop_streaming(self, address):
        """
        Stops the audio/video/data streams.
        
        Sends a stop command to the remote milhouse
        For aesthetic reasons, receiver procecess are terminated before the senders 
        Here's the sequence of events:
            local rx processes are stopped
            STOP_RECEIVERS_CMD msg sent to the remote miville
            remote miville stops both rx and tx processes 
            RECEIVERS_STOPPED msg sent back to the local miville
            local miville stops tx processes
        """
        self._stop_local_rx_procs()
        self.send_message(STOP_RECEIVERS_CMD)
        self.notify_stopped()
        
    def _stop_local_rx_procs(self):
        log.info("Stopping rx processes")
        if self.receiver_engines :
            for engine in self.receiver_engines:
                if engine.mode.upper().startswith("RECEIVE"):
                    engine.stop_streaming()
        else:
            log.error("No rx processes to stop")
    
    def _stop_local_tx_procs(self):
        log.info("Stop tx processes")
        if self.sender_engines:
            for engine in self.sender_engines:
                if engine.mode.upper().startswith("SEND"):
                    engine.stop_streaming()        
        else:
            log.error("No tx processes to stop")
        caller = None
        # self.notify_stopped()

    def notify_stopped(self):
        """
        Notifies the API that we stopped streaming
        """
        self.contact.stream_state = 0
        caller = None
        self.api.notify(caller, {'stopped':True, 'msg':"streaming stopped", "contact_name":self.contact.name}, "stop_streams") 
      
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
        if key == "remote_gst_stop":
            self.api.notify(caller, "Got remote stop streaming message")
                
        if key == REMOTE_STREAMING_CMD:
            self.api.notify(caller, "Got remote GST parameters", "info" )
            rx_params = args[0]
            tx_params = args[1]
            remote_ip = args[2] #self.remote_addr
            setting_name = args[3]
            self.api.notify(caller, "Starting GST processes", "info" )
            log.debug("on_remote_message got REMOTE_STREAMING_CMD: will call start_local_gst_processes(%s, %s)" % (str(rx_params), str(tx_params)))
            try:
                self.start_local_gst_processes(rx_params, tx_params, setting_name)
            except GstError, e:
                log.error(e.message)
                self.notify_error(e)
                #def stop_streaming(self, address):
                self.send_message('COULD_NOT_START_STREAMING', [str(self.remote_addr)])

        elif key == 'COULD_NOT_START_STREAMING':
            address = args[0]
            log.error('remote contact could not start streaming.')
            self.contact.stream_state = 0 # IMPORTANT !!!!!!!!!
            self.api.notify(None, GstError("Remote contact could not start streaming."), "start_streams")
            #reactor.callLater(4, self.stop_streaming, address)
            self._stop_local_rx_procs()
            self._stop_local_tx_procs()
        
        elif key == STOP_RECEIVERS_CMD:
            self._stop_local_rx_procs()
            self.send_message(RECEIVERS_STOPPED)
            self._stop_local_tx_procs()
            self.notify_stopped()
            
        elif key == RECEIVERS_STOPPED:
            self._stop_local_tx_procs()
            self.notify_stopped()

        elif key =='SEND_ME_MY_IP':
            self.my_addr = args[0]
            log.debug('XXXXXXXXXXXXXXXXX my ip is ' + str(self.my_addr))
            self.send_message('YOUR_IP_IS', [self.remote_addr])

        elif key =='YOUR_IP_IS':
            self.my_addr = args[0]
            log.debug('XXXXXXXXXXXXXXXXX my ip is ' + str( self.my_addr))
            
        else:
            log.error("Unknown message in settings channel: " + key +  " args: %s"  % str(args) )
    
    def _start_stream_engines(self, engines):
        """
        Let us start streaming
        """
        for engine in engines:
            log.debug('GstChannel._start_stream_engines: ' + str(engine))
            # send IPCP message "start"
            engine.start_streaming()
            
    def start_local_gst_processes(self, rx_params, tx_params, setting_name=''):
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
        try:
            self.receiver_engines = _create_stream_engines(self.api, 'receive', self.receiver_procs_params)
        except GstError, e:
            #self.notify_error(e)
            raise
        else:
            self._start_stream_engines(self.receiver_engines)
            
            log.debug("   GstChannel.on_remote_message: received gst_receiver_params: %s" % str(self.sender_procs_params) )
            log.debug("   Initialize SENDING PROCESSES:")
            log.debug( pprint.pformat(self.sender_procs_params)) 
            self.sender_engines = _create_stream_engines(self.api, 'send', self.sender_procs_params)
            self._start_stream_engines(self.sender_engines)
            self.notify_started(setting_name)

    def notify_started(self, setting_name=''):
        """
        Notifies the observers that we started to stream.
        """
        caller = None
        self.api.notify(caller, {
            'started':True, 
            'contact_name':self.contact.name,
            'setting_name':setting_name,
            'msg':"streaming started"
            }, "start_streams") 
        self.contact.stream_state = 2 # IMPORTANT !

    def notify_error(self, exc, caller=None):
        """
        Notifies the api with start_streams key and an exception as data.
        """
        self.api.notify(caller, exc, "start_streams")
            
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

