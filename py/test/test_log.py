#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technoligiques (SAT)
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

import os
import sys
import shutil
from twisted.trial import unittest

from utils import log
#test to_utf and open

def generateString(sauf = ''):
    alphabet = 'abcdefghijklmnopqrstuvwxyz_@12345-6789.0!?=+%$()[]{}#<>€£éèêëà§çπ‡Ò∂ƒﬁ~'
    pbchar = '/"\'`¬µ‹≈©◊≠÷'
    problem_with_those_char = '&â†°'
    string=''
    for x in range(len(alphabet)):
        if ( not alphabet[x] in sauf):
            string+=alphabet[x]
    return string

class TestLog(unittest.TestCase):
    
    def setUp(self):
        self.orig_home = os.environ['HOME']
        os.environ['HOME'] = '/var/tmp'
        log.start('debug', 1, 1, 'test')

    def tearDown(self):
        shutil.rmtree(os.environ['HOME'] + '/.sropulpof', True)
        os.environ['HOME'] = self.orig_home        
        log.stop()

#    def test_start(self):  
#        #to test log_name     
#        levels = ['info', 'warning', 'error', 'critical', 'debug']
#        #assert ( log.start('info', 1, 1) == 'twisted'), self.fail('problem getting the correct log name')
#        res = log.start('info', 1, 1)
#        assert(res == None), self.fail('can\'t get the instance of the logger')
#               
#        for i in levels:
#            res = log.start(i, 1, 1, generateString())
#            assert (res != None), self.fail('problem getting the correct log name after creating')
#            log.stop()
#            
#        
#    def test_stop(self):
#        log.stop()
        
    def test_set_level(self):
        levels = ['info', 'warning', 'error', 'critical', 'debug']
        
        for i in levels:
            log.set_level(i)
        
    def test_critical(self):
        log.critical(generateString())
        
    def test_error(self):
        log.error(generateString())
        
    def test_warning(self):
        log.warning(generateString())
        
    def test_info(self):
        log.info(generateString())
        
        
    def test_debug(self):
        log.debug(generateString())                        
