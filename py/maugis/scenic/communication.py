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

class Network(object):
    def __init__(self, negotiation_port):
        self.buf_size = 1024
        self.port = negotiation_port
    
    def validate(self, msg):
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

    def recv(self, conn):
        data = conn.recv(self.buf_size)
        buffer = data
        while len(data) == self.buf_size: # maybe we should recall handle_data on io_watch
            data = conn.recv(self.buf_size)
            buffer += data
        return buffer

    def close(self):
        self.sock.close()

class Server(Network):
    def __init__(self, app):
        Network.__init__(self, app.config.negotiation_port)
        self.app = app
        self.host = ''

    def start_listening(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((self.host, self.port))
        sock.listen(1)
        gobject.io_add_watch(sock, gobject.IO_IN, self.handle_data)
        self.sock = sock

    def handle_data(self, source, condition):
        conn, addr = source.accept()
        buffer = self.recv(conn)
        msg = self.validate(buffer)
        self.app.on_server_rcv_command(self, (msg, addr, conn))
        return True

class Client(Network):
    def __init__(self, app):
        Network.__init__(self, app.config.negotiation_port)
        self.app = app
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(10)

    def connect(self, host, msg):
        self.host = host
        try:
            self.sock.connect((host, self.port))
        except socket.timeout, err:
            self.app.on_client_socket_timeout(self)
        except socket.error, err:
            self.app.on_client_socket_error(self, (err, msg))
        else:
            if self.send(msg):
                self.io_watch = gobject.io_add_watch(self.sock, gobject.IO_IN, self.handle_data)
        return False

    def send(self, msg):
        if not len(msg) % self.buf_size:
            msg += " "
        try:
            self.sock.sendall(msg)
            self.app.on_client_add_timeout(self)
            return True
        except socket.error, err:
            self.app.on_client_add_timeout(self)
            self.app.on_client_socket_error(self, (err, msg))
            return False

    def handle_data(self, source, condition):
        buffer = self.recv(source)
        source.close()
        msg = self.validate(buffer)
        self.app.on_client_rcv_command(self, msg)
        return False
