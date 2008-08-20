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

import sys

# App imports

class ControllerApi():
    
    def __init__(self, core):
        self.adb = core.adb
        self.notify = core.notify
        self.setting = core.curr_setting


    ### Contacts ###
        
    def get_contacts(self, caller):
        self.notify(caller, self.adb.contacts)
        
    def add_contact(self, caller, name, address):
        self.notify(caller, self.adb.add(name, address))
        
    def delete_contact(self, caller, name):
        self.notify(caller, self.adb.remove(name))
        
    def modify_contact(self, caller, name, new_name, address):
        self.notify(caller, self.adb.modify(name, new_name, address))
        
    def select_contact(self, caller, name):
        self.notify(caller, self.adb.select(name))
        
        
    ### Streams ###
    
    def start_stream(self, stream):
        self.notify('stream_status', 'starté')


    ### Audio ###
    
    def audio_set(self, caller, name, attr, value):
        stream = self.setting.get_stream(name, 'audio')
        if stream:
            self.notify(caller, (stream.set_attr(attr, value), name))
        else:
            self.notify(caller, name, 'not_found')

    def audio_settings(self, caller, name):
        self.notify(caller, (self.setting.get_stream(name, 'audio'), name))
        
    def audio_add(self, caller, name):
        self.notify(caller, (self.setting.add_stream(name, 'audio'), name))
        
    def audio_delete(self, caller, name):
        self.notify(caller, (self.setting.delete_stream(name, 'audio'), name))
        
    def audio_rename(self, caller, name, new_name):
        self.notify(caller, (self.setting.rename_stream(name, new_name, 'audio'), name, new_name))
        
    def audio_list(self, caller):
        self.notify(caller, self.setting.list_stream('audio'))




                