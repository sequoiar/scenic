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


class Streams(object):
    """Class representing a group of media streams, a master stream.
    """
    def __init__(self):
        self.streams = {} 
        self.mode = None
    
    def start(self, address=None):
        """
        Start all the sub-streams.
                
        address: string or None
        """
        if address:
            self.mode = 'local'
        else:
            self.mode = 'remote'
        for stream in self.streams:
            self.streams[stream].start(address)
    
    def stop(self):
        """
        Stop all the sub-streams.
        """
        for stream in self.streams:
            self.streams[stream].stop()
    
    def add(self, name, stream):
        """
        Add a media stream to the master stream.
                
        name: string
        stream: stream
        """
        self.streams[name] = stream
    
    def remove(self, name):
        """
        Add a media stream to the master stream.

        name: string
        """
        del self.streams[name]
    
    def status(self):
        """
        Return the global status (started or not) and the status for each
        sub-stream (settings).
        """
        return None
    


class Stream(object):
    """
    Class representing one media stream.
    """

    def __init__(self):
        self.port = None  # (int) 
        self.buffer = None  # (int) 
        self.mode = None  # (string) 
    
    def start(self, address=None):
        """ 
        To be overridden by the sub-subclass
               
        address: string
        """
        raise NotImplementedError()
    
    def stop(self):
        """
        To be overridden by the sub-subclass
        """
        raise NotImplementedError()

    
class AudioStream(Stream):
    """
    Class representing an audio stream
    """
    def __init__(self):
        Stream.__init__(self)
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
    def __init__(self):
        Stream.__init__(self)
        self.container = None  # () 
        self.codec = None  # () 
        self.codec_settings = None  # () 
        self.width = 640  # (int) 
        self.height = 480  # (int) 

    
class DataStream(Stream):
    """
    Class representing a data stream
    """
    def __init__(self, kind):
        Stream.__init__(self)
        self.kind = kind  # (string) 
    

