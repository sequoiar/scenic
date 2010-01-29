#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Scenic
# Copyright (C) 2008 Société des arts technologiques (SAT)
# http://www.sat.qc.ca
# All rights reserved.
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Scenic is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Scenic. If not, see <http://www.gnu.org/licenses/>.

import sys
import socket
import gobject

# JSON import:
try:
    import json # python 2.6
except ImportError:
    import simplejson as json # python 2.4 to 2.5
try:
    _tmp = json.loads
except AttributeError:
    import warnings
    warnings.warn("Use simplejson, not the old json module.")
    sys.modules.pop('json') # get rid of the bad json module
    import simplejson as json

from scenic import sig #TODO

class Network(object):
    def __init__(self, negotiation_port):
        self.buf_size = 1024
        self.port = negotiation_port
    
    def _validate(self, msg):
        """
        Parses binary data and return a dict or so.
        """
        tmp_msg = msg.strip()
        msg = None
        if tmp_msg.startswith("{") and tmp_msg.endswith("}"):
        #if tmp_msg[0] == "{" and tmp_msg[-1] == "}" and tmp_msg.find("{", 1, -2) == -1 and tmp_msg.find("}", 1, -2) == -1:
            try:
                tmp_msg = json.loads(tmp_msg)
                if type(tmp_msg) is dict:
                    msg = tmp_msg
            except:
                pass
        return msg

    def _listen_for_data(self, conn):
        """
        @param conn: socket stuff
        """
        data = conn.recv(self.buf_size)
        buffer = data
        while len(data) == self.buf_size: # maybe we should recall _handle_data on io_watch
            data = conn.recv(self.buf_size)
            buffer += data
        return buffer

    def close(self):
        self.sock.close()

class Server(Network):
    def __init__(self, app, negotiation_port):
        Network.__init__(self, negotiation_port)
        self.received_command_signal = sig.Signal()
        self.received_command_signal.connect(app.on_server_rcv_command)
        self.host = ''

    def start_listening(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((self.host, self.port))
        sock.listen(1)
        gobject.io_add_watch(sock, gobject.IO_IN, self._handle_data)
        self.sock = sock

    def _handle_data(self, source, condition):
        conn, addr = source.accept()
        buffer = self._listen_for_data(conn)
        msg = self._validate(buffer)
        self.received_command_signal(self, msg, addr, conn)
        return True

class Client(Network):
    def __init__(self, app, negotiation_port):
        Network.__init__(self, negotiation_port)
        
        self.socket_timeout_signal = sig.Signal()
        self.socket_timeout_signal.connect(app.on_client_socket_timeout)
        self.socket_error_signal = sig.Signal()
        self.socket_error_signal.connect(app.on_client_socket_error)
        self.connecting_signal = sig.Signal()
        self.connecting_signal.connect(app.on_client_connecting)
        self.received_command_signal = sig.Signal()
        self.received_command_signal.connect(app.on_client_rcv_command)
        
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(10)

    def connect(self, host, msg):
        """
        @param host: ip
        @param msg: sends it right now !!! (UDP)
        """
        self.host = host
        try:
            self.sock.connect((host, self.port))
        except socket.timeout, err:
            self.socket_timeout_signal(self)
        except socket.error, err:
            self.socket_error_signal(self, err, msg)
        else:
            if self.send(msg):
                self.io_watch = gobject.io_add_watch(self.sock, gobject.IO_IN, self._handle_data)
        return False

    def send(self, msg):
        if not len(msg) % self.buf_size:
            msg += " "
        try:
            self.sock.sendall(msg)
            self.connecting_signal(self)
            return True
        except socket.error, err:
            self.connecting_signal(self)
            self.socket_error_signal(self, err, msg)
            return False

    def _handle_data(self, source, condition):
        buffer = self._listen_for_data(source)
        source.close()
        msg = self._validate(buffer)
        self.received_command_signal(self, msg)
        return False

# ------------------------------------ new stuf:



class NewServer(object):
    def __init__(self, app, negotiation_port):
        self.port = negotiation_port
        self.server_factory = sic.SICServerFactory()
        self.server_factory.dict_received_signal.connect(self.on_dict_received)
        self._port_obj = None
        
        self.received_command_signal = sig.Signal()
        self.received_command_signal.connect(app.on_server_rcv_command)
 
    def start_listening(self):
        if not self.is_listening()
            self._port_obj = reactor.listenTCP(self.port, self.server_factory)
        else:
            print "Already listening"

    def on_dict_received(self, protocol, d):
        print "received", d
        msg = d
        addr = "secret"
        conn = "what?"
        self.received_command_signal(self, msg, addr, conn)

    def close(self): # TODO: important ! 
        if self.is_listening():
            self._port_obj.stopListening()
            self._port_obj = None
        else:
            print "no server to close"

    def is_listening(self):
        return self._port_obj is not None
        
        

class NewClient(object):
    def __init__(self, app, negotiation_port):
        self.port = negotiation_port
        self.host = None
        self.client = None
        self._connected = False
        
        self.socket_error_signal = sig.Signal()
        self.socket_error_signal.connect(app.on_client_socket_error)
        self.connecting_signal = sig.Signal()
        self.connecting_signal.connect(app.on_client_connecting)
        self.received_command_signal = sig.Signal()
        self.received_command_signal.connect(app.on_client_rcv_command)
        # unused:
        #self.socket_timeout_signal = sig.Signal()
        #self.socket_timeout_signal.connect(app.on_client_socket_timeout)
        
    def connect(self, host, msg):
        """
        Connects and sends an INVITE message
        """
        def _on_connected(result, message_to_send):
            print "connected"
            self._connected = True
            self.send(message_to_send)
        
        def _on_error(reason):
            print "could not connect"
            self._connected = False
            self.client = None
            err = str(reason)
            msg = "Could not send to remote host."
            self.socket_error_signal(self, err, msg)
        
        if not self.is_connected():
            self.connecting_signal(self)
            self.host = host
            self.client = sic.create_SIC_client(self.host, self.port).addCallback(_on_connected, msg).addErrback(_on_error)
        else:
            print "client already connected to some host"

        #TODO: 
    
    def send(self, msg):
        """
        @param msg: dict
        """
        if self.is_connected():
            self.client.send_message(msg)
        else:
            msg = "Not connected. Client is None."
            print msg
            err = None
            self.socket_error_signal(self, err, msg)
    
    def is_connected(self):
        return self._connected
