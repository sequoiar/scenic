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
import traceback
#App imports
from miville.ui.web.web import expose
from miville.ui.web.web import Widget
from miville.utils import log
from miville.utils.i18n import to_utf
from miville.errors import *

from twisted.python import failure

log = log.start('debug', 1, 0, 'web_adb')

class Addressbook(Widget):
    """
    Addressbook widget for the miville web ui.
    
    Manages contacts and connection state with each contact.
    Also displays a streaming icon. 

    Streaming and network testing should be mutually exclusive.

     * rc_ methods are called from js/adressbook.js
     * cb_ methods are called from miville/api.py
    """
    def __init__(self, api, template):
        Widget.__init__(self, api, template)
        self.connections = {}
        self._currently_selected_contact = None # adding this to avoid using api.select_contact

    def rc_get_list(self):
        """
        Javascript wants the list of contacts.
        """
        self.api.get_contacts(self)
        return False
        
    def cb_get_contacts(self, origin, data):
        """
        Called from Python API when get_contacts notification is triggered

        Maybe we should add a better sorting algorithm with collation support
        and/or natural order. See:
        http://jtauber.com/2006/02/13/pyuca.py
        http://www.codinghorror.com/blog/archives/001018.html
        """
        addressbook = []
        contacts_dict = data[0]
        sorted_keys = sorted(contacts_dict, key=unicode.lower)
        for key in sorted_keys:
            contact = contacts_dict[key]
            addressbook.append({'name':contact.name,
                        'state':contact.state,
                        'auto_answer':contact.auto_answer,
                        'setting':contact.profile_id, #TODO: rename to profile
                        'auto_created':contact.auto_created,
                        'stream_state':contact.stream_state})
        log.info('receive update: get_contacts %r %s' % (self, data))
        self.callRemote('update_list', addressbook)
        
    def rc_get_contact(self, name):
        """
        Javascript wants a contact's infos.
        """
        self.api.get_contact(name, self)
        return False
    
    def cb_get_contact(self, origin, data):
        """
        The API answers with a contact's infos.
        
        Called from Python API when get_contact notification is triggered
        """
        log.debug('notify(\'%s\', \'%s\', \'%s\')' % (origin, data, 'get_contact'))
        if origin is self:
            if isinstance(data, AddressBookError):
                log.info('%s' % data);
            else:
                contact_info = data.__dict__.copy()
                if contact_info.has_key('connection'):  # TODO: deal properly with auto created contact
                    del contact_info['connection']
                self.callRemote('show_contact_info', contact_info)

    def rc_start_connection(self, name):
        """
        Javascripts want to connect/join a contact.
        
        connects to a remote contact.
        called from Javascript widget
        """
        contact = self.api.get_contact(name)

        if isinstance(contact, Exception):
            self.callRemote('error', contact.message)
        else:
            self.api.start_connection(self, contact)
        return False
    
    def cb_start_connection(self, origin, data):
        """
        The API tells us it has connected/joined a contact.
        
        Called from Python API when start_connection notification is triggered
        """
        port = "PORT" # TODO FIXME XXX where should that var come from ?
        log.debug('notify(\'%s\', \'%s\', \'%s\')' % (origin, data, 'start_connection'))
        if origin is self:
            if data.has_key('exception'):
                self.callRemote(
                    'update_status',
                    data['name'],
                    '%s with %s' % (data['msg'], data['name']),
                    '%s with %s%s. Error: %s' % (
                        data['msg'],
                        data['address'],
                        port,
                        data['exception']
                        )
                    )
            elif not data.has_key('name'):
                self.callRemote('error', data['msg'])
        if not data.has_key('exception') and data.has_key('name'):
            self.callRemote('update_status',
                data['name'],
                '%s with %s...' % (data['msg'], data['name']))

    def cb_connection_failed(self, origin, data):
        """
        The API tells us it could not connect/join a contact.
        
        Called from Python API when connection_failed notification is triggered
        """
        log.debug('notify(\'%s\', \'%s\', \'%s\')' % (origin, data, 'connection_failed'))
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
        """
        The API tells us that a contact is inviting us to join/connect with it.
        
        Called from Python API when ask notification is triggered
        """
        log.debug('notify(\'%s\', \'%s\', \'%s\')' % (origin, data, 'ask'))
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
        """
        The API tells us that it is too late to answer yes. (to join/connect a contact)
        
        Called from Python API when ask_timeout notification is triggered
        """
        log.debug('notify(\'%s\', \'%s\', \'ask_timeout\')' % (origin, data))
        if self.connections.has_key(data):
            del self.connections[data]
        caption = 'You didn\'t answer soon enough.'
        body = 'Connection closed with %s.' % data
        self.callRemote('notification', caption, body)
        
    def cb_answer(self, origin, data):
        """
        The API tells us that the remote contact answered yes? (I think)
        
        Called from Python API when answer notification is triggered
        """
        self.callRemote('update_status', data['name'], data['msg'])
    
    def rc_accept(self, connection):
        """
        Javascripts want to accept to connect/join a contact.
        """
        if self.connections.has_key(connection):
            self.api.accept_connection(self, self.connections[connection])
            del self.connections[connection]
        return False

    def cb_accept_connection(self, origin, data):
        """
        The API confirms us that we accepted to join/connect a contact.
        """
        basic_server = data['connection'] # BasicServer instance
        contact = self.api.client_contact(basic_server.addr.host, basic_server.client_port)
        # contact = connection.contact
        self.callRemote('update_status', contact.name, "Connected.", "You accepted the connection.")

    def rc_refuse(self, connection):
        """
        Javascripts want to refuse to connect/join a contact.
        """
        if self.connections.has_key(connection):
            self.api.refuse_connection(self, self.connections[connection])
            del self.connections[connection]
        return False
    
    def cb_stop_connection(self, origin, data):
        """
        The API tells us that we successfully (or not) stopped the connect/join to a contact.
        """
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
        """
        Javascript wants to disconnect/unjoin a contact.
        """
        contact = self.api.get_contact(name)
        if isinstance(contact, Exception):
            self.callRemote('error', contact.message)
        else:
            self.api.stop_connection(self, contact)
        return False
    
    def cb_info(self, origin, data):
        """
        Called from Python API when info notification is triggered
        This should be DEPRECATED.
        """
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
            log.debug('notify(\'%s\', \'%s\', \'%s\')' % (origin, data, 'info'))

    def rc_add_contact(self, name, address, port, auto_answer):
        """
        Javascript wants to add a contact.
        """
        self.api.add_contact(self, name, address, port, auto_answer=auto_answer)
        return False
    
    def rc_remove_contact(self, name):
        """
        Javascript wants to remove a contact.
        """
        log.debug('rc_remove_contact')
        self.api.delete_contact(self, name)
        return False

    def cb_delete_contact(self, origin, data): 
        """
        We need this in order to know that we successfully deleted a contact.
        """
        self.api.get_contacts(self)

    def rc_modify_contact(self, name, new_name, address, port, auto_answer):
        """
        Javascript wants to modify a contact.
        """
        self.api.modify_contact(self, name, new_name, address, port, auto_answer)
        return False
    
    def cb_modify_contact(self, origin, data):
        """
        Called from Python API when modify_contact notification is triggered
        """
        log.debug('notify(\'%s\', \'%s\', \'%s\')' % (origin, data, 'modify_contact'))
        if origin is self:
            if isinstance(data, Exception):
                self.callRemote('error', data.message)
            else:
                self.callRemote('contact_saved', data)
    cb_add_contact = cb_modify_contact
    cb_save_client_contact = cb_modify_contact
    
    def rc_keep_contact(self, name, new_name, auto_answer):
        """
        Javascript wants to save the changes it has done to a contact.
        """
        self.api.save_client_contact(self, name, new_name, auto_answer)
        return False
    
#    def cb_save_client_contact(self, origin, data):
#        if origin is self and isinstance(data, Exception):
#            self.callRemote('error', '%s' % data)
    
    def cb_will_start_streams_with_contact(self, origin, data):
        """
        Triggered from the Streams widget so that we know with who we try to start streaming.
        
        adding this to avoid using api.select_contactt 
        """
        contact = data
        self._currently_selected_contact = contact

    def cb_start_streams(self, origin, data):
        """
        Called once miville has started to stream.

        This is to update the streaming status on B's side.
        (whan A initiates the streams, and B is passive)
        
        The 'start_streams' notification should contain a dict with keys:
         * 'started : bool
         * 'contact_name' : str
         * 'msg': str
        IMPORTANT: right now, it is not a dict! Just a str
        """
        log.debug('notify(\'%s\', \'%s\', \'%s\')' % (origin, data, 'start_streams'))
        #  START_STREAMS {'started': True, 'msg': 'streaming started'}
        if not data["success"]:
            log.error('Could not start streaming ' + data["message"])
            contact = data["contact"]
            contact_name = data["contact_name"]
            self.callRemote('update_status', 
                contact_name, 
                '%s' % (data["message"]), 
                'Could not start streaming. %s' % (data["message"]))
        else:
            # success ! 
            contact_name = data["contact_name"]
            contact = data["contact"]
            self.callRemote('update_selected', contact_name)
            profile_id = contact.profile_id
            setting_name = self.api.config_db.get_profile(profile_id).name # might raise an error
            self.callRemote('update_status', contact_name, 'Streaming (%s)' % (setting_name), 'Currently streaming. (%s)' % (setting_name))

    def cb_stop_streams(self, origin, data):
        """
        Called once miville has stopped streaming.

        The 'stop_streams' notification should contain a dict with keys:
         * 'stopped' : bool
         * 'contact_name' : str
         * 'msg': str
        """
        log.debug("STOP_STREAMS " + str(data))
        log.debug('notify(\'%s\', \'%s\', \'%s\')' % (origin, data, 'stop_streams'))
        if isinstance(data, failure.Failure):
            msg = data.getErrorMessage()
        else:
            contact = data["contact"]
            contact_name = data["contact_name"]
            self.callRemote('update_selected', contact_name)
            self.callRemote('update_status', contact_name, 'Stopped streaming', 'Stopped streaming.')

    def cb_remote_started_streams(self, origin, data):
        """
        The API tells us that the remote Alice has started streams with us. (Bob)
        
        sets the contact stream_state to streaming when started from remote.
        """
        log.debug("cb_remote_started_streams %s %s" % (origin, data))
        if isinstance(data, failure.Failure):
            pass
        else:
            if data["success"]: 
                contact = data["contact"]
                contact_name = data["contact_name"] #:contact_name,
                message = "Started to stream. " + str(data["message"])
                self.callRemote('update_selected', contact_name)
                self.callRemote('update_status', contact_name, message, message)
            else:
                log.error("cb_remote_started_streams success is False.")

    def cb_remote_stopped_streams(self, origin, data):
        """
        sets the contact stream_state to stopped when stopped from remote.
        """
        log.debug("cb_remote_stopped_streams %s %s" % (origin, data))
        if isinstance(data, failure.Failure):
            pass
        else:
            if data["success"]: 
                contact = data["contact"]
                contact_name = data["contact_name"]
                message = "Stream stopped. " + str(data["message"])
                self.callRemote('update_selected', contact_name)
                self.callRemote('update_status', contact_name, message, message)
            else:
                log.error("cb_remote_started_streams success is False.")
    
    # important
    expose(locals())
