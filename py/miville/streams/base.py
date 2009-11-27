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
Base class for streamers
"""
from miville.utils import log
from miville.streams import constants

log = log.start("info", 1, 0, "streams.base")

class Stream(object):
    """
    Description of a stream as passed using the Streaming Session Protocol.
    """
    def __init__(self, session=None, direction=constants.DIRECTION_TO_ANSWERER, entries=None, service=None):
        """
        :param entries: dict
        """
        # TODO: add steamer_class param
        self.session = session
        self.entries = entries # dict or None
        self.service = service
        self.state = constants.STATE_IDLE
        self.direction = direction
        self.remote_problems = []
        self.local_problems = []
         
        # modes
        send = constants.STREAMER_SENDER
        recv = constants.STREAMER_RECEIVER
        # directions
        to_alice = constants.DIRECTION_TO_OFFERER
        to_bob = constants.DIRECTION_TO_ANSWERER
        # roles
        alice = constants.ROLE_OFFERER
        bob = constants.ROLE_ANSWERER
        mapping = {
            alice: {to_alice: recv, to_bob: send},
            bob: {to_alice: send, to_bob: recv}
        }
        self.mode = mapping[self.session.role][self.direction]

class BaseFactory(object):
    """
    Base class for streamer factories.
    """
    # attribute: ports allocator
    # attrbute: config_db
    # attribute: config_fields
    def config_init(self, config_db):
        raise NotImplementedError("Implemented in child class.")

    def prepare_session(self, session):
        # TODO: move start args here.
        raise NotImplementedError("Implemented in child class.")

    def start(self, session):
        raise NotImplementedError("Implemented in child class.")

    def stop(self, session):
        raise NotImplementedError("Implemented in child class.")
