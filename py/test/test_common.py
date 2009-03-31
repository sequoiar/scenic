# -*- coding: utf-8 -*-

# Miville
# Copyright (C) 2008 Soci�t� des arts technologiques (SAT)
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
import sys
from twisted.trial import unittest
from twisted.python.filepath import FilePath
from twisted.python.modules import getModule
from miville.utils import common

class ToTestCallback():
    def __init__(self):
        self.dummy = 42
        
    def _good_test(self):
        pass
    
    def __bad_test(self):
        pass
       
class TestCommon(unittest.TestCase):
    def setUp(self):
        pass
    
    def test_find_modules(self):
        res = common.find_modules('miville.ui')
        #check if all user interface found are correctly formated
        for ui in res:
            if not ui.isPackage() or FilePath(ui.filePath.dirname() + '/off').exists():
                self.fail("Some of user interface are incorrect or bad formated")
        
        #check if it doesn't forget any interface???
        uis = []
        mods = getModule('ui').iterModules()
        for ui in mods:
            if ui.isPackage() and not FilePath(ui.filePath.dirname() + '/off').exists():
                uis.append(ui)
                
        assert(res == uis ), self.fail("find_modules didn't get all ui ")        
                
        
    def dont_test_load_modules(self):
        """Test if all module find can be loaded
        """
        uis = common.find_modules('ui')
        loaded = common.load_modules(uis)
        uis_copied = []
        for ui in uis :
            uis_copied.append(ui.load())            
        assert(uis_copied == loaded), self.fail("Can't load all modules found")
        #compare res with list of module in the folder
    
    def test_find_callbacks(self):
        r = ToTestCallback()       
        res = common.find_callbacks(r)

        for k, v in res.iteritems():            
            vv = str(v)
            if len(vv) >= 2 : 
                if vv[0]=='_' and vv[1]=='_' :
                    self.fail("problem detecting good callbacks")

        #compare res with function list of r
        
