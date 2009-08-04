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

import os

from twisted.trial import unittest
from twisted.internet import reactor
from twisted.internet import defer
from twisted.python import failure
from twisted.python import procutils
from twisted.internet.utils import getProcessOutputAndValue

COMMAND_NAME = "puppet"

def _get_output_and_value_callback(results):
    print(results)

class Test_01_Process(unittest.TestCase):
    """
    Tests the ProcessProtocol.
    """
    def test_01_simple(self):
        global COMMAND_NAME
        executable = procutils.which(COMMAND_NAME)[0]
        deferred = getProcessOutputAndValue(executable, args=("-z", "-v", "-g", "-t", "1"), env=os.environ, path='/', reactor=reactor)
        deferred.addCallback(_get_output_and_value_callback)
        return deferred


