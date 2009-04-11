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

log = log.start('debug', 1, 0, 'web_strm')



class Streams(Widget):
    """
    """
        
#    def __init__(self, api, template):
#        Widget.__init__(self, api, template)
#        self.connections = {}
        
    def rc_start_streams(self, contact):
        self.api.start_streams_tmp(self, contact)
        return False
        
    def rc_stop_streams(self, contact):
        self.api.stop_streams_tmp(self, contact)
        return False
        
#    def cb_get_contactss(self, origin, data):
#        """
#        Maybe we should add a better sorting algorithm with collation support
#        and/or natural order. See:
#        http://jtauber.com/2006/02/13/pyuca.py
#        http://www.codinghorror.com/blog/archives/001018.html
#        """
#        adb = []
#        contacts_dict = data[0]
#        sorted_keys = sorted(contacts_dict, key=unicode.lower)
#        for key in sorted_keys:
#            contact = contacts_dict[key]
#            adb.append({'name':contact.name,
#                        'state':contact.state,
#                        'auto_answer':contact.auto_answer,
#                        'auto_created':contact.auto_created})
#        log.info('receive update: %r' % self)
#        self.callRemote('update_list', adb)
                
    expose(locals())
