from twisted.internet import udp
from twisted.internet import reactor, task
from twisted.internet.protocol import DatagramProtocol
import cPickle
from struct import pack, unpack
import time
from utils import log
from listCirc import DelayCirc
from ringBuffer import myRingBuffer
from midiOut import MidiOut

class RTPServer(DatagramProtocol):

    def __init__(self, streamerAddress="127.0.0.1", streamerPort=44010, permisif=1 ):
        log.info( "OUTPUT: Initializing RTP receiver" )
        
        #Init var
        self.actualSeqNo = 32767
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

        #Launching midi Out writter
        self.midiOut = MidiOut(permisif)
        
        #looping call check sync
        self.releaser = task.LoopingCall(self.check_sync)
        self.sendTime = task.LoopingCall(self.send_local_time)
        self.nbNotes = 0

        #tmp
        self.lastMidiNoteTime = 0

    #self.start_receiving()

    def launch(self):
        self.releaser.start(1)
        self.sendTime.start(0.5)

		
    def start_receiving(self):
        """start midi output and get notes from the socket
        """
        if ( self.sync ):
            if ( not self.midiOut.MidiOut is None):
                self.midiOut.start_publy()
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
        #on arrete la publicatiom des notes
        self.midiOut.stop_publy()
        #on vide les buffers
        self.midiOut.midiOutCmdList.flush()
        self.midiOut.lastMidiTimeDiff.flush()


    def check_sync(self):
        """Checking Synchronisation
        """
        #S il n y  a pas eu de sync du delay depuis 1 second 
        if ( (time.time() - self.lastDelays.lastSync) > 1 ):
            self.delaySync = 0

        if ( (time.time() - self.midiOut.lastMidiTimeDiff.lastSync) > 1 ):
            self.midiTimeSync = 0

        if ( self.delaySync and self.midiTimeSync):
            
            if ( not self.sync ):
                log.info( "OUTPUT: server sync" )
                #Both side are sync
                self.sync = 1
                self.lastDelays.flush()
                #uniquement pour test ( le start receiving ne sera autoriser uniquement s il est synchro
                self.start_receiving()

            #sending synchro packet
            header = self.generateRTPHeader()
            chunk = 's'
            chunk = header + chunk
            self.transport.write(chunk, (self.peerAddress,self.peerPort) )
                                        
        else:

            if ( self.sync ):
                log.info( "OUTPUT: server not sync" )
                #Both side aren't sync
                self.sync = 0

                #reinitialize delay between host
                self.lastDelays.flush()
            
                log.error( "OUTPUT: stopping receiving because syncronisation failed")
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
            self.launch()
            

	if self.actualSeqNo == 32767:
            self.actualSeqNo = 1

            #on set les address du serveur afin de renvoyer certain packet
            self.peerAddress = host
            self.peerPort = port
        else:
            self.actualSeqNo += 1

      	#Parsing packet header
        midiChunk = self.parseRTPHeader(data)

	#if header is correct
        if (midiChunk != -1) :
            if (midiChunk[0] == 'd'):

            	#setting up delay between the two machine
                newdelay = float( ((time.time() -  self.lastLocalTime)*1000) / float(2))
                self.lastDelays.to_list(newdelay)
                self.midiOut.delay = int(self.lastDelays.average())
                
                #setting flag to sync
                if not self.delaySync:
                    self.delaySync = 1
                
            elif(midiChunk[0] == 'm'):

                #setting the midi time diff between the two machine
                self.parse_midi_time(midiChunk)
                
                #setting flag to sync
                if not self.midiTimeSync:
                    self.midiTimeSync = 1

            else:
                #if listening
                if ( not self.midiOut.MidiOut is None):
                    #enable witness
                    
                    self.receivingMidiData = 1
                    self.nbNotes += 1
                    #unpickle list midi note in the packet               
                    midiNote = cPickle.loads(midiChunk)
                    
                    for i in range(len(midiNote)):
                        #Adding the note to the playing buffer
                    	self.midiOut.midiOutCmdList.put(midiNote[i])

                    #disable witness
                    self.receivingMidiData = 0
                    self.midiOut.publy_midi_note()
    

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

    def generateRTPHeader(self):
        """ generateRTPHeader needed for each packet sent to the server
        """
        # If sequence number is zero apply Mark bit to RTP header
        header = '\x80\x8E'   
        header += pack("!h", 0)
        header += pack("!L", 0)
        header += pack("!L", 12345679) # Synchronization Source Identifier
    
        # Technically this is apart of the 
        # RFC2250 3.5 MPEG Audio-specific header and not RTP but include
        # it here
        header += '\x00\x00\x00\x00' 
    
        return header
	

    def parseRTPHeader(self, data):
        """parse RTPHeader
           return -1 if a problem occured
        """
        #header of the rtp header packet
		
        if ( len (data) > 17):
            seq = data[0]
            seq += data[1]
		
            #No of the packet
            no = data[2]
            no += data[3]
            no = unpack("!h", no)
			
            #check s il n y a pa eu de deconnectio
            if ( self.actualSeqNo > no[0]):
                log.warning("OUTPUT: The connection has been drop off then retreive")
                self.actualSeqNo = no[0]
                
            #check if there is no loose of packet last one receive
            if (int(self.actualSeqNo) != int(no[0])):
                #on redemande les paquets perdu
                self.actualSeqNo = no[0]
                self.ask_packet(no[0])
                
			
			
            #The timestamp
            timestamp = data[4]
            timestamp += data[5]
            timestamp += data[6]
            timestamp += data[7]
            timestamp = unpack("!L", timestamp)
			
            #The identifier of the source SSRC identifier
            sync = data[8]
            sync += data[9]
            sync += data[10]
            sync += data[11]
            sync = unpack("!L", sync)
		
            #If the identifier of the source is wrong
            if (sync[0] != 12345679):
                log.error( "OUTPUT: Wrong SSRC identifier : bad source of streaming")
                return -1
			
            #Bite Null of the packet
            endheader = data[12]
            endheader += data[13]
            endheader += data[14]
            endheader += data[15]
			
            #Enfin on recupere les infos midi
            chunk = data[16]
		
            if chunk[0] =='0':
                return -1

            i = 17
            while(i<len(data)):
                chunk += data[i]
                i += 1

            return chunk

        else:
            return -1		

    def __del__(self):
        del self.midiOut
        del self.lastDelays
