#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
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

from twisted.trial import unittest

import devices
import utils.log

del devices.log
devices.log = utils.log.start('error', 1, 0, 'devices')

class Test_1_Driver(unittest.TestCase):
    """
    Tests for the Driver base class.
    """
    def test_1_basic(self):
         # simple test
        class TestAudioDriver(devices.Driver):
            def start(self):
                self.shell_command_start(['ls','-la'])
            def list(self):
                return []
            def get(self):
                return None
            def shell_command_result(self,command,results):
                print "results are :\n",results
        
        # any exception will make the test fail.
        d = TestAudioDriver()
        
