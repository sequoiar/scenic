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
Unit tests for the port allocator.
"""
import socket
from twisted.trial import unittest
from scenic.ports import PortsAllocator
from scenic.ports import PortsAllocatorError

class Test_01_Ports_Allocator(unittest.TestCase):
    def _tst(self, expected, value):
        if value != expected:
            self.fail("Expected value %s but got %s." % (expected, value))

    def test_01_add_remove(self):
        a = PortsAllocator(minimum=22000, increment=2, maximum=22010)

        # value = 2000; value < 2012; value += 2
        for value in xrange(22000, 22012, 2):
            self._tst(value, a.allocate())
        try:
            value = a.allocate()
        except PortsAllocatorError, e:
            pass
        else:
            self.fail("Ports allocator should have overflown. Got value %d." % (value))
        # value = 2000; value < 2012; value += 2
        for value in xrange(22000, 22012, 2):
            a.free(value)
        try:
            a.free(100)
        except PortsAllocatorError, e:
            pass
        else:
            self.fail("Trying to free value %d should have raised an error." % (100))

    def test_02_add_many(self):
        a = PortsAllocator(minimum=22000, increment=2, maximum=22010)
        values = a.allocate_many(6)
        a.free_many(values)
        values = a.allocate_many(3)
        values = a.allocate_many(3)
        try:
            value = a.allocate()
        except PortsAllocatorError, e:
            pass
        else:
            self.fail("Ports allocator should have overflown. Got value %d." % (value))

    def test_03_allocate_busy_port(self):
        # let's use a port
        PORT = 31000
        listener = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # SOCK_DGRAM ?
        listener.bind(("localhost", PORT)) # socket.gethostname()

        a = PortsAllocator(minimum=PORT, increment=2, maximum=PORT + 100)
        num = a.allocate()
        self._tst(PORT + 2, num)
        # that should have worked
        listener.close()
