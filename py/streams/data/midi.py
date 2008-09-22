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

# App imports
from protocols import osc_protocols
from streams import stream

#Midi Import
import pypm
from midiIn import MidiIn
from RTPServer import RTPServer
from twisted.internet import reactor

#Log import
from utils import log

#redexp
import re

#log = log.start('info', 1, 0, 'midiStream')

class MidiStream(stream.DataStream):
    """Class MIDI
    """
    
    def __init__(self, address='127.0.0.1'):
        
        #init var
        self.address = address
                
        #initialisation du midi
        pypm.Initialize()
	   
        #Configuring RTP Server
        #witness for receiving data is self.server.receivingMidiData
        #latency is self.server.midiOut.latency
        #midi output device list is in self.midiOut.midiDeviceList
        self.server = RTPServer()

        #listenUDP(port sur lequel on lit les donnees midi, server)
        reactor.listenUDP(44000,self.server)

        #MidiIn ( "adresse sur lequel envoyer les donnees midi" , port sur lequel envoyer les midi data)
        #midi input device list is in self.midiIn.midiDeviceList
        #witness for sending data is in self.midiIn.sendingMidiData
        self.midiIn = MidiIn(self.address,44000)

        reactor.run()


    def set_ip(self,address):
        """Set ip address of the midi stream server
        """
        if self.check_ip(address):
            self.address = address
            self.midiIn.address.client.peerAddress = address
            log.info("IP address of the server has been set")
        else:
            self.address = '127.0.0.1'
            log.warning("A loopback address has been assign")

            
    def check_ip(self,address):
        if re.match('^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$', address):
            values = [int(i) for i in address.split('.')]
            if self.ip_range(values):
                return 1
            else:
                log.error("Wrong ip address format")
                return 0
                
        else:
            log.error("Wrong ip address format")
            return 0
            

    def ip_range(self, nums):
        for num in nums:
            if num < 0 or num > 255:
                return False
            return True
 
    
#INPUT	
    def start_sending(self):
        """function start_sending        
        returns 0 if can start sending else -1
        """
        res = self.midiIn.start_sending()
        return res
    

    def stop_sending(self):
        """function stop_sending
        """
        res = self.midiIn.stop_sending()


    def set_input_device(self, device):
        """function set_input_device, setting midi input device
        """
        self.midiIn.set_device(device)

		
    def get_input_devices(self):
        """function get_input_device , return list of midi devices
        """
        self.midiIn.get_devices()
        return getattr(self.midiIn, "midiDeviceList")


    def get_sending_witness(self):
        return getattr(self.midiIn.client, "sendingMidiData")


    def get_client_sync_witness(self):
        return getattr(self.midiIn.client, "sync")
		

#OUTPUT		
    def start_receving(self):
        """function start_receving
        returns 0 if started else -1
        """
        res = self.server.start_receiving()
        return res

    
    def stop_receiving(self):
        """function stop_receving
        returns 
        """
        self.server.stop_receiving()


    def set_output_device(self, device):
        """function set_input_device
        """
        self.server.midiOut.set_device(device)
 
   
    def get_output_devices(self):
        """function get_input_device, return list of midi device
        """
        self.server.midiOut.get_devices()
        return getattr(self.server.midiOut, "midiDeviceList")

		
    def set_latency(self, latency):
        """function set output latency
        """
        if ( latency >= 0 ):
            setattr(self.server.midiOut, "latency", latency)


    def get_receiving_witness(self):
        """ return 1 if receiving data 0 if not
        """
        return getattr(self.server, "receivingMidiData")


    def get_server_sync_witness(self):
        """return the sync witness, 1 if sync , 0 if not
        """
        return gettattr(self.server, "sync")


    def __del__(self):

        del self.server
        del self.midiIn
		
        #Ending pyport midi
        pypm.Terminate()	




  

    
