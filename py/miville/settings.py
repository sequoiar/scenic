#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
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
Audio/video/data settings handling for miville. (configuration)

State Saving
============
Settings are saved in the ~/.miville/settings.txt file.
there is a dict. all settings with id under 10000 are presets. Those with ID over
10000 are user settings. (changed by the user)

Streaming initialization 
========================
In get_init_params, we turn the media settings into the arguments for the 
sropulpof process. 

Types of Settings
=================
 * A Media Setting is related to a audio or video stream
 * A global setting is linked to a contact in the addressbook. 
 * A devices setting has to deal with the harware configuration in the devices packages.
 * A preset is how we call a default factory setting. 

Design Notes
============
 * Stream subgroup instances contain media streams. (audio and video)

Usage
=====
In the telnet UI, type "s --load" to load settings
"""

import os
import re # used when reading settings file
import pprint


from miville.utils import log
from miville.utils.i18n import to_utf
from miville.errors import *
from miville import connectors
from miville.utils.common import install_dir
# persistence is not futile
from miville.engines import create_channel

from twisted.spread.jelly import jelly, unjelly
from twisted.internet import reactor

log = log.start('info', 1, 0, 'settings')

# These file names are overriden by Miville's Core !
PRESETS_FILENAME = "presets.txt"
SETTINGS_FILENAME = "settings.txt"

# All settings with id under 10000 are presets. Those with ID over
# 10000 are user settings. (changed by the user)
first_global_id = 10000
first_media_setting = 10000

# The currently supported file version
current_major_version_number = 1 # TODO: replace with __version__ 

# Global (module scope) variables for the GstChannel instances.
global_settings = {}
media_settings = {}


_api = None
_settings_channels_dict = {}


  
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
    global _settings_channels_dict
    global COM_CHAN_KEY

    log.debug("settings.on_com_chan_connected")
    
    contact = connection_handle.contact
    
    chan = create_channel('Gst')
    chan.contact = contact
    chan.com_chan = connection_handle.com_chan
    
    callback = chan.on_remote_message
    chan.com_chan.add(callback, 'Gst')
    chan.api = _api
    chan.remote_addr = contact.address
        
    _settings_channels_dict[chan.contact.name] = chan
    log.debug("settings.on_com_chan_connected: settings_chans: " + str(_settings_channels_dict))
    
def init_connection_listeners(api):
    """
    Registers the callbacks to the com_chan. 

    This function must be called from the API.
    """
    global _api
    _api = api
    log.debug("settings.init_connection_listeners")
    connectors.register_callback("settings_on_connect", on_com_chan_connected, event="connect")
    connectors.register_callback("settings_on_disconnect", on_com_chan_disconnected, event="disconnect")

def on_com_chan_disconnected(connection_handle):
    """
    Called when a connection is stopped
    """
    global _settings_channels_dict
    try:
        del _settings_channels_dict[connection_handle.contact.name]
        log.debug("settings.on_com_chan_disconnected: settings_chans: " + str(_settings_channels_dict))
    except KeyError, e:
        log.error("error in on_com_chan_disconnected : KeyError " + e.message)        

def get_settings_channel_for_contact(contact):
    return _settings_channels_dict[contact]

  
class Settings(object):
    """
    Settings handling utilities. 
    
    Contains method/function to : 
     * read the presets from file
     * merge them with user settings
     * etc.

    Settings instances contain global settings and media settings.
    quote: 'Use defaults, Luke!', 0B1knob ... a long time ago
    """
    def __init__(self):
        self.global_settings = global_settings
        self.selected_global_setting = None
        self.selected_media_setting = None
        #self.current_global_id = 0  [i for i in l if i<20]
        self.media_settings = media_settings
        self.load() # RTF File
        
        
    @staticmethod
    def get_media_setting_from_id(id):
        if media_settings.keys().__contains__(id):
            return media_settings[id]
        raise SettingsError, 'The media setting id "' + str(id) + '" does not exist' 

    @staticmethod
    def _load_nice_object_from_file(filename, major_version):
        """
        this 'private' method loads an object from a text file, using the 
        nice representation (dict with line breaks and tabs)
        """
        object = {}
        try:
            file = open(filename, 'r')
        except IOError, e:
            log.info('error reading settings file "%s" : %s' % (filename,e))
        else:
            version = re.findall('v(\d+)\.(\d+)', file.readline())
            if version:
                d1 = version[0][0]
                d2 = version[0][1]
                major = int(d1)
                minor = int(d2)
                if major == current_major_version_number:
                    try:
                        data =  file.read().replace('\n', '').replace('\t', '')
                        evaldata = eval(data)
                        object = unjelly(evaldata)
                    except:
                        log.warning('Unable to read the setting file "%s". Bad format.' % filename)
            else:
                file.seek(0)
                try:
                    object = pickle.load(file)
                except:
                    log.warning('Unable to read the setting file "%s". Bad format.' % filename)
    
            file.close()
            return object
        
    @staticmethod    
    def _save_object_to_file_nicely(object, filename, major_version):
        """
        This 'private' method saves an object to a text file, using the 
        nice representation (jelly text with line breaks and tabs)
        """
        try:
            file = open(filename, 'w')
            log.info('Saving settings to file: ' + filename)
        except:
                msg = 'Could not open the file %s.' % filename
                log.warning(msg)
                raise SettingsError, msg
        else:
            if major_version == current_major_version_number:
                log.info("Saving object jelly: " + str(object) )
                jello = jelly(object)
                dump = repr(jello)
                level = 0
                nice_dump = ['v1.0\n']
                for char in dump:
                    if char == '[':
                        if level > 0:
                            nice_dump.append('\n' + '\t' * level)
                        level += 1
                    elif char == ']':
                        level -= 1
                    nice_dump.append(char)
                what_to_write = ''.join(nice_dump)    
                file.write(what_to_write)
            else:
                pickle.dump(object, file, 1)
                file.close()    

    def load(self):
        """
        Loads presets from the settings state saving file.
        
        Default location of this file is ~/.miville/presets.txt
        If not present, it loads presets.txt
        """
        global PRESETS_FILENAME
        global SETTINGS_FILENAME

        presets_file_name       = install_dir(PRESETS_FILENAME) #"presets.txt")
        user_settings_file_name = install_dir(SETTINGS_FILENAME) # "settings.txt")
        
        # settings are saved as tuple()
        presets = {}
        user_settings = {}
        
        # we should expect presets settings to exist, but however
        # we will ramp it up slowly
        if os.path.exists(presets_file_name):
            log.info("Loading object jelly from: " + presets_file_name)
            presets = Settings._load_nice_object_from_file(presets_file_name, current_major_version_number)
        
        # user settings file is optional...
        if os.path.exists(user_settings_file_name):
            log.info("Loading object jelly from: " + user_settings_file_name)
            user_settings = Settings._load_nice_object_from_file(user_settings_file_name, current_major_version_number)
        
        # load presets and then user global settings
        if presets:
            presets_globals = presets[0]
            self.global_settings.clear()
            self.selected_global_setting = None
            for k, glob in presets_globals.iteritems():
                 self.global_settings[k] = glob
                     
        if user_settings:   
            user_global_settings = user_settings[0] 
            for k, glob in user_global_settings.iteritems():
                # check for duplicates
                if not self.global_settings.has_key(k):
                    self.global_settings[k] = glob
                else:
                    log.error("Global user setting %d clashes with preset" % k)
    
        # load presets and user media settings
        if presets:
            presets_media_settings = presets[1]
            self.media_settings.clear()
            self.selected_media_setting = None
            for k, med in presets_media_settings.iteritems():
                self.media_settings[k] = med
                
        if user_settings:
            user_media_settings = user_settings[1]
            for k, med in user_media_settings.iteritems():
                # check for duplicates
                if not self.media_settings.has_key(k):
                    self.media_settings[k] = med
                else:
                    log.error("Media user setting %d: '%s' clashes with preset" % (k,med.name))
                 
    def save(self):
        """
        Saves the current settings to a file. 
        """
        user_media_settings = {} 
        user_global_settings =  {}
        
        first_user_setting_id = first_global_id
        
        for k in self.global_settings.keys():
            glob = self.global_settings[k]
            if k >= first_user_setting_id: 
                user_global_settings[k] = glob
        
        for k in self.media_settings.keys():
            med = self.media_settings[k]
            if k >= first_user_setting_id:
                user_media_settings[k] = med
        
        filename = install_dir(SETTINGS_FILENAME) # "settings.txt")
        
        # little hack to  create a preset file
        stuff = (user_global_settings, user_media_settings)  
        Settings._save_object_to_file_nicely(stuff, filename, current_major_version_number)

    def get_media_setting(self, name):
        id = self._get_media_setting_id_from_name(name)
        return self.media_settings[id]
    
    def get_global_setting_from_id(self, id):
        if self.global_settings.has_key(id):
            return self.global_settings[id]
        raise SettingsError, 'The global setting ' + str(id) + ' does not exist'  
    
    def get_global_setting(self, name):
        id = self._get_global_setting_id_from_name(name)
        return self.global_settings[id]
    
    def  pretty_list_settings(self):
         return (self.global_settings, self.media_settings)
        
    def list_global_setting(self):
         return (self.global_settings, self.selected_global_setting)
       
    def add_global_setting(self, name):
        if not name:
            raise SettingsError, 'Global setting must have a unique name'
        if self._get_global_setting_id_from_name_no_fail(name) >= 0:
            raise SettingsError, 'Global setting "%s" already exists' % name
        try:
            glob = GlobalSetting(name)
            new_id = first_global_id
            if len(self.global_settings) > 0: 
                new_id = max(self.global_settings.keys()) + 1
                if new_id < first_global_id:
                    new_id = first_global_id
            self.global_settings[new_id] = glob
        finally:
            pass 
        return True
    
    def erase_global_setting(self, name):
        id = self._get_global_setting_id_from_name(name)
        if self.selected_global_setting == name:
            self.selected_global_setting = None
        del self.global_settings[id]
       
    def modify_global_setting(self, global_setting_name, attribute, new_value):
        id = self._get_global_setting_id_from_name(global_setting_name)
        raise SettingsError, 'settings.py l 92 MODIFY : ' + global_setting_name + " " + attribute + " = " + new_value  
        return True
       
    def duplicate_global_setting(self, name):
        raise SettingsError, 'This command is not implemented yet' 
        return True

    def _get_media_setting_id_from_name(self, name):
        for id in self.media_settings.keys():
            setting_name = self.media_settings[id].name
            if  setting_name == name:
                return id
        raise SettingsError, 'Media setting "%s" does not exist' % name
    
    def _get_global_setting_id_from_name_no_fail(self, name):
        for id in self.global_settings.keys():
            setting_name = self.global_settings[id].name
            if  setting_name == name:
                return id
        return -1
    
    def _get_global_setting_id_from_name(self, name):
        id = self._get_global_setting_id_from_name_no_fail(name)
        if id == -1:
            raise SettingsError, 'Global setting "%s" does not exist' % name
        return id

    def select_global_setting(self, name):
        id = self._get_global_setting_id_from_name(name)
        self.selected_global_setting = name
        return True
            
    def select_global_setting(self, name):
        id = self._get_global_setting_id_from_name(name)
        self.selected_global_setting = name
        return True
       
       
    def description_global_setting(self, name):
        raise SettingsError, 'This command is not implemented yet' 
        return True

    def list_media_setting(self):
         return (self.media_settings, self.selected_media_setting)
       
    def add_media_setting(self, name):
        if not name:
            raise SettingsError, 'Media setting must have a name'
        for media in self.media_settings.values():
            if media.name == name: 
                raise SettingsError, 'Media setting "%s" already exists' % name
        # the nex id is either first_media_setting or, if there are
        # items already, the next possible id that's equal or higher than the 
        # first usr id
        id = first_media_setting
        if len(self.media_settings) > 0:
            id = max(self.media_settings.keys()) + 1
            if id < first_media_setting:
                id = first_media_setting
                
        new_setting = MediaSetting(name, id)
        self.media_settings[new_setting.id] = new_setting
        return new_setting. id
    
    def erase_media_setting(self, name):
        id = self._get_media_setting_id_from_name(name)
        if id == None:
            raise SettingsError, 'Media setting "%s" does not exist' % name
        if self.selected_media_setting == name:
            self.selected_media_setting = None
        del self.media_settings[id]

    def select_media_setting(self, name):
        id = self._get_media_setting_id_from_name(name)
        self.selected_media_setting = name
        return True


def split_gst_parameters(global_setting, address):
    
    receiver_procs = {}
    sender_procs = {}
    
    for id, group in global_setting.stream_subgroups.iteritems():
        if group.enabled:
            # procs is used to select between rx and tx process groups
            procs = receiver_procs
            s = group.mode.upper()
            if s.startswith('SEND'):
                procs = sender_procs
                
            for stream in group.media_streams:
                if not isinstance(stream, VideoStream):
                    log.info("split_gst_parameters: stream " + stream.name + " is not a GST stream..." )
                else:
                    if stream.enabled:
                        proc_params = None
                        if not procs.has_key(stream.sync_group):
                            procs[stream.sync_group]  = {}
                            
                        proc_params = procs[stream.sync_group]
                        proc_params = procs[stream.sync_group]
                        # proc_params now points to a valid dict.

                        # get params from media stream
                        params = {}
                        media_setting = Settings.get_media_setting_from_id(stream.setting)
                        params['port'] = stream.port
                        for k,v in media_setting.settings.iteritems():
                                params[k] = media_setting.settings[k]
                        
                        
                        params['address'] = address
                        proc_params[stream.name]= params
    return receiver_procs, sender_procs
    
class GlobalSetting(object):
    """
    Global setting instances contain a list of stream subgroups (see StreamSubGroup class).
    subgroups, in turn, contain streams settings (see MediaStream class)
    A global setting is the first level of settings...
    """
    def __init__(self, name):
        self.is_preset = False
        self.stream_subgroups = {}
        self.selected_stream_subgroup = None
        self.name = name
        self.communication = ''
        log.info("GlobalSetting__init__ " + name)
    
    def start_streaming(self, listener, address, settings_channel):
        """
        Starts the audio/video/data streaming between two miville programs. 

        this is where the arguments to the milhouse processes are exchanged. 
        A stream can be of audio or video type. 
        
        first, the settings are browsed and sorted between receiver an sender
        processes (local and remote), according to the streams type and
        sync groups. Then the settings are sent to the engines (pobably GST)
        """
        # TODO first making sure the stream is off...
        # self.stop_streaming()
        receiver_procs_params, sender_procs_params = split_gst_parameters(self, address)
        
        remote_address  = settings_channel.com_chan.owner.localhost
        log.debug("REMOTE ADDRESS: " + str(remote_address) )
        remote_sender_procs_params, remote_receiver_procs_params = split_gst_parameters(self, remote_address)
        # send settings to remote miville
        settings_channel.initiate_streaming(receiver_procs_params, sender_procs_params, remote_receiver_procs_params, remote_sender_procs_params)
        
    def stop_streaming(self, address, settings_channel):
        """
        Stops the audio/video/data streams.
        
        Sends a stop command to the remote milhouse
        For aesthetic reasons, receiver procecess are terminated before the senders 
        """
        settings_channel.terminate_streaming()
        
        
    def _get_stream_subgroup_id_from_name(self, name):
        for id in self.stream_subgroups.keys():
            setting_name = self.stream_subgroups[id].name
            if  setting_name == name:
                return id
        raise SettingsError, 'Stream subgroup "%s" does not exist' % name
    
    def get_stream_subgroup(self, name):
        id = self._get_stream_subgroup_id_from_name(name)
        return self.stream_subgroups[id]
    
    def modify_global_setting(self, prop, value):
        modify('self', prop, value)
        
    def list_stream_subgroup(self):
         return (self.stream_subgroups, self.selected_stream_subgroup)
       
    def add_stream_subgroup(self, name):
        if not name:
            raise SettingsError, 'Stream subgroup must have a unique name'
        if self.stream_subgroups.has_key(name):
            raise SettingsError, 'Stream subgroup "%s" already exists' % name
        try:
            new_guy = StreamSubgroup(name)
            new_id = 1
            if len(self.stream_subgroups) > 0:
                new_id = max(self.stream_subgroups.keys())+1
            self.stream_subgroups[new_id] = new_guy
            
        finally:
            pass
        return True
    
    def erase_stream_subgroup(self, name):
        id = self._get_stream_subgroup_id_from_name(name)
        if self.selected_stream_subgroup == name:
            self.selected_stream_subgroup = None
        del self.stream_subgroups[id]
    
    def select_stream_subgroup(self, name):
        id = self._get_stream_subgroup_id_from_name(name)
        self.selected_media_setting = name
        return True


class StreamSubgroup(object):
    """
    Stream subgroup instances contain media streams.
    """
    def __init__(self, name):
        self.enabled = False
        self.mode = None
        self.container = None
        self.port = None
        self.name = name
        log.info("StreamSubgroup__init__ " + name)    
        
        self.media_streams = []
        self.selected_media_stream = None
        # these ids insure that every media stream has a unique name 
        self.media_stream_ids = {}
    
    def _find_media_stream(self, name):
        for m in self.media_streams:
            setting_name = m.name
            if  setting_name == name:
                return m
        return None 
        
    def get_media_stream(self, name):
        r = self._find_media_stream(name)
        if r == None:
            raise SettingsError, 'Media stream "%s" does not exist' % name
        return r
        
    def list_media_stream(self):
         return (self.media_streams, self.selected_media_stream)
       
    def add_media_stream(self,typename):
        if not typename:
            raise SettingsError, 'Media stream must have a type'
        # create a new stream with a unique id
        new_guy = MediaStream.create(typename, self.media_stream_ids)
        new_guy.port = self.port
        self.media_streams.append(new_guy)
        return new_guy
    
    def erase_media_stream(self, name):
        media_stream = self.get_media_stream(name)
        if self.selected_media_stream == name:
            self.selected_media_stream = None
        self.media_streams.remove(media_stream)
    
    def select_media_stream(self, name):
        id = self.get_media_stream(name)
        self.selected_media_setting = name

class MediaSetting(object):
    """
    General settings information. They are loaded from a preset file and
    user generated files.

    A media setting is for audio/video streams. 
    """
    
    def __init__(self, name, id):
        self.id = id
        self.name = name
        self.settings = {}
        self.is_preset = False
        log.info("MediaSetting__init__: " + name)        

class MediaStream(object):
    """
    Each media stream instance references a media setting. The MediaStream class
    is a a base class for each stream type: AudioStream, DataStream, VideoStream...

    = Synchronization =

    A sync_group is an attribute of the media stream. Its default value is 'master'. 
    This is for when we want the audio and video not to be in sync. 
    Not being in sync can be achieve by starting on each 
    side two sender processes, and two receiver processes. One for the 
    audio and one for the video. Otherwise, when there is only one process for sending
    both the audio and the video streams, Gstreamer makes sure there are all in sync. 
    """
    # this class variable contains the list of available types of streams
    # each subclass adds a string that identifies its type.
    # the create method will create an instance of the type as long as the
    # type is in the list 
    media_stream_kinds = {}
        
    def __init__(self, name):
       self.name = name
       self.enabled = False
       self.port = None
       self.gain_levels = 'no pain'
       self.channel_names ={}
       self.sync_group = "master"
       # media setting id
       self.setting = None
#       self.engine = None
       
    def _create(media_stream_kind, media_stream_ids):
        """
        Factory method that creates an instance based on the type.
        The ids are auto incremented for extra smooth naming
        """
        if not MediaStream.media_stream_kinds.has_key(media_stream_kind):
            raise SettingsError, '"' + media_stream_kind + '" is not a valid type'
        class_name = MediaStream.media_stream_kinds[media_stream_kind]
        if media_stream_ids.has_key(class_name):
            media_stream_ids[class_name] += 1
        else:
            media_stream_ids[class_name] = 1
        number = "%02d" % media_stream_ids[class_name]
        instanceName = media_stream_kind + number
        command = class_name + '("' + instanceName+ '")'
        return eval(command)

    # make create a class  (aka static)method     
    create = staticmethod(_create)

class MidiStream(MediaStream):
    """
    Contains data settings informations.
    """    
    MediaStream.media_stream_kinds['midi'] = 'MidiStream'
    
    
class DataStream(MediaStream):
    """
    Contains data settings informations.
    """    
    MediaStream.media_stream_kinds['data'] = 'DataStream'
 


class VideoStream(MediaStream):
    """
    Contains video settings informations.
    """
    MediaStream.media_stream_kinds['video'] = 'VideoStream'
    
    def _start_the_engine(self, engine_name):
        self.engine = streams.create__video_engine(engine_name)
        # return engine      
        msg = 'Engine "' + engine_name + '" is not supported'
        log.error(msg)
        raise SettingsError, msg


     


class AudioStream(VideoStream):
    """
    Contains Audio Settings information
    """
    MediaStream.media_stream_kinds['audio']= 'AudioStream'
    pass
