import time

#motherclass
class ListCirc(object):
	
	def __init__(self, listeSize):
		self.index = 0
		self.list = []
		self.round = 0
		self.listeSize = listeSize	
		
	def to_list(self, note):
		if self.round == 1:
			self.replace_note(note)
		else:
			self.add_note(note)
	
	def add_note(self, note):
		self.index += 1
		self.list.append(note)
		
		if self.index == self.listeSize - 1:
			self.round = 1
			self.index = 0

	def replace_note(self, note):
		self.index += 1
		if (self.index == self.listeSize - 1):
			self.index = 0
			self.list[self.index] = note
		else:
			self.list[self.index] = note
			

	def flush(self):
		self.list = []
		self.round = 0
		self.index = 0

    	def __getitem__(self,key):
        	return self.list[key]		
	


#list for sent packet in order to retreive them in case of lose on the network
from midiObject import packetTime

class PacketCirc(ListCirc):
		
	#Recupere une note avec son compteur (numero)		
	def find_packet(self, seqNo):
		
		packet = [i for i in range(len(self.list)) if (self.list[i].seqNo == seqNo)] 
		
		if ( len(packet) > 0 ):
			return packet[0]
		else:
			return -1


#list to stock last delays between the two computers ( time of a trip packet )
class DelayCirc(ListCirc):

	def __init__(self, listeSize):
		ListCirc.__init__(self, listeSize)
		self.lastSync = 0

	def to_list(self,note):
		ListCirc.to_list(self,note)
		self.lastSync = time.time()
	
	def average(self):
		if (len(self.list)>0):
			average = float(sum(self.list)/len(self.list))
			return average

#list to stock last miditime difference
class MidiTimeCirc(DelayCirc):
	
	def split(self):
		if ( len(self.list)>0):
			split =  max(self.list) - min(self.list)
      			return abs(split)
		else:
			return 0
