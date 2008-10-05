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
from streams.stream import AudioStream, Stream
from streams.gst_client import GstClient
from utils import log

log = log.start('debug', 1, 0, 'audioGst')

class AudioGst(AudioStream, GstClient):
    """Class streams->audio->gst.AudioGst
    """
    
    def __init__(self, core):
        AudioStream.__init__(self, core)
        setting = core.curr_setting.others['gst']
        mode = core.curr_setting.streams[core.api.curr_streams].mode
        if mode == 'send':
            port = setting['port_s']
            address = setting['addr_s']
        else:
            port = setting['port_r']
            address = setting['addr_r']
        GstClient.__init__(self, mode, port, address)
        self._chan = None
        
    def get_attr(self, name):
        """        
        name: string
        """
        return getattr(self, name)
    
    def get_attrs(self):
        return [(attr, value) for attr, value in self.__dict__.items() if attr[0] != "_"]
    
    def set_attr(self, name, value):
        """
        name: string
        value: 
        """
        if hasattr(self, name):
            setattr(self, name, value)
            return True, name, value
        return False, name, value
    
    def start_sending(self, address, channel):
        """function start_sending
        address: string
        """
        self._chan = channel
        attrs = self.get_attrs()
        attrs.append(('address', address))
        self._send_cmd('audio_start', attrs, (self.sending_started, 'caps'))
        
    def sending_started(self, caps_str):
        self._del_callback('caps')
        if caps_str.isdigit():
            self._core.notify(None, caps_str, 'audio_sending_started')
        else:
            self._core.notify(None, 1, 'audio_sending_started')
            log.debug('SHOULD SEND CAPS VIA TCP HERE!')
            self._chan.callRemote('AudioGst.caps', caps_str)
            
    def stop_sending(self):
        """function stop_sending
        """
        self._send_cmd('stop_audio', None, self.sending_stopped)
    
    def sending_stopped(self, state):
        self._del_callback()
        self._core.notify(None, state, 'audio_sending_stopped')
   
    def start_receving(self, channel):
        """function start_receving
        """
        self._chan = channel
        self._chan.add(self.caps)
    
    def caps(self, caps):
        self._chan.delete(self.caps)
        attrs = self.get_attrs()
        attrs.append(('caps', caps))
        self._send_cmd('audio_start', attrs)
   
    def receiving_started(self, answer):
        log.info('Return value after sending caps: %s.' % answer)
        self._del_callback()
        if answer:
            self._core.notify(None, 1, 'audio_receiving_started')
            
    def stop_receving(self):
        """function stop_receving
        
        returns 
        """
        return None # should raise NotImplementedError()

    
def start(core):
    return AudioGst(core)
