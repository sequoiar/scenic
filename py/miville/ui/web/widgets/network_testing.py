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
import pprint

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
        duration = 1 # s
        kind = "dualtest"
        log.debug("trying to get contact object for %s" % (contact))
        log.debug(type(contact))
        try:
            contact_obj = self.api.get_contact(contact) # we need the object, not the name
        except AddressBookError, e:
            log.error("AddressBookError %s" % e.message)
        else:
            log.debug("widget is trying to start network testing with %s" % (contact_obj))
            self.api.network_test_start(caller, bandwidth, duration, kind, contact_obj)
        return False # we must do this for rc_* methods
        
    def rc_stop_test(self, contact):
        log.debug("network testing stop is not implemented yet")
        return False
        
    def cb_network_test_start(self, origin, data):
        # data is a string
        # data could be {message, contact_name, duration}
        log.debug("started network test" + str(origin) + str(data))
        #log.debug("origin:" + str(origin))
        #log.debug("data:" + str(data))

    def cb_network_test_done(self, origin, data):
        """
        Results of a network test. 
        See network.py
        :param data: a dict with iperf statistics
        """
        contact_name = data['contact'].name
        local_data = None
        remote_data = None
        txt = ""
        for host_name in ['local', 'remote']:
            if data.has_key(host_name):
                if host_name == "local":
                    txt += "From local to remote" + "\n"
                    local_data = data[host_name]
                else:
                    txt += "From remote to local" + "\n"
                    remote_data = data[host_name]
                host_data = data[host_name]
                for k in host_data:
                    txt += "\t%s: %s\n" % (k, str(host_data[k]))
        #log.debug(txt)
        #pprint.pprint(data)
        #raw_data = {} 
        if data.has_key('local'):
            local_data = data['local']
        if data.has_key('remote'):
            remote_data = data['remote']
        self.callRemote('test_results', contact_name, txt, local_data, remote_data) # data)
    
    def cb_network_test_error(self, origin, data):
        """
        :param data: dict or string

        Example ::
        self.api.notify(
            caller, 
            {
            'msg':'Connection failed',
            'exception':'%s' % err,
            }, 
            "error")
        """
        if isinstance(data, dict):
            msg = "Error: \n"
            # mandatory arguments
            for k in data.keys():
                msg += "  %s\n" % (data[k])
            log.error(msg)
        else:
            log.error(data)
                
    expose(locals())
