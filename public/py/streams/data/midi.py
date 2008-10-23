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
#from protocols import osc_protocols
#from streams import stream

#Midi Import

from twisted.internet import glib2reactor
glib2reactor.install()

from twisted.internet import reactor
import pypm
from midiIn import MidiIn
from RTPServer import RTPServer

#Log import
from utils import log
import sys
#redexp
import re
import signal
log = log.start('debug', 1, 0, 'midiStream')

#class MidiStream(stream.DataStream):
class MidiStream(object):
    """Class MIDI
    """

    def __init__(self, address):


        #init var
        if self.check_ip(address):
            self.address = address
        else:
            print "Please enter a correct formated address "
            sys.exit(1)

        #initialisation du midi
        pypm.Initialize()

        #Configuring RTP Server
        #witness for receiving data is self.server.receivingMidiData
        #latency is self.server.midiOut.latency
        #midi output device list is in self.midiOut.midiDeviceList
        self.server = RTPServer('127.0.0.1')

        #listenUDP(port sur lequel on lit les donnees midi, server)
        self.listen_s = reactor.listenUDP(44000,self.server)

        #MidiIn ( "adresse sur lequel envoyer les donnees midi" , port sur lequel envoyer les midi data)
        #midi input device list is in self.midiIn.midiDeviceList
        #witness for sending data is in self.midiIn.sendingMidiData
        self.midiIn = MidiIn(self.address,44000)
        signal.signal(signal.SIGINT, self.handler)


    def handler(self, signum, frame):
        #stopping threads and reactor
        log.info("SIGINT caught! Shutting down midi module.")
        self.stop_sending()
        self.stop_receiving()

        #a enlever dans final version
        reactor.stop()
        #A mettre
        #self.listen_s.stopListenning()
        #self.midiIn.listen_c.stopListenning()

    def set_ip(self,address):
        """Set ip address of the midi stream server
        """
        if self.check_ip(address):
            self.address = address
            self.midiIn.client.peerAddress = address
            log.info("IP address of the server has been set")
            return 0
        else:
            self.address = '127.0.0.1'
            print "Please enter a correct formated address "
            log.error("User attemp to assign a bad formated ip")
            return -1


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
        #log.info('Sending notes has been started')
        return res


    def stop_sending(self):
        """function stop_sending
        """
        res = self.midiIn.stop_sending()
        return res

    def set_input_device(self, device):
        """function set_input_device, setting midi input device
        """
        res = self.midiIn.set_device(device)
        return res

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
    def start_receiving(self):
        """function start_receving
        returns 0 if started else -1
        """
        res = self.server.start_receiving()
        #log.info('Receiving notes has been started')
        return res


    def stop_receiving(self):
        """function stop_receving
        returns
        """
        self.server.stop_receiving()


    def set_output_device(self, device):
        """function set_input_device
        """
        res = self.server.midiOut.set_device(device)
        return res

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
        else:
            print 'You got to enter a positive latency' #print ou log?


    def get_receiving_witness(self):
        """ return 1 if receiving data 0 if not
        """
        return getattr(self.server, "receivingMidiData")


    def get_server_sync_witness(self):
        """return the sync witness, 1 if sync , 0 if not
        """
        return getattr(self.server, "sync")


    def __del__(self):

        #log.info("pyPortMidi terminate")
        del self.server
        del self.midiIn
        del self.address
        #Ending pyport midi
        pypm.Terminate()



if __name__ == "__main__":

    midi = MidiStream('127.0.0.1')

    print midi.get_input_devices()
    midi.set_input_device(2)

    print midi.get_output_devices()
    midi.set_output_device(3)


    #reactor.callLater(20, midi.stop_sending)
    #reactor.callLater(20, midi.stop_receiving)


    reactor.callLater(2,midi.start_receiving)

    reactor.callLater(2,midi.start_sending)

    reactor.run()
