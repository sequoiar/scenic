from twisted.internet import reactor
from twisted.internet import task
from twisted.internet import defer

import time

import pypm

INPUT = 0
OUTPUT = 1

VERBOSE = 0

class MidiIn(object):

    def __init__(self, client, verbose=0):
        if verbose:
            global VERBOSE
            VERBOSE = 1

        #Init var
        self.midi_device_list = []
       	self.midi_device = None
        self.midi_in = None
				
        #setting looping task
        self.releaser = task.LoopingCall(self._get_input)
		
        #Launching RTP Client to call after midi time start in order to sync TS
        self.client = client

        #stats
        self.nbNotes = 0
        self.fps_note = 0
        
        #flag ( 1 == syncronyse with remote peer )
        self.sync = 0
        self.end_flag = True

        #check activity
        self.last_activity = 0
        self.in_activity = False

        #value to set
        #in s (increase it when note frequency is high in order to reduce packet number)
        self.polling_interval = 0.015

        #in ms
        #Time out must be > to polling
        self.time_out = 5

        #Init time is for timestamp in RTP
        self.init_time = pypm.Time()


    def start(self):
        if self.end_flag :
            if not (self.midi_in is None) :
                self.end_flag = False
                reactor.callInThread(self._polling)   
                return 1

            else:
                line = "INPUT: you have to set a midi device before start "
                line += "sending midi data"
                print line
                return 0
        else:
            line = "INPUT: already sending midi data"
            return 0

        
    def stop(self):
        if not self.end_flag:
            self.end_flag = True
	

    def _polling(self):
        #need by twisted to stop properly the thread
        d = defer.Deferred()

        #setting new scopes
        last_poll = self.last_activity
        midi_in = self.midi_in
        in_activity = self.in_activity

        while not self.end_flag:
            # hasattr is workaround for weird race condition on stop whereby midi_in is an int
            if hasattr(midi_in, 'Poll') and midi_in.Poll():
                last_poll = time.time() * 1000
                reactor.callInThread(self._get_input)
                in_activity = True

            if in_activity and ((time.time() * 1000) - last_poll >= self.time_out):
                #send silent packet after 3ms of inactivity 
                self.client.send_silence()
                in_activity = False
                
            time.sleep(self.polling_interval)

        return d            
		    
    def get_devices(self):
	"""Print midi input device list
	"""
        self.midi_device_list = []
        for loop in range(pypm.CountDevices()):
            interf, name, inp, outp, opened = pypm.GetDeviceInfo(loop)
            if inp == 1:
                self.midi_device_list.append([loop,name, opened])

        return self.midi_device_list


    def get_device_info(self):
        """print info of the current device
        """
        return pypm.GetDeviceInfo(self.midi_device)

    def set_device(self, device):
	"""select midi device
	""" 
        #check if device exist
        dev_list = [self.midi_device_list[i][0]
                    for i in range(len(self.midi_device_list))]	

        if device in dev_list :
            self.midi_device = device

            if self.midi_in is not None:
                #delete old midi device if present
                del self.midi_in

            #Initializing midi input stream
            self.midi_in = pypm.Input(self.midi_device)

            if VERBOSE:
                line = "  Midi device in: " + str(self.get_device_info()[1])
                print line

            return True

        else:
            print "INPUT: Wrong midi device selected (-d to list devices)"
            print dev_list
            return False
        

    def _get_input(self):
	"""Get input from selected device 
	""" 
        current_time = pypm.Time()

        #Reading Midi Input
        midi_data = self.midi_in.Read(1024)
        if VERBOSE:
            print midi_data
        
        if len(midi_data) > 0:
            reactor.callFromThread(self.client.send_midi_data, 
                                   midi_data, current_time)   

    def __del__(self): 
        #deleting objects 
        del self.client
        del self.midi_in
