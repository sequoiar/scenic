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


#App imports
from ui.web.web import Widget, expose
from utils import log
from utils.i18n import to_utf

log = log.start('debug', 1, 0, 'web_adb')


class Addressbook(Widget):
    """
    """
        
    def rc_get_list(self):
        self.api.get_contacts(self)
        return False
        
    def cb_get_contacts(self, origin, data):
        """
        Maybe we should add a better sorting algorithm with collation support
        and/or natural order. See:
        http://jtauber.com/2006/02/13/pyuca.py
        http://www.codinghorror.com/blog/archives/001018.html
        """
        adb = []
        contacts_dict = data[0]
        sorted_keys = sorted(contacts_dict, key=unicode.lower)
        for key in sorted_keys:
            contact = contacts_dict[key]
            adb.append((contact.name, contact.state))
        log.info('receive update: %r' % self)
        self.callRemote('updateList', adb)
        
    def rc_get_contact(self, name):
        self.api.get_contact(name, self)
        return False
    
    def cb_get_contact(self, origin, data):
        log.debug('GCOrigin: %s - Data: %s' % (origin, data))
        if origin is self:
            contact_info = data.__dict__.copy()
            del contact_info['connection']
            self.callRemote('showContact', contact_info)

    def rc_start_connection(self, name):
        contact = self.api.get_contact(name)
        if isinstance(contact, Exception):
            self.callRemote('error', contact.message)
        else:
            self.api.start_connection(self, contact)
        return False
    
    def cb_start_connection(self, origin, data):
        log.debug('SCOrigin: %s - Data: %s' % (origin, data))
        if origin is self:
            if data.has_key('exception'):
                self.callRemote('status', '%s with %s. Error: %s' % (data['msg'], data['name'], data['exception']))
            elif not data.has_key('name'):
                self.write(data['msg'])
        if not data.has_key('exception') and data.has_key('name'):
            self.callRemote('status', '%s with %s...' % (data['msg'], data['name']))

    def cb_connection_failed(self, origin, data):
        log.debug('CFOrigin: %s - Data: %s' % (origin, data))
        if data['port']:
            port = ':%s' % data['port']
        else:
            port = ''
        self.callRemote('status', data['name'], '%s with %s' % (data['msg'], data['name']),
                        '%s with %s%s. Error: %s' % (data['msg'], data['address'], port, data['exception']))
    
    def cb_info(self, origin, data):
        log.debug('IOrigin: %s - Data: %s' % (origin, data))
#        self.callRemote('status', data)

    def rc_add_contact(self, name, address, port):
        self.api.add_contact(self, name, address, port)
        return False
    
    def rc_remove_contact(self, name):
        self.api.delete_contact(self, name)
        return False
    
    def rc_modify_contact(self, name, new_name, address, port):
        self.api.modify_contact(self, name, new_name, address, port)
        return False
    
    def cb_modify_contact(self, origin, data):
        log.debug('MCOrigin: %s - Data: %s' % (origin, data))
        if origin is self and isinstance(data, Exception):
            self.callRemote('error', data.message)

    cb_add_contact = cb_modify_contact
        
    expose(locals())
