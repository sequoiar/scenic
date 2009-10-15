# -*- coding: utf-8 -*-
# 
# Miville
# Copyright (C) 2008 Societe des arts technologiques (SAT)
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
from miville.utils.common import PortNumberGenerator

class ToTestCallback():
    def __init__(self):
        self.dummy = 42
        
    def _good_test(self):
        pass
    
    def __bad_test(self):
        pass

class TestSimple(unittest.TestCase):
    
    def test_port_generator(self):
        
        porc = PortNumberGenerator(33,17)
        x = porc.get_current_port()
        self.assertTrue(x == None)
        x1 = porc.generate_new_port()
        self.assertTrue(x1 == 33)
        y = porc.generate_new_port()
        self.assertTrue(y == 50)
        z = porc.get_current_port()
        self.assertTrue(z == 50)
        w = porc.generate_new_port()
        self.assertTrue(w == 67)
        w1 = porc.generate_new_port()
        self.assertTrue(w1 == 84)
        # conflict.. 67 is already taken
        jerzy = PortNumberGenerator(67,1)
        jx = jerzy.get_current_port()
        self.assertTrue(jx == None)
        jy = jerzy.generate_new_port()
        self.assertTrue(jy == 68)
        
        jzs = jerzy.generate_new_ports(17)
        self.assertTrue(len(jzs) == 17)
        self.assertTrue(84 not in jzs)
    
    def test_free_port(self):
        #TODO: improve this test
        alloc = PortNumberGenerator(1, 2)
        a = alloc.get_current_port()
        self.assertTrue(a == None)
        b = alloc.generate_new_port()
        self.assertTrue(b == 1)
        c = alloc.generate_new_port()
        self.assertTrue(c == 3)
        alloc.free_port(c)
        if c in alloc.get_all():
            self.fail("Port %d should have been freed." % (c))
        
        
        
        
           
class TestCommon(unittest.TestCase):

    def setUp(self):
        pass
        
    def test_find_modules(self):
        res = common.find_modules('ui')
        #check if all user interface found are correctly formated
        for ui in res:
            if not ui.isPackage() or FilePath(ui.filePath.dirname() + '/off').exists():
                self.fail("Some of user interface are incorrect or bad formated")
        #check if it doesn't forget any interface???
        uis = []
        mods = getModule('miville.ui').iterModules()
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
