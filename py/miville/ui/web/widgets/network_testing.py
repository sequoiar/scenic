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
# along with Miville. If not, see <http://www.gnu.org/licenses/>.

# System import
import time

#App imports
from miville.ui.web.web import Widget, expose
from miville.utils import log
from miville.utils.i18n import to_utf
from miville.errors import *

log = log.start('debug', 1, 0, 'web_nettest')

class NetworkTesting(Widget):
    """
    Web widget for network performance testing with remote contacts.
    """
    def rc_start_test(self, contact):
        # default values 
        caller = self
        bandwidth = 10 # M
        duration = 10 # s
        kind = "dualtest"
        try:
            contact = self.api.get_contact(self, contact) # we need the object, not the name
        except AddressBookError, e:
            log.error(e.message)
        
        log.debug("widget is trying to start network testing with %s" % (contact))
        self.api.network_test_start(caller, bandwidth, duration, kind, contact)
        return False
        
    def rc_stop_test(self, contact):
        log.debug("network testing stop is not implemented yet")
        return False
        
    #def cb_something(self, args):
                
    expose(locals())
