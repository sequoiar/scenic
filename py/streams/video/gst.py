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

log = log.start('debug', 1, 0, 'videoGst')

class VideoGst(VideoStream, GstClient):
    """Class streams->video->gst.VideoGst
    """
    
    def __init__(self, core=None):
        VideoStream.__init__(self, core)
        setting = core.curr_setting.others['gst']
        self.port = setting['port'] + 10
        self.codec = setting['vcodec']
        mode = core.curr_setting.streams[core.api.curr_streams].mode
        if mode == 'send':
            port = setting['port_s']
            address = setting['addr_s']
        else:
            port = setting['port_r']
            address = setting['addr_r']
        GstClient.__init__(self, mode, port, address)
#        self._chan = None
            
    def start_sending(self, address, channel):
        """function start_sending
        address: string
        """
#        self._chan = channel
        attrs = self.get_attrs()
        attrs.append(('address', address))
        self._send_cmd('video_start', attrs)
        
    def sending_started(self):
        self._del_callback()
#        if caps.isdigit():
#            self._core.notify(None, caps, 'video_sending_started')
#        else:
#            self._core.notify(None, 1, 'video_sending_started')
#            log.info('SHOULD SEND CAPS VIA TCP HERE!')
   
    def stop_sending(self):
        """function stop_sending
        """
        self._send_cmd('video_stop')
        self.stop_process()
    
    def sending_stopped(self, state):
        self._del_callback()
        self._core.notify(None, state, 'video_sending_stopped')
   
    def start_receving(self, channel):
        """function start_receving"""
        attrs = self.get_attrs()
        self._send_cmd('video_start', attrs)
    
    def stop_receving(self, state):
        """function stop_receving
        
        returns 
        """
        self._del_callback()
        self._core.notify(self, state, 'video_receving_stopped')
    
def start(core):
    return VideoGst(core)
