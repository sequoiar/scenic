class MidiNote(object): #TODO: Maybe we could convert this object to a tuple to improve the performance

	def __init__(self, time, event, note, velocity, what=0):
		self.time = time
		self.event = event
		self.note = note
		self.velocity = velocity


class packetTime(object):
	
    def __init__(self, seqNo, packet, marker=0):
        self.seqNo = seqNo
        self.packet = packet
        self.marker = marker
	
	
