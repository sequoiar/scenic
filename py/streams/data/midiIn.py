import pypm
from twisted.internet import protocol, reactor, task, defer
from utils import log
import time
from midiObject import MidiNote
from RTPClient import RTPClient

from twisted.internet import threads

INPUT = 0
OUTPUT = 1

class MidiIn(object):

    def __init__(self, peerAddress="127.0.0.1", port=44000):
	        
        #Init var
        self.midiDeviceList = []
       	self.midiDevice = None
        self.MidiIn = None
				
        #setting looping task
        self.releaser = task.LoopingCall(self._get_input)
        self.sendTime = task.LoopingCall(self._send_midi_time)
		
        #Launching RTP Client
        self.client = RTPClient(peerAddress, port)
        self.listen_c = reactor.listenUDP(port+10, self.client)
        
       	#launching midi listener
       	log.info("INPUT: Launching midi input listener")

        #stats
        self.nbNotes = 0
        self.fps_note = 0
        
        #flag ( 1 == syncronyse with remote peer )
        self.sync = 0
        self.end_flag = True

        #check activity
        self.last_activity = 0
        self.in_activity = False

        #starting looping task
        self.sendTime.start(1)

        
    def stop_sending(self):
        if not self.end_flag:
            self.end_flag = True
            
            self.client.stop_streaming()
            
            #self.client.midiInCmdList.flush()
	
    def _polling(self):
        #need by twisted to stop properly the thread
        d = defer.Deferred()

        #setting new scopes
        last_poll = self.last_activity
        MidiIn = self.MidiIn
        in_activity = self.in_activity

        while not self.end_flag:
            if MidiIn.Poll():
                last_poll = time.time()*1000
                reactor.callFromThread(self._get_input)
                in_activity = True

            if in_activity and ((time.time() * 1000) - last_poll >= 5):
                #send two empty packet
                self.client.send_empty_packet() 
                self.client.send_empty_packet() 
                in_activity = False
            
                
            time.sleep(0.001)

        return d


    def start_sending(self):
        if self.end_flag :
            if not (self.MidiIn is None) :
                res = self.client.start_streaming()
                if res != -1:
                    self.end_flag = False
                    reactor.callInThread(self._polling)    
                return res
            else:
                log.warning("INPUT: you have to set a midi device before start sending midi data")
                return -1
        else:
            log.warning("INPUT: already sending midi data")
            return -1
            
		    
    def get_devices(self):
	"""Print midi input device list
	"""
        self.midiDeviceList = []
        for loop in range(pypm.CountDevices()):
            interf, name, inp, outp, opened = pypm.GetDeviceInfo(loop)
            if inp == 1:
                self.midiDeviceList.append([loop,name, opened])

    def get_device_info(self):
		"""print info of the current device
		"""
		return pypm.GetDeviceInfo(self.midiDevice)

    def set_device(self, device):
	"""select midi device
	""" 
        self.get_devices()

        try:
            dev = device
        except ValueError:
            log.error("INPUT: Enter a number for midi input device")

    
        #check if device exist
        devList = [self.midiDeviceList[i][0] for i in range(len(self.midiDeviceList))]	
        if dev in devList :
            self.midiDevice = dev

            if self.MidiIn is None:
                #Initializing midi input stream
                self.MidiIn = pypm.Input(self.midiDevice)
                l = "INPUT: " + str(self.get_device_info()[1]) + " has been set as input device"
                log.info(l)
                return 0
            else:
                #delete old midi device
                del self.MidiIn
                #Initializing new midi input stream
                self.MidiIn = pypm.Input(self.midiDevice)
                l = "INPUT: " + str(self.get_device_info()[1]) + " has been set as input device"
                log.info(l)
                return 0
        else:
            log.error("INPUT: incorrect device selected")
            return -1
        

    def _get_input(self):
	"""Get input from selected device 
	""" 
        currentTime = pypm.Time()

        #Reading Midi Input
        MidiData = self.MidiIn.Read(1024)
        
        if len(MidiData) > 0:
            [self.client.midiInCmdList.put([MidiData[i][0],currentTime]) for i in range(len(MidiData))]
            #self.client.send_midi_data()   


    def _send_midi_time(self):
        """send midi time to the remote peer in order to syncronise midi event
		"""
        self.client.send_midi_time(pypm.Time())
                    

    def __del__(self):
        #Stoping looping task
        self.sendTime.stop()
        
        #deleting objects 
        del self.client
        del self.MidiIn
