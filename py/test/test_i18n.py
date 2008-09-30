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
from twisted.trial import unittest

#import address_book
d = os.path.abspath('../utils/')
sys.path.append(d)
__import__('i18n')
import i18n


def generateString(sauf = ''):
    alphabet = 'abcdefghijklmnopqrstuvwxyz_@12345-6789.0!?=+%$()[]{}#<>"'
    pbchar = '/\'`¬µ‹≈©◊≠÷€£ëà§çπ‡Ò∂ƒﬁ~éèê'
    problem_with_those_char = '&â†°'
    string=''
    for x in range(len(alphabet)):
        if ( not alphabet[x] in sauf):
            string+=alphabet[x]
    return string


def generateEncoding(string):
    stringEncoded = []
    stringEncoded.append(string)
    encodingType = ['latin-1', 'ascii', 'utf8']
    #encType = random.choice(encodingType)
    a = unicode(string,'iso-8859-1')
    for i in range(len(encodingType)):
        string = a.encode(encodingType[i], 'replace' )
        stringEncoded.append(string)
    return stringEncoded


#test to_utf and open
class TestI18n(unittest.TestCase):
    
    def setUp(self):
        #test convert to utf
        self.stri = generateString()
    
    def tearDown(self):
        pass

    def test_to_utf(self):
        #strEnc = generateEncoding(str)
        strEnc = ['latin-1', 'ascii', 'utf8']
        for i in strEnc:
            i18n.to_utf(self.stri, i)
        
    def test_open(self):
        #test openning a file 
        i18n.open('test_log.py', 'a')    
        
        
        