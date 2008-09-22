import pypm
from twisted.internet import protocol, reactor, task

from utils import log
import time
from midiObject import MidiNote
from RTPClient import RTPClient

INPUT = 0
OUTPUT = 1

class MidiIn():

    def __init__(self, peerAddress="127.0.0.1", port=44000):
	        
        #Init var
        self.midiDeviceList = []
       	self.midiDevice = None
        self.MidiIn = None
        self.intervalTimeUpdate = 500
        #attention verifier que le manque de set device n'est pas un pb ??????????
				
        #setting looping task
        self.releaser = task.LoopingCall(self.get_input)
        self.sendTime = task.LoopingCall(self.send_midi_time)
		
        #Launching RTP Client
        self.client = RTPClient(peerAddress, port)
        reactor.listenUDP(port+10, self.client)
        
       	#launching midi listener
       	log.info("INPUT: Launching midi input listener")

        #stats
        self.nbNote = 0

        #flag ( 1 == syncronyse with remote peer )
        self.sync = 0

        #starting looping task
        self.sendTime.start(0.5)

    def stop_sending(self):
        if ( self.releaser.running):
            self.releaser.stop()
        
        self.client.stop_streaming()
	
	
    def start_sending(self):
        if (not self.releaser.running):
            if ( not (self.MidiIn is None) ):
                self.releaser.start(0.001)
                res = self.client.start_streaming()
                return res
            else:
                log.info("INPUT: you have to set a midi device before start sending midi data")
                return -1
            
		    
    def get_devices(self):
	"""Print midi input device list
	"""
        self.midiDeviceList = []
        for loop in range(pypm.CountDevices()):
            interf, name, inp, outp, opened = pypm.GetDeviceInfo(loop)
            if inp == 1:
                self.midiDeviceList.append([loop,name, opened])


    def set_device(self, device):
	"""select midi device
	""" 
        try:
            dev = device
        except ValueError:
            log.error("INPUT: Enter a number for midi input device")

        #check if device exist
        devList = [self.midiDeviceList[i][0] for i in range(len(self.midiDeviceList))]	
        if ( dev in devList ):
            self.midiDevice = dev
            log.info( "INPUT: Midi Input selected")

            if self.MidiIn is None:
                #Initializing midi input stream
                self.MidiIn = pypm.Input(self.midiDevice)
            else:
                #delete old midi device
                del self.MidiIn
                #Initializing new midi input stream
                self.MidiIn = pypm.Input(self.midiDevice)
                
                log.info( "INPUT: Input device is set up")
        else:
            log.error("INPUT: incorrect device selected")
        

    def get_input(self):
	"""Get input from selected device 
	"""
        self.intervalTimeUpdate += 1
        
        if self.MidiIn.Poll():
            currentTime = pypm.Time()
            self.client.startChrono.append(time.time())
            #Reading Midi Input
            MidiData = self.MidiIn.Read(1024)

            for i in range(len(MidiData)):                   
                note = MidiNote(currentTime,MidiData[i][0][0], MidiData[i][0][1], MidiData[i][0][2], MidiData[i][0][3])
                #Adding the note to the buffer
                self.client.midiInCmdList.put(note)
                    


    def send_midi_time(self):
        """send midi time to the remote peer in order to syncronise midi event
		"""
        self.client.send_midi_time(pypm.Time())
                    

    def __del__(self):
        #Stoping looping task
        self.releaser.stop()

        #deleting objects 
        del self.client
        del self.MidiIn
