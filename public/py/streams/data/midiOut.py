from twisted.internet import protocol, reactor, task

from midiObject import MidiNote

from listCirc import MidiTimeCirc
from ringBuffer import myRingBuffer
import pypm
from utils import log
import time


INPUT = 0
OUTPUT = 1

class MidiOut():
	
	def __init__(self, permisif):
		
		log.info ("OUTPUT: Initialize midi output")
		
		self.midiDeviceList = []
		self.MidiOut = None
		self.midiDevice = None
		self.latency = 1
		
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

		#tmp
		self.lastTime = 0


	def start_publy(self):
		"""Start publying notes
		"""
		#if ( not self.publy.running):
		if (not self.MidiOut is None):
			self.send_note_off()
				#self.publy.start(0.001)
		else:
			log.info("can't publy note if no midi device is set")


	def stop_publy(self):
		"""Stop publy
		"""
		#if ( self.publy.running):
			#self.publy.stop()
		self.send_note_off()
		#reinitialize midi time difference average
		self.lastMidiTimeDiff.flush()

		
	def sync_midi_time(self,time):
		"""Sync set the difference between local midi time and
	    remote midi time in order to apply it to the notes
		"""
		if (pypm.Time() >= int(time)):
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
		print str(pypm.GetDefaultOutputDeviceID())
		self.midiDeviceList = []
		for loop in range(pypm.CountDevices()):
			interf, name, inp, outp, opened = pypm.GetDeviceInfo(loop)
			if outp == 1:
				self.midiDeviceList.append([loop, name, opened])


	def set_device(self, device):
		"""set output midi device
		"""
		try:
			dev = int(device)
		except ValueError:
			log.error("OUTPUT: Enter a number for midi input device")

		#check if device exist
		devList = [self.midiDeviceList[i][0] for i in range(len(self.midiDeviceList))]	
		if (dev in devList):
			self.midiDevice = dev

			if (self.MidiOut is None):
				#Initializing midi input stream
				self.MidiOut = pypm.Output(self.midiDevice, 0)
			else:
				#delete old midi device
				del self.MidiOut
				#Initializing new midi input stream
				self.MidiOut = pypm.Output(self.midiDevice, 0)
				log.info( "OUTPUT: Input device is set up")
		else:
			log.error("OUTPUT: Incorrect device selected, can't setting up")

		
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
		
		#Playing note on the midi device
		self.MidiOut.Write(midiNotes)

		new_list = [midiNotes[i][1] for i in range(len(midiNotes)) if (pypm.Time() >  midiNotes[i][1]) ]

		for i in range(len(new_list)):
			self.nbXRun += 1
			print "OUTPUT: " + str(pypm.Time()) + " xrun no. " + str(self.nbXRun) + " can't play in time , " + str(len(midiNotes)) + " notes - late of " + str(pypm.Time() - new_list[i]) + " ms"

		
	def publy_midi_note(self):
		midiNotes = []
		#if there are notes to play 
		if ( self.midiOutCmdList.avail_for_get() > 0 ):
			
			note = self.midiOutCmdList.buffer[self.midiOutCmdList.start]
			
#if they are in time ( 2 is for processing time )
			if (note.time + self.midiTimeDiff - int(self.delay) + self.latency <= pypm.Time() + 3):
				
				#get notes from the buffer
				noteList = self.midiOutCmdList.get()
				if ( self.lastTime > noteList[0].time):
					print "______________________ERROR____________ERROR___________"
				
				self.lastTime = noteList[0].time
				for i in range(len(noteList)):
					currentNote = noteList[i]
					midiNotes.append( [[currentNote.event,currentNote.note,currentNote.velocity], currentNote.time + self.midiTimeDiff + self.latency - int(self.delay)])

		#if (len(midiNotes)>0):
				self.play_midi_note(midiNotes)
			else:
				pass
				#print 'to early'
				#print str(note.time + self.midiTimeDiff - int(self.delay)+ self.latency) + ' <= cmp to ' + str (pypm.Time() + 2)
			
	
	def __del__(self):
		self.terminate = 1
