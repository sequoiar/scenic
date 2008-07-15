# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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

from utils import Subject, get_def_name

class ControllerApi():
    
    def __init__(self, core):
        self.adb = core.adb
        self.notify = core.notify


    ### Contacts ###
        
    def get_contacts(self, caller):
        self.notify(caller, 'contacts', self.adb.contacts)
        
    def add_contact(self, caller, name, address):
        self.notify(caller, get_def_name(), self.adb.add(name, address))
        
    def delete_contact(self, caller, name):
        self.notify(caller, get_def_name(), self.adb.remove(name))
        
    def modify_contact(self, caller, name, new_name, address):
        self.notify(caller, get_def_name(), self.adb.modify(name, new_name, address))
        
    def select_contact(self, caller, name):
        self.notify(caller, get_def_name(), self.adb.select(name))
        
        
    ### Streams ###
    
    def start_stream(self, stream):
        self.notify('stream_status', 'starté')

