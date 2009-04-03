#!/usr/bin/env python
# -*- coding: utf-8 -*-

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
Distibuted telnet system test, local file 
Usage: On local machine: trial test/dist_telnet_sys_test3.py IP_ADDRESS
You should set your env variables first. 
""" 
import unittest
import pexpect
import os
import time
import sys

import miville.utils.telnet_testing as testing

testing.VERBOSE_CLIENT = True #True
testing.VERBOSE_SERVER = False
testing.START_SERVER = False # You must start miville manually on both local and remote host.
testing.start()


        

cmd = 'cp ~/src/miville/trunk/py/test/configs/settings.sets /var/tmp/.sropulpof'
os.system(cmd)


#TC-1 Access Interface

class Test_TC1_Access_Interface(testing.TelnetBaseTest):
   

    def test_01_yes(self):
        self.expectTest('pof: ', 'The default prompt is not appearing.')




class Test_TC2_List_Contacts(testing.TelnetBaseTest):

    
    def test_01_list_contacts_empty(self): 
        self.tst("contacts --list","")
        time.sleep(2)

    def test_02_list_contacts_one(self):      
        # add a contact
        self.tst("contacts --add contact1 10.10.10.10","Contact added")          
        self.tst("contacts --list","")
        time.sleep(2)
        self.tst("contacts --erase contact1","Contact deleted")
        time.sleep(2)

    def test_03_list_contacts_many(self):
        
        # add a contact
        self.tst("contacts --add contact1 10.10.10.10","Contact added")
        self.tst("contacts --add contact2 10.10.10.10","Contact added")
        self.tst("contacts --add contact3 10.10.10.10","Contact added")
        self.tst("contacts --add contact4 10.10.10.10","Contact added")
        #figure out the RE mechanism here
        self.tst("contacts --list","contact1")
        self.tst("contacts --erase contact1","Contact deleted")
        self.tst("contacts --erase contact2","Contact deleted")
        self.tst("contacts --erase contact3","Contact deleted")
        self.tst("contacts --erase contact4","Contact deleted")
        time.sleep(2)


class Test_TC3_Select_Contacts(testing.TelnetBaseTest):

    def test_01_select_contact(self): 
        self.tst("contacts --add contact1 10.10.10.10","Contact added")
        self.tst("contacts --select contact1","Contact selected")        
        self.tst("contacts --erase contact1","Contact deleted")
        time.sleep(2)    

    def test_02_select_non_existent_contact(self): 
        self.tst("contacts --select vjshv","Could not select this contact.")        
        time.sleep(2)             
       

class Test_TC4_Add_Contacts(testing.TelnetBaseTest):

    def test_01_add_contact(self): 
        self.tst("contacts --add contact1 10.10.10.10","Contact added")
        self.tst("contacts --select contact1","Contact selected")        
        self.tst("contacts --erase contact1","Contact deleted")
        time.sleep(2)   
 
    def test_02_add_contact_bad_IP(self): 
        self.tst("contacts --add contact1 nsdfmvklsdfn","Could not add contact.")
        time.sleep(2)

    def test_03_add_contact_no_name(self): 
        self.tst("contacts --add  10.10.10.10","You need to give at least a name and an address.")
        time.sleep(2)
    def test_04_add_duplicate(self):
        self.tst("c -a contact3 192.168.20.20","Contact added")
        self.tst("c -a contact3 192.168.20.20","Could not add contact.")
        
    
      
class Test_TC5_Delete_Contacts(testing.TelnetBaseTest): 
    def test_01_select_contact(self):   
        self.tst("contacts --add  contact1 10.10.10.10","Contact added")
        self.tst("contacts --select contact1","Contact selected")       
        time.sleep(2)

    def test_02_delete_invalid_contact(self):
        self.tst("contacts --erase some_invalid_name", "Could not delete")
        self.tst("contacts --erase contact1","Contact deleted") 
        self.sleep()



class Test_TC6_Load_default_settings(testing.TelnetBaseTest): 
    def test_01_load_settings(self):   
        self.tst("s --load","Settings loaded")
        self.tst("s -x","GLOBAL SETTINGS:")        
        time.sleep(2)

class Test_TC7_Modify_Contacts(testing.TelnetBaseTest): 
    
    def test_01_modify_selected_name(self):
        self.tst("contacts --add  contact1 10.10.10.10","Contact added")
        self.tst("contacts --select contact1","Contact selected")
        self.tst("contacts --modify contact2", "Contact modified")
        self.tst("contacts --modify contact1", "Contact modified")

    def test_02_modify_selected_ip(self):
        self.tst("contacts --modify address=172.16.20.30","Contact modified")
        self.tst("contacts --list","contact1")

    def test_03_modify_selected_name_and_ip(self):
        self.tst("contacts --modify contact1 192.168.30.30","Contact modified")
        time.sleep(2)

    def test_04_modify_selected_name_and_ip(self):
        self.tst("contacts --modify contact1 192.168.30.30","Contact modified")
        time.sleep(2)

    def test_05_modify_port(self):
        self.tst("contacts --select contact1 ","Contact selected")
        self.tst("contacts --modify port=2222","Contact modified")
        time.sleep(2)

    def test_06_modify_no_argument(self):
        self.tst("contacts --select contact1","Contact selected")
        self.tst("contacts --modify ","You need to give at least one argument.")
        time.sleep(2)

    def test_07_modify_settings(self):
        self.tst("contacts --add bloup 10.10.10.72","Contact added")
        self.tst("contacts --select bloup","Contact selected")
        self.tst("contacts --modify port=2222","Contact modified")
        self.tst("contacts --modify setting=10000 ","Contact modified")
        time.sleep(2)


class Test_TC8_Duplicate_contact(testing.TelnetBaseTest): 
    def test_01_duplicate_selected(self):
        self.tst("contacts --add  test1 10.10.10.10","Contact added")
        self.tst("contacts --select test1","Contact selected")        
        self.tst("c -d ","Contact duplicated")
        self.tst("contacts --list","test1")
        self.sleep()

    def test_02_duplicate_named(self):
        self.tst("c -d  test2 test1", "Contact duplicated")
        self.sleep()
        

class Test_TC9_Join(testing.TelnetBaseTest): 

    def test_01_create_remote_contact(self):   
        stdin, stdout, stderr = os.popen3("ssh bloup")
        
        stdin.write("""cd /home/scormier/src/miville/trunk/py;trial test/dist_telnet_sys_test7b.py\n""")
        time.sleep(5)        
        stdin.write("""exit\n""")
        stdin.close()
        stdout.close()
        stderr.close()    


    def test_02_join(self): 
        
        self.tst("contacts --select  bloup","Contact selected")
        self.tst("j -s ", "")
        time.sleep(5)


class Test_TC_10_Automatic_connection(testing.TelnetBaseTest): 
     
    

    def test_01_stream(self):
        self.tst("streams --start bloup", "")
        time.sleep(50)
    
