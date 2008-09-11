#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Société des arts technologiques (SAT)
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

import unittest
from addressbook import AddressBook
import random

def randomString(min, max, sauf = ''):
    alphabet = 'abcdefghijklmnopqrstuvwxyz@1234567890#-_()[]{}&$€%ù+=?!§ç°*æœéàèëôêÔÊÂÏÈÌ./£~"\'<>`®†Úºîπ‡Ò∂ƒﬁ¬µ‹≈©◊∞'
    string=''
    for x in random.sample(alphabet,random.randint(min,max)):
        if ( not x in sauf):
            string+=x
    return string


def randomEncoding(string):
    encodingType = ['latin-1', 'ascii', 'utf8', 'utf-16', 'big5', 'cp424', 'cp037', 'cp856', 'euc_jp', 'iso8859_7', 'koi8_r', 'mac_greek', 'iso8859_14']
    encType = random.choice(encodingType)
    a = unicode(string,'iso-8859-1')
    string = a.encode(encType, 'replace' )
    return a


def randomAddress():
    #mail/SIP/ip/nomdedomaine
    addressType = ['mail', 'SIP', 'IP', 'domain']
    t = random.choice(addressType)
    if ( t == 'mail' ):
        address = randomString(5,128, '@#()[]{}&$€%ù+=?!§ç°*æœéàèëôêÔÊÂÏÈÌ/£~"\'<>`®†Úºîπ‡Ò∂ƒﬁ¬µ‹≈©◊∞')
        address += '@'
        address += randomString(5,128, '@#()[]{}&$€%ù+=?!§ç°*æœéàèëôêÔÊÂÏÈÌ./£~"\'<>`®†Úºîπ‡Ò∂ƒﬁ¬µ‹≈©◊∞')
        address += '.com'
        print address
        return address
    
    elif ( t == 'IP' ):
        ip = ''
        for i in range(3):
            ip += str(random.randint(0,255))
            ip += '.'
        ip += str(random.randint(0,255))
        return ip
    
    elif ( t == 'domain' ):
        domain =''
        i = random.randint(0,1)
        if ( i == 0):
            domain += 'http://'
            
        domain += randomString(5, 128, '@#()[]{}&$€%ù+=?!§ç°*æœéàèëôêÔÊÂÏÈÌ/£~"\'<>`®†Úºîπ‡Ò∂ƒﬁ¬µ‹≈©◊∞')
        domain += '.com'
        return domain
    
    else:
        sip = 'sip:'
        sip += randomString(1,128,'@')
        sip += '@'
        sip += randomString(1,128,'@')
        sip += '.com'
        return sip
    
            
class TestAddressBook(unittest.TestCase):
    
    def setUp(self):
        print "setup"
        self.contactList = []
        self.adb = AddressBook(randomEncoding(randomString(5,128)))
        self.test_add()
 
    
    def test_add(self):
        print "test_add"
        for i in range(1,random.randint(1,10)):
            ncontact = randomString(5,128)
            contact = randomEncoding(ncontact)
            self.contactList.append(contact)
            self.adb.add(contact, randomAddress())
 
        
    def test_modify(self):
        print "test_modify"
        for i in range(len(self.contactList)):
            ncontact = randomEncoding(randomString(5,128))
            
            self.adb.select(self.contactList[i])
            curr = self.adb.get_current()
            
            self.adb.modify(self.contactList[i], ncontact, randomAddress())
            self.contactList[i] = ncontact
            
            self.adb.select(self.contactList[i])
            curr2 = self.adb.get_current()
            
            #on test que le pointeur select ne soit pas modifier par la modif du contact
            assert (curr != curr2), self.fail('probleme de select apres modification du contact')

       
    def test_select(self):
        print "test_select"
        for i in range(len(self.contactList)+random.randint(0,10)):
            if i < len(self.contactList) :
                self.adb.select(self.contactList[i])
            else:
                self.adb.select(randomString(5,128))
 
    
    def test_remove(self):
        print "test_remove"
        for i in range(len(self.contactList) + random.randint(0,10)):
            if i < len(self.contactList) :
                self.adb.remove(self.contactList[i])
            else:
                self.adb.remove(randomString(5,128))
                
                                        
    def runTest(self):
        pass
    
if __name__ == "__main__":
    #print randomEncoding(randomString(5,50))
    
    suite = unittest.TestSuite()
    suite.addTest(TestAddressBook("test_add"))
    suite.addTest(TestAddressBook("test_modify"))
    suite.addTest(TestAddressBook("test_select"))
    suite.addTest(TestAddressBook("test_remove"))
    unittest.main()
    
    #rapport :
    #list des char qui ne pass pas (0xXX or \xXX) : 88 ae 93 a9 82 ba 8f 8a a0 80 8c b9 94 92 a8 b4 a7 94