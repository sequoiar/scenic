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

"""
Distibuted telnet system test, local file 
Usage: On local machine: trial test/dist_telnet_sys_test3.py IP_ADDRESS
You should set your env variables first. 
""" 
import unittest
import pexpect
import os
import time
import sys

import utils.telnet_testing as testing

testing.VERBOSE_CLIENT = True #True
testing.VERBOSE_SERVER = False
testing.START_SERVER = False # You must start miville manually on both local and remote host.
testing.start()


stdin, stdout, stderr = os.popen3("ssh bloup")
stdin.write("""cd /home/scormier/src/miville/trunk/py;trial test/dist_telnet_sys_test4.py 10.10.10.73\n""")
time.sleep(10)        
stdin.write("""exit\n""")
stdin.close()
stdout.close()
stderr.close()
        



#TC1-Interface accessed
class Test_001_Gen_Settings(testing.TelnetBaseTest):
    def test_02_yes(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')
        
    def clean_03_contacts(self):
        self.client.sendline("contacts -e test")
        self.client.sendline("contacts -e test2")

#TC2-Contacts are shown     
class Test_001_List_Contacts(testing.TelnetBaseTest):
    """
    System Tests for listing contacts
    """
    def test_01_list_empty_contacts(self):
        self.tst("contacts --list","")
        
    def test_02_add_contact(self):    
        self.tst("contacts --add test 10.10.10.100", "Contact added")
        time.sleep(2)

    def test_03_list_contact(self):    
        self.tst("contacts --list","test")
        time.sleep(2)

    def test_04_add_contact(self):    
        self.tst("contacts --add test2 10.10.10.101","Contact added")
        time.sleep(2)

    def test_05_list_contacts(self):    
        self.tst("contacts --list","[.*test2]")
        
    def test_06_remove_contact(self):    
        self.tst("contacts --erase test","Contact deleted")
        self.sleep()
       
#    def test_99_close(self):
#        self.client.close()


#stdin, stdout, stderr = os.popen3("ssh bloup")
#stdin.write("""cd /home/scormier/src/miville/trunk/py;trial test/dist_telnet_sys_test4.py 10.10.10.73\n""")
#time.sleep(10)        
#stdin.write("""exit\n""")
#stdin.close()
#stdout.close()
#stderr.close()
