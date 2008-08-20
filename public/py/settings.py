#!/usr/bin/env python
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
from streams import audio, video, data
from streams.stream import AudioStream, VideoStream
from utils import log
from utils.i18n import to_utf

log = log.start('info', 1, 0, 'settings')

class Settings(object):
    def __init__(self, current=None):
        self.settings = {}
        self.current = current
#        self.select(self.current)
            

    def read(self):
        pass
    
    def write(self):
        pass

    def select(self, name=None):
        if name:
            if name in self.settings:
                self.current = name
                return self.settings[name]  # not good, should create the instance (serialize)
            else:
                log.warning('No setting have this name: %s.' % name)
        return Setting('Custom')


class Setting(object):
    def __init__(self, name):
        self.name = name
        self.streams = {}
        self.contact = None
        self.kinds = {'audio': AudioStream,
                      'video': VideoStream}
        
    def add_stream(self, name, kind):
        if name in self.streams:
            return 0
        else:
            if kind == 'audio':
                self.streams[name] = audio.gst.AudioGst(22222)
                return 1
            elif kind == 'video': 
                self.streams[name] = video.gst.VideoGst(22222)
                return 1
#            elif kind == 'data'
#                self.streams[name] = data
            else:
                return -1
        
    def delete_stream(self, name, kind):
        if kind in self.kinds:
            if name in self.streams:
                klass = self.kinds[kind]
                if isinstance(self.streams[name], klass):
                    del self.streams[name]
                    return 1
            else:
                return 0
        return -1
    
    def rename_stream(self, name, new_name, kind):
        if kind in self.kinds:
            if name in self.streams:
                klass = self.kinds[kind]
                if isinstance(self.streams[name], klass):
                    self.streams[new_name] = self.streams[name]
                    del self.streams[name]
                    return 1
            else:
                return 0
        return -1
    
    def list_stream(self, kind):
        streams = {}
        if kind in self.kinds:
            klass = self.kinds[kind]
            for stream in self.streams:
                if isinstance(self.streams[stream], klass):
                    streams[stream] = self.streams[stream]
        else:
            log.warning('%s is not a valid type of stream.' % kind)
        return streams

    def get_stream(self, name, kind):
        if name in self.streams:
            if kind in self.kinds:
                klass = self.kinds[kind]
                if isinstance(self.streams[name], klass):
                    return self.streams[name]
        return None







