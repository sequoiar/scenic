#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Miville
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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
Starts one miville and tests the devices module.
"""
import unittest
from test import lib_miville_telnet as libmi

libmi.kill_all_running_miville()

local = libmi.MivilleTester(use_tmp_home=True)
local.start_miville_process()
local.start_telnet_process()

class Test_Devices(unittest.TestCase):
    def setUp(self):
        global local
        local.unittest = self
        self.local = local

    def test_01_v4l2(self):
        self.local.telnet_process.sendline("devices -k video -t v4l2 -l")
        index = self.local.telnet_process.expect(["Devices for driver", "No device"])
        if index == 0: # else there is no device
            self.local.tst("changed", "devices -k video -t v4l2 d /dev/video0 -m norm pal")
            self.local.tst("changed", "devices -k video -t v4l2 d /dev/video0 -m norm ntsc")

    def test_02_jackd(self):
        self.local.telnet_process.sendline("devices -k audio -t jackd -l")
        index = self.local.telnet_process.expect(["Devices for driver", "No device"])
        if index == 0: # else there is no device
            pass
