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

from twisted.trial import unittest
import sys
import random
import os

#import address_book
d = os.path.abspath('../')
sys.path.append(d)
__import__('addressbook')
from addressbook import AddressBook


def generateString(sauf = ''):
    alphabet = 'abcdefghijklmnopqrstuvwxyz_@12345-6789.0!?=+%$()[]{}#<>€£éèêëà§çπ‡Ò∂ƒﬁ~'
    pbchar = '/"\'`¬µ‹≈©◊≠÷'
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


def generateAddress():
    addressList = []
    #ip address
    ip = ''
    for i in range(3):
        ip += '255.'
    ip += '255'
    addressList.append(ip)
    
    #sip name
    domain = generateString('@_-!?=+%$()[]{}#<>€£éèêëà§çπ‡Ò∂ƒﬁ°~')
    domain += '@'
    domain += generateString('@_-!?=+%$()[]{}#<>€£éèêëà§çπ‡Ò∂ƒﬁ°~')
    domain += '.com'
    addressList.append(domain)
    
    #SIP ip
    sip = 'sip:'
    sip = generateString('@_.-!?=+%$()[]{}#<>€£éèêëà§çπ‡Ò∂ƒﬁ°~')
    sip += '@'
    sip += '255.255.255.255'
    addressList.append(sip)   

    #multicast 
    multicast = '224.255.255.255'
    addressList.append(multicast)
    
    return addressList

def generateBadFormatAddress():
    addressList = []
    #ip address
    ip = ''
    for i in range(3):
        ip += '300.'
    ip += '2555'
    addressList.append(ip)
    #domain address
    domain = 'http://'
    domain += generateString('€£éèêëà§çπ‡Ò∂ƒﬁ~')
    addressList.append(domain)
    #SIP Address
    sip = generateString('€£éèêëà§çπ‡Ò∂ƒﬁ~')
    addressList.append(sip)  
    return addressList

            
class TestAddressBook(unittest.TestCase):
    
    def setUp(self):
        self.contactList = []
        #entre parenthèse les char interdit pour adb name
        name = generateString('€£éèêëà§çπ‡Ò∂ƒﬁ~')
        encodedName = generateEncoding(name)
        self.name = name
        #test des diff type d'encodage
        for i in range(len(encodedName)):
            self.adb = AddressBook(encodedName[i])
               
    def tearDown(self):
        del self.adb
        del self.contactList
        #delete test files
        os.remove(os.environ['HOME'] + '/.' + str(self.name) + '/' + str(self.name) + '.adb')
        os.rmdir(os.environ['HOME'] + '/.' +str(self.name))
        
    def test_add(self):
        #generate contact name
        ncontact = generateString('%$()[]{}#<>€£éèêëà§çπ‡Ò∂ƒﬁ°~-_')
        contact = generateEncoding(ncontact)     
         
        #testing bad address format
        addressList = generateBadFormatAddress()
        for i in range(len(contact)):        
            for j in range(len(addressList)):
                res = self.adb.add(contact[i], addressList[j])
                assert ( res == False ) , self.fail(str(j) + " " + str(contact[i]) + " "+ str(addressList[j])+ 'Problem , can add bad address format contact')
        

        print "test good address"
        #testing good address format
        del addressList
        addressList = generateAddress()
        for i in range(len(contact)):
            self.contactList.append(contact[i])
            for j in range(len(addressList)):
                
                res = self.adb.add(contact[i], addressList[j])
                assert(res == True), self.fail('problem adding good formating contact') 
                
                res = self.adb.remove(contact[i])
                assert(res == True), self.fail('problem removing existing contact') 

    def test_modify(self):
        #on ajoute les contact a modifier
        name = generateString('€£éèêëà§çπ‡Ò∂ƒﬁ~')
        ad = generateAddress()
        res = self.adb.add(name, ad[0])
        assert(res == True), self.fail('problem adding good formating contact') 

        #nouveau contact
        ncontact = generateString('€£éèêëà§çπ‡Ò∂ƒﬁ~')
        ncontactEncoded = generateEncoding(ncontact)
        actContact = name
        adList = generateAddress()
        #on test la modif sur tout les types d'encodages
        for j in range(len(adList)):
            #testing select over modify
            res = self.adb.select(actContact)
            assert(res == True), self.fail('Problem selecting a contact')
            
            #testing get current over modify
            curr = self.adb.get_current()
            assert(curr != None), self.fail('Problem getting current contact')
                       
            res = self.adb.modify(actContact, actContact, adList[j])
            assert ( res == True), self.fail('Problem modifying a contact')
            
            curr2 = self.adb.get_current()
            assert ( res != None), self.fail('Problem getting current contact')
            
            #on test que le pointeur select ne soit pas modifier par la modif du contact
            assert (curr != curr2), self.fail('probleme get current is on the oldest instance of the contact')

 
    def test_select(self):
        #on ajoute les contact a select
        name = generateString('£éèêëà§çπ‡Ò∂ƒﬁ~')
        ad = generateAddress()
        self.contactList.append(name)
        res = self.adb.add(name, ad[0])
        assert(res == True), self.fail('problem adding good formating contact') 
                
        for i in range(len(self.contactList)+1):
            if i < len(self.contactList) :
                a = self.adb.select(self.contactList[i])
                assert(a == True), self.fail('problem select , can\'t found element with select .')
            else:
                name = generateString()
                nameEncoded = generateEncoding(name)
                for i in range(len(nameEncoded)):
                    a = self.adb.select(nameEncoded[i])
                    assert(a == False), self.fail('problem de select , find a non existing element .')   
   
   
    def test_remove(self):
        #on ajoute les contact a modifier
        name = generateString()
        ad = generateAddress()
        self.contactList.append(name)
        res = self.adb.add(name, ad[0])
        assert(res == True), self.fail('problem adding good formating contact') 
        
        for i in range(len(self.contactList) + 1):
            if i < len(self.contactList) :
                a = self.adb.remove(self.contactList[i])
                assert(a == True), self.fail('can\'t locate the element in order to delete it.')
            else:
                name = generateString()
                nameEncoded = generateEncoding(name)
                for i in range(len(nameEncoded)):
                    res = self.adb.remove(nameEncoded[i])
                    assert(res == False), self.fail('removing non existing element') 
  
    def test_get_current(self):
        #on ajoute les contact a getter
        name = generateString()
        ad = generateAddress()
        self.contactList.append(name)
        res = self.adb.add(name, ad[0])
        assert(res == True), self.fail('problem adding good formating contact') 
        
        for i in range(len(self.contactList)):
            actContact = self.contactList[i]
            
            res = self.adb.select(actContact)
            assert(res == True), self.fail('Problem selecting contact')
            
            res = self.adb.get_current()
            assert(res != None), self.fail('Problem getting current contact')
    
    def test_read(self):
        #on ajoute un contact au moins 
        name = generateString()
        ad = generateAddress()
        self.contactList.append(name)
        res = self.adb.add(name, ad[0])
        assert(res == True), self.fail('problem adding good formating contact') 
        
        self.adb.read()
        
    def test_write(self):
        #on ajoute un contact au moins 
        name = generateString()
        ad = generateAddress()
        self.contactList.append(name)
        res = self.adb.add(name, ad[0])
        assert(res == True), self.fail('problem adding good formating contact') 
        
        self.adb.write()
  
from addressbook import Contact
  
class TestContact(unittest.TestCase):
    
    def setUp(self):
        ad = generateAddress()
        self.contact = Contact(generateString(), ad[0])
   
        
    def tearDown(self):
        del self.contact 
   
   
    def test_contact(self):
        ad = generateAddress()
        Contact(generateString(), ad[0])      
        

    def test_type(self):
        ad = generateAddress()
        
        self.contact.address = ad[0]
        res = self.contact.type()
        assert(res == 'ip'), self.fail('problem detecting good format ip address')
        
        self.contact.address = ad[1]
        res = self.contact.type()
        assert(res == 'sip_name'), self.fail('problem detecting good format sip name')
        
        self.contact.address = ad[2]
        res = self.contact.type()
        assert(res == 'sip_ip'), self.fail('problem detecting good format sip ip')
        
        self.contact.address = ad[3]
        res = self.contact.type()
        assert(res == 'multicast'), self.fail('problem detecting good format multicast ip')
                                                
    #rapport :
    #problem d'encodage utf8 pour le char 0xFF => levé l'erreur UnicodeDecodeError pour AdressBook.add
    #problem d'encodage utf8 pour le char 0xFF => levé l'erreur UnicodeDecodeError pour AdressBook.__init__()
    #Attention avec les différent type d'encodage
    #list des char qui ne pass pas (0xXX or \xXX) : 88 ae 93 a9 82 ba 8f 8a a0 80 8c b9 94 92 a8 b4 a7 94