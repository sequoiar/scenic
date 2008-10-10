from twisted.internet import protocol, reactor, task, defer

from midiObject import MidiNote

from listCirc import MidiTimeCirc
from ringBuffer import myRingBuffer
import pypm
from utils import log
import time


INPUT = 0
OUTPUT = 1

class MidiOut(object):
	
	def __init__(self, permissif):
		
		log.info ("OUTPUT: Initialize midi output")
		
		self.midiDeviceList = []
		self.MidiOut = None
		self.midiDevice = None
		self.latency = 20
		
		#midiTimeDiff => difference between the midi time 
		#of the server and the midi time of the client
		self.midiTimeDiff = 0

		#time for a round trip packet
		self.delay = 0
		
		log.info("OUTPUT: Setting midi output looping task")
		#self.publy = task.LoopingCall(self.publy_midi_note)
		
		self.lastMidiTimeDiff = MidiTimeCirc(25)

		#stat
		self.nbNote = 0
		self.nbXRun = 0
		self.startChrono = 0
		self.midiOutCmdList = myRingBuffer()
		
		#flag
		self.isListening = 0
		self.publy_flag = False
		self.start_time = 0
		self.permissif = permissif
		
		#tmp
		self.lastTime = 0
		


	def start_publy(self):
		"""Start publying notes
		"""		
		self.send_note_off()
		self.publy_flag = True
		reactor.callInThread(self.publy_midi_notes)



	def stop_publy(self):
		"""Stop publy
		"""
		#if ( self.publy.running):
			#self.publy.stop()
		self.publy_flag = False
		self.send_note_off()
		#reinitialize midi time difference average
		self.lastMidiTimeDiff.flush()

		
	def sync_midi_time(self,time):
		"""Sync set the difference between local midi time and
	    remote midi time in order to apply it to the notes
		"""
		if pypm.Time() >= int(time) :
			self.lastMidiTimeDiff.to_list(pypm.Time() - int(time))
		else:
			self.lastMidiTimeDiff.to_list(-(int(time) - pypm.Time()))
		
		#midiTime diff recoit la moyenne des dernier tps calculer
		self.midiTimeDiff = self.lastMidiTimeDiff.average()
		
		#Checking if the delay between the two machine is highter than the current latency
		if ( self.latency <= self.delay ):
			l = "OUTPUT: Can't play on time = delay between hosts is higher than the latency !"
			log.error(l)

	
	def get_devices(self):
		"""list and set midi device
		"""
		self.midiDeviceList = []
		for loop in range(pypm.CountDevices()):
			interf, name, inp, outp, opened = pypm.GetDeviceInfo(loop)
			if outp == 1:
				self.midiDeviceList.append([loop, name, opened])


	def set_device(self, device):
		"""set output midi device
		"""

		self.get_devices()
		try:
			dev = int(device)
		except ValueError:
			log.error("OUTPUT: Enter a number for midi input device")

		#check if device exist
		devList = [self.midiDeviceList[i][0] for i in range(len(self.midiDeviceList))]	
		if dev in devList :
			self.midiDevice = dev

			if self.MidiOut is None :
				#Initializing midi input stream
				self.MidiOut = pypm.Output(self.midiDevice, 0)
				return 0
			else:
				#delete old midi device
				del self.MidiOut
				#Initializing new midi input stream
				self.MidiOut = pypm.Output(self.midiDevice, 0)
				log.info( "OUTPUT: Input device is set up")
				return 0
		else:
			log.error("OUTPUT: Incorrect device selected, can't setting up")
			return -1

		
	def get_device_info(self):
		"""print info of the current device
		"""
		return pypm.GetDeviceInfo(self.device)

	
	def send_note_off(self):
		"""send Note Off in case of broken connection
		"""	
		midiNotes = []
		for i in range(0,127):
			midiNotes.append([[0x80,i,100], pypm.Time()])

		self.MidiOut.Write( midiNotes) 

		
	def midi_time(self):
		return pypm.Time()
		
#Permisive Mode => joue toutes les notes meme si en retard de qq milisecond en affichant un erreur style xrun dans le fichier de log

		
	def play_midi_note(self,midiNotes):
		"""PlayMidi Note
	   	Separate midi infos to choose the good function for the good action
		"""
		
		if self.permissif :
			#on fait une liste des notes en retard pour le signaler
			new_list = [midiNotes[i][1] for i in range(len(midiNotes)) if (pypm.Time() >  midiNotes[i][1]) ]

			if ( len(new_list) > 0 ) :
				self.nbXRun += 1
				l = "OUTPUT: time=" + str(pypm.Time()) + "ms  can't play in time , " + str(len(midiNotes)) + " notes - late of " + str(pypm.Time() - new_list[0]) + " ms"
				log.error(l)
			note_filtred = midiNotes
		else:
		#filtre note off program change pour note en retard
		#si mode non permissif on skip les note enn retard sauf note off et program change
			note_filtred = [midiNotes[i] for i in range(len(midiNotes)) if (midiNotes[i][1] >= pypm.Time() or midiNotes[i][0][0] == int(0xc0) or midiNotes[i][0][2] == 0) ]

			if ( len(note_filtred) < len(midiNotes)):
				l = "OUTPUT: time=" + str(pypm.Time()) + "ms can't play in time,  " + str(len(midiNotes)-len(note_filtred)) + " note(s) skipped"
				log.error(l)
				
        #Playing note on the midi device
		self.MidiOut.Write(note_filtred)	

	def publy_midi_notes(self):
		d = defer.Deferred()
		while self.publy_flag :
			midiNotes = []
		        #if there are notes to play 
			if ( self.midiOutCmdList.avail_for_get() > 0 ):
				note = self.midiOutCmdList.buffer[self.midiOutCmdList.start]

				#if they are in time ( 4 is for processing time )
				if (note.time <= pypm.Time() + 4):
			                #get notes from the buffer
					noteList = self.midiOutCmdList.get()

			        #formating notes
					midiNotes = [ [[noteList[i].event, noteList[i].note, noteList[i].velocity], noteList[i].time] for i in range(len(noteList)) ] 
					#a garder pour faire control du nb notes
					self.nbNote += len(midiNotes)
			
			if (len(midiNotes)>0):
				reactor.callFromThread(self.play_midi_note,midiNotes)

			time.sleep(0.001)
		
		return d


			
	def __del__(self):
		self.terminate = 1
