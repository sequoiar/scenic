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

import os
import sys
from twisted.trial import unittest
from miville.utils.observer import Observer
from miville.utils.observer import Subject


class lillController():
    def __init__(self):
        pass
    
################## for test observer ##################
class GoodObserver(Observer):
    def __init__(self, subject , controller):
        Observer.__init__(self, subject)
        self.controller = controller
        self.callbacks = {'silly_var' : 10 }
        self.callbacks['a_test_function'] = 0
        
    def update(self, origin, key, data):
        #si orign change -> update
        self.callbacks[key] = data
        #print "update " + str(key) + " " + str(self.callbacks[key])
    
#Observer without update implemented to verify raise NotImplemented    
class BadObserver(Observer):
    def __init__(self, subject , controller):
        Observer.__init__(self, subject)
    

#####################################################    
#Test 1 observer and 1 subject        
class TestObserver(unittest.TestCase):
    
    def setUp(self):
        #creation subject
        self.gs = GoodSubject(42, 1)
        controler = lillController()
        
        #creation good observer
        self.obs = GoodObserver(self.gs, controler)
        
    def tearDown(self):
        del self.obs
    
    def test_function_notify(self):
        self.gs.a_test_function()
        
        assert(self.obs.callbacks['a_test_function'] == 1), self.fail('Problem, lake of notify for function')

    def test_var_notify(self):
        self.gs.change_silly_var()
        assert(self.obs.callbacks['silly_var'] == 42), self.fail('Problem lake of notify for var change')

        
#test 1 observer and 2 subjects
class TestObserver2(unittest.TestCase):
    
    def setUp(self):
        #creation subject
        self.gs = GoodSubject(42, 1)
        self.gs2 = GoodSubject(44, 3)
        
        controler = lillController()
        
        #creation good observer
        self.obs = GoodObserver(self.gs, controler)
        
        #on ajoute le lien observer - subject
        self.obs.append(self.gs2)
        
    def tearDown(self):
        del self.obs
    
    def test_function_notify(self):
        self.gs.a_test_function()
        assert(self.obs.callbacks['a_test_function'] == 1), self.fail('Problem, lake of notify for function')

        self.gs2.a_test_function()
        assert(self.obs.callbacks['a_test_function'] == 3), self.fail('Problem, lake of notify for function')
        
    def test_var_notify(self):
        self.gs.change_silly_var()
        assert(self.obs.callbacks['silly_var'] == 42), self.fail('Problem, lake of notify for var change')

        self.gs2.change_silly_var()
        assert(self.obs.callbacks['silly_var'] == 44), self.fail('Problem, lake of notify for var change')        
  
         
#test 2 observer and 2 subjects
class TestObserver3(unittest.TestCase):
    
    def setUp(self):
        #creation subject
        self.gs = GoodSubject(42, 1)
        self.gs2 = GoodSubject(44, 3)
        
        controler = lillController()
        
        #creation good observer
        self.obs = GoodObserver(self.gs, controler)
        #on ajoute le lien observer - subject
        self.obs.append(self.gs2)
               
        self.obs2 = GoodObserver(self.gs, controler)
        #on ajoute le lien observer - subject
        self.obs2.append(self.gs2)
        
        
    def tearDown(self):
        del self.obs
    
    def test_function_notify(self):
        self.gs.a_test_function()
        assert(self.obs.callbacks['a_test_function'] == 1), self.fail('Problem, lake of notify for function')
        assert(self.obs2.callbacks['a_test_function'] == 1), self.fail('Problem, lake of notify for function')

        self.gs2.a_test_function()
        assert(self.obs.callbacks['a_test_function'] == 3), self.fail('Problem, lake of notify for function')
        assert(self.obs2.callbacks['a_test_function'] == 3), self.fail('Problem, lake of notify for function')
        
    def test_var_notify(self):
        self.gs.change_silly_var()
        assert(self.obs.callbacks['silly_var'] == 42), self.fail('Problem, lake of notify for function')
        assert(self.obs2.callbacks['silly_var'] == 42), self.fail('Problem, lake of notify for function')
        
        self.gs2.change_silly_var()
        assert(self.obs.callbacks['silly_var'] == 44), self.fail('Problem, lake of notify for function') 
        assert(self.obs2.callbacks['silly_var'] == 44), self.fail('Problem, lake of notify for function') 



#test if not implemented is raised
class TestObserver4(unittest.TestCase):
    def test_bad_observer(self):
        self.gs = GoodSubject(42, 1)
        controler = lillController()
        self.obs = BadObserver(self.gs, controler)
        
        #test if NotImplementedError is called
        self.failUnlessRaises(NotImplementedError,self.gs.change_silly_var)
        
############# for test Subject ################
class GoodSubject(Subject):
    def __init__(self, var, fun):
        Subject.__init__(self)
        self.silly_var = 10
        self.var = var
        self.fun = fun
        
    def change_silly_var(self):
            self.silly_var = self.var
            self.notify('silly_var',self.silly_var, 'silly_var') 

    def a_test_function(self):
            self.notify('a_test_function',self.fun)
            
    
############################################
    
        
    
        
        