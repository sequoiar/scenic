import cPickle
from utils import log
import time

import pypm
from struct import pack, unpack
from midiObject import packetTime
from listCirc import  PacketCirc
from ringBuffer import myRingBuffer
from twisted.internet.protocol import DatagramProtocol
from twisted.internet import udp, task
class RTPClient(DatagramProtocol):

    def __init__(self, peerAddress="127.0.0.1", port=44000):
    
        log.info("INPUT: Initialize RTP streamer")
        
        #Init var
        self.sock = None
        self.seqNo = 0
        self.peerAddress = peerAddress
        self.port = port
        self.startChrono = []

        #buffer de midi data
        self.midiInCmdList = myRingBuffer()

        #buffer of midi notes sent in order to resend in case of losy packet network 
        self.packetsSentList = PacketCirc(512)
        
        self.releaser = task.LoopingCall(self.continue_streaming)   

        self.lastSync = 0

        #flag
        self.sync = 0
        self.sendingMidiData = 0

        #launching sync checker
        self.checker = task.LoopingCall(self.check_sync)
        self.checker.start(1)


    def check_sync(self):
        """checking if client is sync with the server
        """
        if ( ((time.time() - self.lastSync) > 2) and self.sync ):
            log.info( "INPUT: client not sync" )
            self.sync = 0


    def startProtocol(self):
        """needed by twisted super class
        """
        pass

    
    def datagramReceived(self, data, (host, port)):

        data = self.parseRTPHeader(data)
        if (data != -1):
            if (data[0]== 'd'):
                #resending for delay calcul
            	packet = self.generateRTPHeader(0)
            	chunk = packet + "d "
            	self.transport.write(chunk,(self.peerAddress, self.port))

            elif ( data[0] == 's'):
            	self.lastSync = time.time()
            	#setting flag to sync
            	if ( not self.sync ):
                    log.info( "INPUT: client sync" )
                    self.sync = 1
                    self.start_streaming()

            else:
            	#Sinon cest une demande de paquet perdu
            	retransmitPacket = self.packetsSentList.find_packet(int(data))
            	#si on la dans le buffer de sauvegarde on revoie sinon rien
            	if ( int(retransmitPacket) != -1 ):
                    header = self.generateRTPHeader(0)
                    ch = self.packetsSentList[int(retransmitPacket)].packet
                    chunk = header + ch
                    log.warning("INPUT: Resending lost packet to the server")
                    self.transport.write(chunk,(self.peerAddress, self.port))
        else:
            log.warning("INPUT: Bad packet format")


    def start_streaming(self):

        if ( not self.releaser.running ):
			#check sync to start streaminglog
			if (self.sync):
				self.releaser.start(0.001)
				return 0
			else:
				log.error("INPUT: can't start streaming if not sync with the server")
				return -1			


    def continue_streaming(self):
        """startStreaming launch streaming of midi notes to client
        """

        #si il y a des notes a envoyer on envoie
        if (self.midiInCmdList.avail_for_get() > 0):
            
            #Enable witness
            self.sendingMidiData = 1

            #getting new notes to send
            c = self.midiInCmdList.get()
            #time = c[0].time

            #marshalling
            chunk = cPickle.dumps(c,1)
                
            #Creating header
            header = self.generateRTPHeader(0)

            #saving packet
            pack = packetTime(self.seqNo, chunk)
            self.packetsSentList.to_list(pack)
                
            #Writting it to the socket
            chunk = header + chunk
            
            if ( self.sync ):
                    self.transport.write(chunk,(self.peerAddress, self.port))

            #disable witness
            self.sendingMidiData = 0
    
        else:

            #sinon on envoie un packet vide
            header = self.generateRTPHeader(0)
            chunk = "000"
            pack = packetTime(self.seqNo, chunk)
            self.packetsSentList.to_list(pack)
            chunk = header + chunk
            self.transport.write(chunk,(self.peerAddress, self.port))


    def send_midi_time(self, time):
        """Send midi time to remote peer
        """
        header = self.generateRTPHeader(0)
        chunk = "m " + str(time)
        chunk = header + chunk
        self.transport.write(chunk,(self.peerAddress, self.port))


    def stop_streaming(self):
        """Stop RTP streaming 
        """
        log.info ("INPUT: Stoping streaming midi notes" )
        if self.releaser is not None:
            if self.releaser.running:
                self.releaser.stop()


    def generateRTPHeader(self, timestamp):
        """ generateRTPHeader needed for each packet sent to the server
        """
        # 0E is MP3
        if self.seqNo == 0:
            # If sequence number is zero apply Mark bit to RTP header
            header = '\x80\x8E'
        else:
            # No mark bit set for subsequent packets
            header = '\x80\x0E'

        if self.seqNo == 30000:
            self.seqNo = 1
        else:  
            self.seqNo += 1 # First packet is #1
        
        header += pack("!h", self.seqNo)
        header += pack("!L", timestamp)
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
    
        if ( len (data) >= 16):
    
            #Only checking the identifier of the source SSRC identifier
            sync = data[8]
            sync += data[9]
            sync += data[10]
            sync += data[11]
            sync = unpack("!L", sync)
    
            #If the identifier of the source is wrong
            if (sync[0] != 12345679):
                log.error( "OUTPUT: Wrong SSRC identifier : bad source of streaming")
                return -1
    
            #Enfin on recupere le contenu du paquet
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
		del self.packetsSentList
		del self.midiInCmdLists
