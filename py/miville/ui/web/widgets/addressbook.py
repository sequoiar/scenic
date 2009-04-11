# -*- coding: utf-8 -*-

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

log = log.start('debug', 1, 0, 'web_adb')



class Addressbook(Widget):
    """
    """
        
    def __init__(self, api, template):
        Widget.__init__(self, api, template)
        self.connections = {}
        
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
            adb.append({'name':contact.name,
                        'state':contact.state,
                        'auto_answer':contact.auto_answer,
                        'auto_created':contact.auto_created,
                        'stream_state':contact.stream_state})
        log.info('receive update: %r' % self)
        self.callRemote('update_list', adb)
        
    def rc_get_contact(self, name):
        self.api.get_contact(name, self)
        return False
    
    def cb_get_contact(self, origin, data):
        log.debug('GCOrigin: %s - Data: %s' % (origin, data))
        if origin is self:
            if isinstance(data, AddressBookError):
                log.info('%s' % data);
            else:
                contact_info = data.__dict__.copy()
                if contact_info.has_key('connection'):  # TODO: deal properly with auto created contact
                    del contact_info['connection']
                self.callRemote('show_contact_info', contact_info)

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
                self.callRemote('update_status',
                                data['name'],
                                '%s with %s' % (data['msg'], data['name']),
                                '%s with %s%s. Error: %s' % (data['msg'],
                                                             data['address'],
                                                             port,
                                                             data['exception']))
            elif not data.has_key('name'):
                self.callRemote('error', data['msg'])
        if not data.has_key('exception') and data.has_key('name'):
            self.callRemote('update_status',
                            data['name'],
                            '%s with %s...' % (data['msg'], data['name']))

    def cb_connection_failed(self, origin, data):
        log.debug('CFOrigin: %s - Data: %s' % (origin, data))
        if data['port']:
            port = ':%s' % data['port']
        else:
            port = ''
        self.callRemote('update_status',
                        data['name'],
                        '%s with %s' % (data['msg'], data['name']),
                        '%s with %s%s. Error: %s' % (data['msg'],
                                                     data['address'],
                                                     port,
                                                     data['exception']))

    def cb_ask(self, origin, data):
        log.debug('ASKOrigin: %s - Data: %s' % (origin, data))
        if data.has_key('name'):
            caption = '%s is inviting you. <em>(address: %s)</em>' % (data['name'], data['address'])
        else:
            caption = '%s is inviting you.' % data['address']
        body = 'Do you accept?'
        if self.connections.has_key(data['address']):
            log.warning('%s was already in connections!?!' % data['address'])
        else:
            self.connections[data['address']] = data['connection']
        self.callRemote('ask', data['address'], caption, body)
    
    def cb_ask_timeout(self, origin, data):
        log.debug('ASKTOOrigin: %s - Data: %s' % (origin, data))
        if self.connections.has_key(data):
            del self.connections[data]
        caption = 'You didn\'t answer soon enough.'
        body = 'Connection closed with %s.' % data
        self.callRemote('notification', caption, body)
        
    def cb_answer(self, origin, data):
        self.callRemote('update_status', data['name'], data['msg'])
    
    def rc_accept(self, connection):
        if self.connections.has_key(connection):
            self.api.accept_connection(self, self.connections[connection])
            del self.connections[connection]
        return False
    
    def rc_refuse(self, connection):
        if self.connections.has_key(connection):
            self.api.refuse_connection(self, self.connections[connection])
            del self.connections[connection]
        return False
    
    def cb_stop_connection(self, origin, data):
        if origin is self and data.has_key('exception'):
            if data.has_key('name'):
                self.callRemote('error',
                                '%s with %s. Reason: %s.' % (data['msg'],
                                                             data['name'],
                                                             data['exception']))
            else:
                self.callRemote('error',
                                '%s. Reason: %s.' % (data['msg'],
                                                     data['exception']))
        if not data.has_key('exception'):
            self.callRemote('update_status', data['name'], data['msg'])
        
    def rc_stop_connection(self, name):
        contact = self.api.get_contact(name)
        if isinstance(contact, Exception):
            self.callRemote('error', contact.message)
        else:
            self.api.stop_connection(self, contact)
        return False
    
    def cb_info(self, origin, data):
        if isinstance(data, dict):
            if data.has_key('context'):
                moment = time.strftime('%X')
                if data['context'] == 'auto-answer':
                    self.callRemote('notification',
                                    data['msg'],
                                    'with %s at %s.' % (data['name'], moment))
                    self.callRemote('update_status',
                                    data['name'],
                                    'Connected',
                                    'Connection made by auto-answer at %s.' % moment)
                elif data['context'] == 'connection_closed':
                    if data.has_key('name'):
                        contact = data['name']
                    else:
                        contact = data['address']
                    self.callRemote('notification',
                                    data['msg'],
                                    'by %s at %s.' % (contact, moment))
                    self.callRemote('update_status',
                                    data['name'],
                                    'Connection closed',
                                    '%s by %s at %s.' % (data['msg'], contact, moment))
        else:
            log.debug('IOrigin: %s - Data: %s' % (origin, data))

    def rc_add_contact(self, name, address, port, auto_answer):
        self.api.add_contact(self, name, address, port, auto_answer=auto_answer)
        return False
    
    def rc_remove_contact(self, name):
        self.api.delete_contact(self, name)
        return False
    
    def rc_modify_contact(self, name, new_name, address, port, auto_answer):
        self.api.modify_contact(self, name, new_name, address, port, auto_answer)
        return False
    
    def cb_modify_contact(self, origin, data):
        log.debug('MCOrigin: %s - Data: %s' % (origin, data))
        if origin is self:
            if isinstance(data, Exception):
                self.callRemote('error', data.message)
            else:
                self.callRemote('contact_saved', data)

    cb_add_contact = cb_modify_contact
    cb_save_client_contact = cb_modify_contact
    
    def rc_keep_contact(self, name, new_name, auto_answer):
        self.api.save_client_contact(self, name, new_name, auto_answer)
        return False
    
#    def cb_save_client_contact(self, origin, data):
#        if origin is self and isinstance(data, Exception):
#            self.callRemote('error', '%s' % data)
        
    expose(locals())
