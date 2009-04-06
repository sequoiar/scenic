#!/usr/bin/env python
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
# along with Miville.  If not, see <http://www.gnu.org/licenses/>.
"""
Starts two miville and tests the protocols/pinger module.
"""
import unittest

from miville.devices import settings

class Test_Simple_Device_Settings(unittest.TestCase):
    def test_01_base(self):
        str_attr = settings.StringAttribute("spam", "Hello I am a young girl")
        int_attr = settings.IntAttribute("egg", 456)
        bool_attr = settings.BooleanAttribute("ham", True)
        str_attr.set_value("Hello I am a big ugly guy")
        int_attr.set_value(69)
        bool_attr.set_value(False)
