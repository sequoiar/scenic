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

import os
import re # used when reading settings file
from streams.stream import Streams
from utils import log
from utils.i18n import to_utf
from errors import *
# persistence is not futile
from twisted.spread.jelly import jelly, unjelly
import connectors
from twisted.internet import reactor

log = log.start('debug', 1, 0, 'settings')

# The currently supported file version
current_major_version_number = 1

COM_CHAN_KEY = "settings"

_api = None
_settings_channels_dict = {}


class SettingsChannel(object):
    """
    Allows to send and receive setting information
    via the com_chan protocol, once we are joined to
    a remote contact
    """
    
    def __init__(self):
        log.debug('SettingsChannel.__init__')
        self.com_chan = None
        self.contact = None
        self.api = None
        self.caller = None
        self.remote_addr = None    
        
        
    def on_remote_message(self, key, args):
        """
        Called by the com_chan whenever a message is received from the remote contact 
        through the com_chan with the "pinger" key.
        
        :param key: string defining the action/message
        :param *args: list of arguments
        
        The key and *args can be one of the following...
        
        """
        log.debug('SettingsChannel.on_remote_message')
        if key == "ping": # from A
            print "PING"
            self._send_message("pong")
            
            val = "received PING"
            key = "info"
            caller = None
            self.api.notify(caller, val, key)
        elif key == "pong": # from B 
            val = "Received PONG"
            key = "info"
            caller = None
            self.api.notify(caller, val, key)
            self.api.notify(caller, val, key)
    
    def _send_message(self, key, args_list=[]):
        """
        Sends a message to the current remote contact through the com_chan with key "network_test".
        :param args: list
        """
        log.debug("SettingsChannel._send_message %s. %r" % (key, args_list))
        try:
            # list with key as 0th element
            global COM_CHAN_KEY
            self.com_chan.callRemote(COM_CHAN_KEY, key, args_list)
        except AttributeError, e:
            log.error("Could not send message to remote: " + e.message)
        
  
def on_com_chan_connected(connection_handle, role="client"):
    """
    Called from the Connector class in its com_chan_started_client or com_chan_started_server method.

    registers the com_chan callback for network_test
    
    :param connection_handle: connectors.Connection object.
    :param role: string "client" or "server"
    We actually do not care if this miville is a com_chan client or server.
    """
    global _api
    global _settings_channels_dict
    global COM_CHAN_KEY
    log.debug("on_com_chan_connected")
    settings_chan = SettingsChannel()
    contact = connection_handle.contact
    settings_chan.contact = contact
    settings_chan.com_chan = connection_handle.com_chan
    callback = settings_chan.on_remote_message
    settings_chan.com_chan.add(callback, COM_CHAN_KEY)
    settings_chan.api = _api
    settings_chan.remote_addr = contact.address
    _settings_channels_dict[settings_chan.contact.name] = settings_chan
    log.debug("settings_chans: " + str(_settings_channels_dict))
    reactor.callLater(10, settings_chan._send_message, "ping" )
    

def init_connection_listeners(api):
    global _api
    _api = api
    log.debug("init_connection_listeners")
    connectors.register_callback("settings_on_connect", on_com_chan_connected, event="connect")
    connectors.register_callback("settings_on_disconnect", on_com_chan_disconnected, event="disconnect")
  

def on_com_chan_disconnected(connection_handle):
    """
    Called when a connection is stopped
    """
    global _settings_channels_dict
    try:
        del _settings_channels_dict[connection_handle.contact.name]
        log.debug("settings_chans: " + str(_settings_channels_dict))
    except KeyError, e:
        log.error("error in on_com_chan_disconnected : KeyError " + e.message)        
  
class Settings(object):
    """
    Settings: all the gear related settings: reads the presets from file, 
    merges them with user settings.
    Settings instances contain global settings and media settings
    
    quote: 0B1knob: 'Use defaults, Luke!'
    """
    
    def __init__(self, current=None):
        #self.settings = {}
        #self.current = current

        self.global_settings = {}
        self.selected_global_setting = None
        self.current_global_id = 0
        
        self.media_settings = {}
        self.selected_media_setting = None
        self.current_media_id = 0

    @staticmethod
    def _load_nice_object_from_file(filename, major_version):
        """
        this 'private' method loads an object from a text file, using the 
        nice representation (line breaks and tabs)
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
        this 'private' method saves an object to a text file, using the 
        nice representation (jelly text with line breaks and tabs)
        """
        try:
            file = open(filename, 'w')
        except:
                msg = 'Could not open the file %s.' % filename
                log.warning(msg)
                raise SettingsError, msg
        else:
            if major_version == current_major_version_number:
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
    
    @staticmethod   
    def _get_user_settings_filename():
        localDir = os.path.expanduser("~")+"/.sropulpof/"
        localFile = "settings.sets"
        localPath = os.path.join(localDir, localFile)
        return localPath
    
    @staticmethod
    def _get_preset_settings_filename():
        localDir = os.path.expanduser("~")+"/.sropulpof/"
        localFile = "presets.sets"
        localPath = os.path.join(localDir, localFile)
        return localPath

    def load(self):
        presets_file_name       = Settings._get_preset_settings_filename()
        user_settings_file_name = Settings._get_user_settings_filename()
        
        # settings are saved as tuple()
        presets = {}
        user_settings = {}
        
        # we should expect presets settings to exist, but however
        # we will ramp it up slowly
        if os.path.exists(presets_file_name):
            presets = Settings._load_nice_object_from_file(presets_file_name, current_major_version_number)
        
        # user settings file is optional...
        if os.path.exists(user_settings_file_name):
            user_settings = Settings._load_nice_object_from_file(user_settings_file_name, current_major_version_number)
        
        # merge presets and user global settings
        highest_global_id =0
        user_global_settings = user_settings[0]
        presets_globals = presets[0]
        self.global_settings.clear()
        self.selected_global_setting = None
        for k, glob in presets_globals.iteritems():
             self.global_settings[k] = glob
             if k > highest_global_id:
                 highest_global_id = k
        
        for k, glob in user_global_settings.iteritems():
            # check for duplicates
            if not self.global_settings.has_key(k):
                self.global_settings[k] = glob
                if k > highest_global_id:
                    highest_global_id = k
            else:
                log.error("Global user setting %d clashes with preset" % k)
    
        # merge presets and user media settings
        user_media_settings = user_settings[1]
        presets_media_settings = presets[1]
        highest_media_setting_id = 0
        self.media_settings.clear()
        self.selected_media_setting = None
        for k, med in presets_media_settings.iteritems():
            self.media_settings[k] = med
            if k > highest_media_setting_id:
                highest_media_setting_id = k
        for k, med in user_media_settings.iteritems():
            # check for duplicates
            if not self.media_settings.has_key(k):
                self.media_settings[k] = med
                if k > highest_global_id:
                    highest_media_id = k
            else:
                log.error("Media user setting %d: '%s' clashes with preset" % (k,med.name))
            
            
            
    def save(self):
        user_media_settings = {} 
        user_global_settings =  {}
        
        for k in self.global_settings.keys():
            glob = self.global_settings[k]
            if not glob.is_preset:
                user_global_settings[k] = glob
        
        for k in self.media_settings.keys():
            med = self.media_settings[k]
            if not med.is_preset:
                user_media_settings[k] = med
        
        filename = Settings._get_user_settings_filename()
        
        # little hack to  create a preset file
        make_presets = False
        if make_presets:
            for k in self.global_settings.keys():
                glob = self.global_settings[k]
                glob.is_preset = True
        
            for k in self.media_settings.keys():
                med = self.media_settings[k]
                med.is_preset = True
            filename = Settings._get_preset_settings_filename()
            
        stuff = (user_global_settings, user_media_settings)  
        Settings._save_object_to_file_nicely(stuff, filename, current_major_version_number)

    
    def get_media_setting(self, name):
        id = self._get_media_setting_id_from_name(name)
        return self.media_settings[id]
    
    def get_global_setting(self, name):
        id = self._get_global_setting_id_from_name(name)
        return self.global_settings[id]
        

    def list_global_setting(self):
         return (self.global_settings, self.selected_global_setting)
       
    def add_global_setting(self,name):
        if not name:
            raise SettingsError, 'Global setting must have a unique name'
        if self.global_settings.has_key(name):
            raise SettingsError, 'Global setting "%s" already exists' % name
        try:
            glob = GlobalSetting(name)
            self.global_settings[self.current_global_id] = glob
            self.current_global_id += 1
        finally:
            pass 
        return True
    
    def erase_global_setting(self,name):
        id = self._get_global_setting_id_from_name(name)
        if self.selected_global_setting == name:
            self.selected_global_setting = None
        del self.global_settings[id]
       
    def modify_global_setting(self, global_setting_name, attribute, new_value):
        id = self._get_global_setting_id_from_name(global_setting_name)
        raise SettingsError, 'settings.py l 92 MODIFY : ' + global_setting_name + " " + attribute + " = " + new_value  
        return True
       
    def duplicate_global_setting(self,name):
        raise SettingsError, 'This command is not implemented yet' 
        return True

    def _get_media_setting_id_from_name(self,name):
        for id in self.media_settings.keys():
            setting_name = self.media_settings[id].name
            if  setting_name == name:
                return id
        raise SettingsError, 'Media setting "%s" does not exist' % name
    
    def _get_global_setting_id_from_name(self,name):
        for id in self.global_settings.keys():
            setting_name = self.global_settings[id].name
            if  setting_name == name:
                return id
        raise SettingsError, 'Global setting "%s" does not exist' % name
            
    def select_global_setting(self,name):
        id = self._get_global_setting_id_from_name(name)
        self.selected_global_setting = name
        return True
       
       
    def description_global_setting(self,name):
        raise SettingsError, 'This command is not implemented yet' 
        return True



    def list_media_setting(self):
         return (self.media_settings, self.selected_media_setting)
       
    def add_media_setting(self,name):
        if not name:
            raise SettingsError, 'Media setting must have a unique name'
        if self.media_settings.has_key(name):
            raise SettingsError, 'Media setting "%s" already exists' % name
        
        new_setting = MediaSetting(name)
        self.media_settings[self.current_media_id] = new_setting
        self.current_media_id += 1
        return True
    
    def erase_media_setting(self,name):
        id = self._get_media_setting_id_from_name(name)
        if id == None:
            raise SettingsError, 'Media setting "%s" does not exist' % name
        if self.selected_media_setting == name:
            self.selected_media_setting = None
        del self.media_settings[id]

    def select_media_setting(self,name):
        id = self._get_media_setting_id_from_name(name)
        self.selected_media_setting = name
        return True


#class Setting(object):
#    
#    def __init__(self, name):
#        log.info("Setting init: " + name)
#        self.globals = {}
#        self.media = {}
#            
#        if name == 'Custom':
#            self.name = name
#            self.streams = {'send':Streams(),
#                            'receive':Streams('receive')}
#            self.contact = None
#            self.others = {'gst':
#                                {'port_s':10000,
#                                 'port_r':10010,
#                                 'addr_s':'127.0.0.1',
#                                 'addr_r':'127.0.0.1',
#                                 'acodec':'vorbis',
#                                 'vcodec':'h264',
#                                 'port':1244}
#                            }
#
#    def _save(self):
#        """
#        Save to disk
#        """
#        pass
    
        
class GlobalSetting(object):
    """
    Global setting instances contain a list of stream subgroups.
    """
    def __init__(self, name):
        self.is_preset = False
        self.name = None
        self.stream_subgroups = {}
        self.selected_stream_subgroup = None
        self.current_streamsubgroup_id = 0
        self.name = name
        log.info("GlobalSetting__init__ " + name)
    
    
    def _get_stream_subgroup_id_from_name(self,name):
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
       
    def add_stream_subgroup(self,name):
        if not name:
            raise SettingsError, 'Stream subgroup must have a unique name'
        if self.stream_subgroups.has_key(name):
            raise SettingsError, 'Stream subgroup "%s" already exists' % name
        try:
            new_guy = StreamSubgroup(name)
            self.stream_subgroups[self.current_streamsubgroup_id] = new_guy
            self.current_streamsubgroup_id += 1
        finally:
            pass
        return True
    
    def erase_stream_subgroup(self,name):
        id = self._get_stream_subgroup_id_from_name(name)
        if self.selected_stream_subgroup == name:
            self.selected_stream_subgroup = None
        del self.stream_subgroups[id]
    
    def select_stream_subgroup(self,name):
        id = self._get_stream_subgroup_id_from_name(name)
        self.selected_media_setting = name
        return True


class StreamSubgroup(object):
    """
    Stream subgroup instances contain media streams.
    """
   
    def __init__(self, name):
        self.name = None
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

    def _find_media_stream(self,name):
        for m in self.media_streams:
            setting_name = m.name
            if  setting_name == name:
                return m
        return None 
        
    def get_media_stream(self,name):
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
        self.media_streams.append(new_guy)
    
    def erase_media_stream(self,name):
        media_stream = self.get_media_stream(name)
        if self.selected_media_stream == name:
            self.selected_media_stream = None
        self.media_streams.remove(media_stream)
    
    def select_media_stream(self,name):
        id = self._get_media_stream_id_from_name(name)
        if id == None:
            raise SettingsError, 'Media stream "%s" does not exist' % name
        self.selected_media_setting = name
        

class MediaStream(object):
    """
    Each media stream instance references a media setting. The MediaStream class
    is a a base class for each stream type: AudioStream, DataStream, VideoStream...
    """
    
    # this class variable contains the list of available types of streams
    # each subclass adds a string that identifies its type.
    # the create method will create an instance of the type as long as the
    # type is in the list 
    media_stream_kinds ={}
    
    
    def __init__(self, name):
       self.name = name
       self.enabled = False
       
       self.port = None
       self.gain_levels = None
       self.channel_names ={}
       # media setting id
       self.setting = None
       
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
    

class AudioStream(MediaStream):
    """
    Contains Audio Settings information
    """
    MediaStream.media_stream_kinds['audio']= 'AudioStream'
    pass

class VideoStream(MediaStream):
    """
    Contains Video Settings information
    """
    MediaStream.media_stream_kinds['video'] = 'VideoStream'
    pass

class DataStream(MediaStream):
    """
    Contains Data Settings information
    """    
    MediaStream.media_stream_kinds['data'] = 'DataStream'
    pass

class MediaSetting(object):
    """
    General settings information. They are loaded from a preset file and
    user generated files.
    """
    
    next_id = 1
    
    def __init__(self, name):
        self.id = MediaSetting.next_id
        MediaSetting.next_id +=1
        self.name = name
        self.settings = []
        self.is_preset = False
        log.info("MediaSetting__init__: " + name)
    





