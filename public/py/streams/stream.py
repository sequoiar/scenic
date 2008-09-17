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

# App imports

class Streams(object):
    """Class representing a group of media streams, a master stream.
    """

    def __init__(self):
        self.streams = {} 
        self.mode = 'send'
        self.container = None
        self.port = None

    def get_kind(self, stream):
        if isinstance(stream, AudioStream):
            return 'audio'
        elif isinstance(stream, VideoStream):
            return 'video'
        else:
            return None

    def add(self, name, stream):
        kind = self.get_kind(stream)
        if kind:
            dict_name = "_".join([kind, name])
            if dict_name in self.streams:
                return 0
            else:
                self.streams[dict_name] = stream
                return 1
        else:
            return -1

    def delete(self, name, kind):
        dict_name = "_".join([kind, name])
        if dict_name in self.streams:
            del self.streams[dict_name]
            return 1
        else:
            return 0
    
    def rename(self, name, new_name, kind):
        dict_name = "_".join([kind, name])
        if dict_name in self.streams:
            self.streams["_".join([kind, new_name])] = self.streams[dict_name]
            del self.streams[dict_name]
            return 1
        else:
            return 0
    
    def list(self, kind):
        if kind == 'streams':
            streams = [(name.separator('_')[2], stream) for name, stream in self.streams.items()]
            streams.sort() 
            return streams, self.__dict__
        else:
            streams = [(name.separator('_')[2], stream) for name, stream in self.streams.items() if name[0:2] == kind[0] + '_']
            streams.sort() 
            return streams

    def get(self, name, kind):
        dict_name = "_".join([kind, name])
        if dict_name in self.streams:
            return self.streams[dict_name]
        return None
    
    def set_attr(self, name, value):
        """
        name: string
        value: 
        """
        if hasattr(self, name):
            setattr(self, name, value)
            return True, name, value
        return False, name, value
    
    
    
    
    def start(self, address='127.0.0.1'):
        """
        Start all the sub-streams.
                
        address: string or None
        """
        if address:
            self.mode = 'send'
            for stream in self.streams.values():
                stream.start_sending(address)
            return 'Starting sending...'
        else:
            self.mode = 'receive'
            for stream in self.streams.values():
                stream.start_receving()
            return 'Starting receving...'
        
    
    def stop(self, mode='send'):
        """
        Stop all the sub-streams.
        """
        self.mode = mode
        if mode == 'send':
            for stream in self.streams.values():
                stream.stop_sending()
            return 'Stopping sending...'
        else:
            for stream in self.streams.values():
                stream.stop_receving()
            return 'Stopping receving...'
    
    
    
    


class Stream(object):
    """
    Class representing one media stream.
    """

    def __init__(self, core=None):
        self._core = core
        self.port = None  # (int) 
        self.buffer = None  # (int) 
#        self.mode = None  # (string) send or receive
        self._state = 0
    
    def start_sending(self, address=None):
        """ 
        To be overridden by the sub-subclass
               
        address: string
        """
        raise NotImplementedError()
    
    def stop_sending(self):
        """
        To be overridden by the sub-subclass
        """
        raise NotImplementedError()

    def start_receving(self):
        """ 
        To be overridden by the sub-subclass
        """
        raise NotImplementedError()
    
    def stop_receving(self):
        """
        To be overridden by the sub-subclass
        """
        raise NotImplementedError()

    
class AudioStream(Stream):
    """
    Class representing an audio stream
    """
    def __init__(self, core=None):
        Stream.__init__(self, core)
        self.container = None
        self.codec = None
        self.codec_settings = None
        self.bitdepth = 16
        self.sample_rate = 48000
        self.channels = 2

       
class VideoStream(Stream):
    """
    Class representing a video stream
    """
    def __init__(self, core=None):
        Stream.__init__(self, core)
        self.container = None  # () 
        self.codec = None  # () 
        self.codec_settings = None  # () 
        self.width = 640  # (int) 
        self.height = 480  # (int) 

    
class DataStream(Stream):
    """
    Class representing a data stream
    """
    def __init__(self, kind, core=None):
        Stream.__init__(self, core)
        self.kind = kind  # (string) 
    

