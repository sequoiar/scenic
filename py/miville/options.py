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
Startup configuration for miville.

We needed to separate this class from the core, but still not have it in the
executable only, in case we want to make many different executables, such as 
imiville.py. 
"""

import os

class MivilleConfiguration(object):
    """
    Stores configuration options for miville. 
    
    Default values are set here.
    They can be overriden using a dict passed to the constructor.
    They can also be changed manually, simply. 
    This class is not actually used yet.
    """
    def __init__(self, dictionary=None):
        self.verbose = False # useless so far
        # network
        self.com_chan_port = 31054
        self.telnet_port = 14444
        self.restart_jackd = False
        self.connector_port = 2222
        self.web_port = 8080
        self.iperf_port = 5001 # iperf's default
        #self.midi_port = 44000
        # self.ipcp_port = 999999999999
        self.port_numbers_offset = 0
        self.listen_to_interfaces = '' # means all interfaces
        self.ui_network_interfaces = ['127.0.0.1'] # default is only local host
        self.enable_escape_sequences = True
        # files
        self.miville_home = os.path.expanduser("~/.miville") # TODO: change for ~/.miville
        self.addressbook_filename = 'addressbook.txt' # TODO: "contacts.txt"
        self.settings_presets_filename = "presets.txt" # TODO: presets.txt
        self.settings_filename = "settings.txt" # TODO: settings.txt
        # update the attributes to match passed dict
        if dictionary is not None:
            self.update_dict(dictionary)

    def update_dict(self, dico):
        self.__dict__.update(dico)

    def print_values(self):
        for key, value in self.__dict__.items():
            print "%20s: %s" % (key, value)

