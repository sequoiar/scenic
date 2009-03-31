from twisted.internet import udp
from twisted.internet import reactor, task
from twisted.internet.protocol import DatagramProtocol
import cPickle
from struct import pack, unpack
import time
from miville.utils import log
from listCirc import DelayCirc
from ringBuffer import myRingBuffer
from midiOut import MidiOut

class RTPServer(DatagramProtocol):

    def __init__(self, streamerAddress, streamerPort=44010, permissif=False ):
        
        log.info( "OUTPUT: Initializing RTP receiver" )
        
        #Init var
        self.seqNo = 65535
        self.ssrc = 12345679
        self.peerAddress = streamerAddress		
        self.peerPort = streamerPort
        self.lastLocalTime = 0				#use to calculate the delay ( packet trip time)
        self.lastDelays = DelayCirc(20)

        #flag
        self.midiTimeSync = 0
        self.delaySync = 0
        self.sync = 0
        self.receivingMidiData = 0
        self.clientConnect = 0

        #Launching midi Out writter (permissif)
        self.midiOut = MidiOut(permissif)
        
        #looping call check sync
        self.releaser = task.LoopingCall(self.check_sync)
        self.sendTime = task.LoopingCall(self.send_local_time)
        self.nbNotes = 0
        self.start_time = 0

        #tmp
        self.lastMidiNoteTime = 0

    #self.start_receiving()

    def launch(self):
        self.releaser.start(2)
        self.sendTime.start(1)
        log.info( "OUTPUT: Server started" )
		
    def start_receiving(self):
        """start midi output and get notes from the socket
        """
        if self.sync:
            if ( not self.midiOut.MidiOut is None):
                self.midiOut.start_publy()
                log.info("OUTPUT: Server start to receive midi data")
                return 0
            else:
                log.error("OUTPUT: Can't start receiving without output device set")
                return -1
    	else:
            log.error("OUTPUT: Can't start receiving if not syncronize with client")
            return -1


    def stop_receiving(self):
        """stop midi output and stop to watch notes from the socket
        """
        #reinitialize delay between host
        self.lastDelays.flush()
            
        log.error( "OUTPUT: stopping receiving because syncronisation failed")

        #on arrete la publicatiom des notes
        self.midiOut.stop_publy()
        
        #on vide les buffers
        #self.midiOut.midiOutCmdList.flush()
        self.midiOut.lastMidiTimeDiff.flush()

        #sending note off to midi device
        self.midiOut.send_note_off()
        log.info("OUTPUT: Server stop receiving midi data")

    def check_sync(self):
        """Checking Synchronisation
        """
        #S il n y  a pas eu de sync du delay depuis 4 second 
        if ( (time.time() - self.lastDelays.lastSync) > 4 ):
            self.delaySync = 0

        if ( (time.time() - self.midiOut.lastMidiTimeDiff.lastSync) > 4 ):
            self.midiTimeSync = 0

        if (self.delaySync and self.midiTimeSync):
            
            if ( not self.sync ):
                log.info( "OUTPUT: server sync" )
                #Both side are sync
                self.sync = 1
                self.lastDelays.flush()
                #uniquement pour test ( le start receiving ne sera autoriser uniquement s il est synchro
                #self.start_receiving()

            #sending synchro packet
            header = self.generateRTPHeader(0,1)
            chunk = 's'
            chunk = header + chunk
            self.transport.write(chunk, (self.peerAddress,self.peerPort) )
                                        
        else:

            if ( self.sync ):
                log.info( "OUTPUT: server not sync" )
                #Both side aren't sync
                self.sync = 0

                self.stop_receiving()
        

    def startProtocol(self):
        """needed by twisted
	"""
        pass


    def datagramReceived(self, data, (host, port)):
        """Receiving datagram
	"""
        
        #si c est la premiere connection ( a changer )
        if ( not self.clientConnect ):
            self.clientConnect = 1
            #on set les address du serveur afin de renvoyer certain packet
            self.peerAddress = host
            self.peerPort = port
            self.launch()
            

	if self.seqNo == 65535:
            self.seqNo = 1   
        else:
            self.seqNo += 1

      	#Parsing packet header
        midiChunk = self.parseRTPHeader(data)
        
	#if header is correct
        if (midiChunk[0] != -1) :
            #si mark bit == 1 => action dans le packet
            
            if (midiChunk[0] == 1):
                
                #if action delay calculation
                if midiChunk[1][0] == 'd':
            	    #setting up delay between the two machine
                    newdelay = float( ((time.time() -  self.lastLocalTime)*1000) / float(2))
                    self.lastDelays.to_list(newdelay)
                    self.midiOut.delay = int(self.lastDelays.average())
                
                    #setting flag to sync
                    if not self.delaySync:
                        self.delaySync = 1
                
                elif midiChunk[1][0] == 'm':
                    
                    #setting the midi time diff between the two machine
                    self.parse_midi_time(midiChunk[1])
                
                    #setting flag to sync
                    if not self.midiTimeSync:
                        self.midiTimeSync = 1
                        
                elif midiChunk[1][0] == '0':
                    #received an empty packet, necessary to check if there is no loose of packet
                    #self.lastPacketTime = time.time()
                    pass
                    

                else:
                    
                    log.warning("OUTPUT: unrecognise action")

            else:
                #no marker => midi note  
                if ( self.midiOut.publy_flag ):
                    #enable witness
                    self.midiOut.start_time = time.time()
                    self.receivingMidiData = 1
                    
                    #unpickle list midi note in the packet               
                    midiNote = cPickle.loads(midiChunk[1])
                    
                    #profiter du parcours pour appliquer les timestamps
                    for i in range(len(midiNote)):
                        midiNote[i][1] = midiNote[i][1] + self.midiOut.midiTimeDiff + self.midiOut.latency - int(self.midiOut.delay) 
                        #Adding the note to the playing buffer
                    	self.midiOut.midiOutCmdList.put(midiNote[i])

                    #disable witness
                    self.receivingMidiData = 0
        else:
            log.warning("OUTPUT: Incorrect type of packet received")
    

    def ask_packet(self,seqNo):
        """send a demand of lost packet
        """
        seqNo -= 1
        log.warning("OUTPUT: Asking lost packet to client")
		
        header = self.generateRTPHeader()
        chunk = str(seqNo)
        chunk = header + chunk
        self.transport.write(chunk , (self.peerAddress,self.peerPort) )


    def parse_midi_time(self,midiTime):
        """Parse midi time (need it ???)
        """
        i = 2
        time = midiTime[i]
        i += 1
        while (i<len(midiTime) and midiTime[i] != ' '):
        	time += midiTime[i]
        	i += 1        
        self.midiOut.sync_midi_time(int(time))


    def send_local_time(self):
        """send time to server to estimate delay (round trip time for a packet)
        """
        self.lastLocalTime = time.time()
        header = self.generateRTPHeader()
        chunk = "dt"
        chunk = header + chunk
        self.transport.write(chunk,(self.peerAddress,self.peerPort))


    def generateRTPHeader(self, timestamp=0, mark=0):
        """ generateRTPHeader needed for each packet sent to the server
        """
        if mark == 0:
            # If sequence number is zero apply Mark bit to RTP header
            mbit = 0x00
        else:
            # No mark bit set for subsequent packets
            mbit = 0x01
        
        hr = pack("!BBHII", mbit, 96, 0, timestamp, self.ssrc)

        return hr
    	

    def parseRTPHeader(self, data):
        """parse RTPHeader
           return -1 if a problem occured
        """
        #header of the rtp header packet 
        if len(data) > 12 :

            hdr = unpack('!BBHII', data[:12])
            
            M = hdr[0]&127
            PT = hdr[1]&127
            seq = hdr[2]
            times = hdr[3]
            ssrc = hdr[4]
            data = data[12:]
                    
            #check s il n y a pa eu de deconnectio
            if ( self.seqNo > seq):
                l = "OUTPUT: Problem in packet order expect=" + str(self.seqNo) + " / receive=" + str(seq)  + " , This problem can be cause by packet reodering process"
                log.error(l)
                self.seqNo = seq
                
            #check if there is no loose of packet last one receive
            if (self.seqNo < seq):
                #on redemande les paquets perdu
                self.seqNo = seq
                self.ask_packet(seq)
                
					
            #If the identifier of the source is wrong
            if (ssrc != 12345679):
                log.error( "OUTPUT: Wrong SSRC identifier : bad source of streaming")
                return -1, 0
            
            return M, data

        else:
            return -1, 0	


    def __del__(self):
        del self.midiOut
        del self.lastDelays
