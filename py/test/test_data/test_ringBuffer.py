#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Sropulpof
# Copyright (C) 2008 Soci�t� des arts technoligiques (SAT)
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
d = os.path.abspath('../../streams/data/')
sys.path.append(d)

__import__('ringBuffer')
from ringBuffer import myRingBuffer

__import__('midiObject')
from midiObject import MidiNote

def test_croissance(buffer):
    res = True
    for i in range(len(buffer)-1):
        if buffer[i].time > buffer[i+1].time :
            res = False
            break
    return res


class TestmyRingBuffer(unittest.TestCase):
    def setUp(self):
        self.rb = myRingBuffer(10)
        
    
    def tearDown(self):
        pass
    
    def test_len(self):
        #ajout de 0 a 7
        for i in range(0,8):
            self.rb.put(MidiNote(i, 0, 0, 0))
            
        res = self.rb.len()
        assert(res == 8), self.fail('Problem getting the correct len of the data in the ring buffer')
        
    def test_avail_for_get(self):
        #ajout de 0 a 7
        for i in range(0,8):
            self.rb.put(MidiNote(i, 0, 0, 0))
            
        res = self.rb.avail_for_get()
        assert(res == 8), self.fail('Problem getting the correct value for avail_for_get data')     
        
    def test_avail_for_put(self):
                #ajout de 0 a 7
        for i in range(0,8):
            self.rb.put(MidiNote(i, 0, 0, 0))
            
        res = self.rb.avail_for_put()
        assert(res == 2), self.fail('Problem getting the correct value for avail_for_put')   
        
        
    def test_flush(self):
                #ajout de 0 a 7
        for i in range(0,8):
            self.rb.put(MidiNote(i, 0, 0, 0))
            
        self.rb.flush()
        
        res = self.rb.len()
        assert(res == 0), self.fail('Problem getting the correct len of the data in the ring buffer')
        
        res = self.rb.avail_for_get()
        assert(res == 0), self.fail('Problem getting the correct value for avail_for_get data') 
        
        res = self.rb.avail_for_put()
        assert(res == 10), self.fail('Problem getting the correct value for avail_for_put') 
        
   
    def test_put(self):
    #ajout de 0 a 7
        for i in range(0,8):
            self.rb.put(MidiNote(i, 0, 0, 0))
        assert(self.rb.len() == 8), self.fail('Problem getting the correct number of midi data insert')     
            
        
                    
    def test_get(self):
        for i in range(0,8):
            self.rb.put(MidiNote(i, 0, 0, 0))
            
        lent = self.rb.len()
        avail = self.rb.avail_for_get()   
        tmp = self.rb.get()
        size = len(tmp)
        
        assert(lent == avail), self.fail('Difference between buffer len and len data avail for get')
        assert(avail == size), self.fail('Difference between avail for get and len of buffer get')
               
    def test_find_place(self):
        #cas n°1 = insertion au milieu
        for i in range(0,5):
            self.rb.put(MidiNote(i, 0, 0, 0))     
        self.rb.put(MidiNote(15,0,0,0))
        
        last_len = self.rb.len()
        #ajout d'une note au milieu
        self.rb.put(MidiNote(8, 0, 0, 0))
        
        assert( self.rb.len() == last_len + 1 ) , self.fail('Problem of len after a find place')
        
        #checking that the note is in the right place
        buffer = self.rb.get()   
        res = test_croissance(buffer)
        assert(res == True), self.fail('Problem with find place, the buffer is not crescent anymore')
            
    def test_find_place_2(self):
        #cas n°2 = insertion en tete du buffer
        for i in range(0,9):
            self.rb.put(MidiNote(i, 0, 0, 0))     
        
        self.rb.get()
        
        self.rb.put(MidiNote(10,0,0,0))
        self.rb.put(MidiNote(17,0,0,0))
        
        
        self.rb.put(MidiNote(12, 0, 0, 0))
            
        assert( self.rb.len() == 3) , self.fail('Problem of len after a find place')
        
        #checking that the note is in the right place
        buffer = self.rb.get()   
        res = test_croissance(buffer)
        assert(res == True), self.fail('Problem with find place, the buffer is not crescent anymore')
               

    def test_find_place_3(self):
        #cas n°2 = insertion en queue du buffer
        for i in range(0,9):
            self.rb.put(MidiNote(i, 0, 0, 0))     
        
        self.rb.get()
        
        self.rb.put(MidiNote(10,0,0,0))
        self.rb.put(MidiNote(17,0,0,0))       
        
        self.rb.put(MidiNote(9, 0, 0, 0))
            
        assert( self.rb.len() == 3) , self.fail('Problem of len after a find place')
        
        #checking that the note is in the right place
        buffer = self.rb.get()   
        res = test_croissance(buffer)
        assert(res == True), self.fail('Problem with find place, the buffer is not crescent anymore')
               