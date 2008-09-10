class MidiNote():

	def __init__(self, time, event, note, velocity, what=0):
		self.time = time
		self.event = event
		self.note = note
		self.velocity = velocity


class packetTime():
	
    def __init__(self, seqNo, packet, time=0):
        self.seqNo = seqNo
        self.packet = packet
        self.time = time
	
	