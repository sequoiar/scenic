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
        self.streams = core.curr_setting.streams


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
    
    def start_streams(self, caller, address):
        self.notify(caller, self.streams.start(address))

    def stop_streams(self, caller):
        self.notify(caller, self.streams.stop())

    def set_streams(self, caller, attr, value):
        self.notify(caller, self.streams.set_attr(attr, value))



    ### Stream ###
    
    def set_stream(self, caller, name, kind, attr, value):
        stream = self.streams.get(name, kind)
        if stream:
            self.notify(caller, (stream.set_attr(attr, value), name), kind + '_set')
        else:
            self.notify(caller, name, 'not_found')

    def settings_stream(self, caller, name, kind):
        self.notify(caller, (self.streams.get(name, kind), name), kind + '_settings')
        
    def add_stream(self, caller, name, stream, kind):
        self.notify(caller, (self.streams.add(name, stream), name), kind + '_add')
        
    def delete_stream(self, caller, name, kind):
        self.notify(caller, (self.streams.delete(name, kind), name), kind + '_delete')
        
    def rename_stream(self, caller, name, new_name, kind):
        self.notify(caller, (self.streams.rename(name, new_name, kind), name, new_name), kind + '_rename')
        
    def list_stream(self, caller, kind):
        self.notify(caller, self.streams.list(kind), kind + '_list')
        
    
    ### Connect ###
    
    def start_connection(self, caller):
        self.notify(caller, 2)
        
    def stop_connection(self, caller):
        self.notify(caller, 2)




                