import cPickle
from utils import log
import time

import pypm
import md5, random, os

from struct import pack, unpack
from midiObject import packetTime
from listCirc import  PacketCirc
from ringBuffer import myRingBuffer
from twisted.internet.protocol import DatagramProtocol
from twisted.internet import udp, task, defer, reactor
class RTPClient(DatagramProtocol):

    def __init__(self, peerAddress, port=44000):
    
        log.info("INPUT: Initialize RTP streamer")
        
        #Init var
        self.sock = None
        self.peerAddress = peerAddress
        self.port = port
        
        #header RTP
        self.seqNo = 65535
        self.ssrc = 12345679 
        self.packets = 0

        #buffer de midi data
        self.midiInCmdList = myRingBuffer()

        #buffer of midi notes sent in order to resend in case of losy packet network 
        self.packetsSentList = PacketCirc(512)
        self.lastSync = 0

        #flag
        self.sync = 0
        self.sendingMidiData = 0
        self.end_flag = True

        #launching sync checker
        self.checker = task.LoopingCall(self.check_sync)
        self.checker.start(2)


    def check_sync(self):
        """checking if client is sync with the server
        """
        if (time.time() - self.lastSync) > 4 and self.sync:
            log.info( "INPUT: Client not syncronize with peer" )
            self.sync = 0


    def datagramReceived(self, data, (host, port)):

        data = self.parseRTPHeader(data)
        if (data[0] != -1):
            if (data[1][0]== 'd'):
                
                #resending for delay calcul only once by session
            	chunk = 'd'
                self.send_packet(chunk,1)

            elif ( data[1][0] == 's'):
            	self.lastSync = time.time()
            	#setting flag to sync
            	if ( not self.sync ):
                    log.info( "INPUT: Client syncronize with peer" )
                    self.sync = 1
                    #self.start_streaming()

            else:
            	#Else it's a packet request
            	retransmitPacket = self.packetsSentList.find_packet(int(data[1]))
                
            	#If we had it resending else a little log
            	if ( int(retransmitPacket) != -1 ):
                    #if it's midi data
                    ch = self.packetsSentList[int(retransmitPacket)]
                    if ch.marker == 0:
                        ch = ch.packet
                        [self.midiInCmdList.put(ch[i]) for i in range(len(ch))]
                        log.warning("INPUT: Resending lost packet to the server")
                        
                    #else it's an action packet
                    else:
                        #on ne s occupe que des packet vide, les autres sont renvoyer periodiquement
                        if ch.packet[0] == '0' :
                            log.warning("INPUT: Resending lost packet to the server")
                            self.send_empty_packet()
                        else:
                            l = "INPUT: Don't resending time packet at " + str(time.time()) 
                            log.warning(l)
                    
                else:
                    log.error("INPUT: Can't found packet to resend")
                
        else:
            log.warning("INPUT: Wrong packet format")


    def start_streaming(self):
        #check sync to start streaming
        if self.sync:
            self.end_flag = False
            reactor.callInThread(self.send_midi_data)
            log.info("INPUT: Client start streaming")
            return 0
        else:
            log.error("INPUT: Can't start streaming if not sync with the server")
            return -1			


    def send_midi_data(self):
        d = defer.Deferred()
        
        while not self.end_flag:
            if len(self.midiInCmdList.buffer) > 0 :
                
                #Enable witness
                self.sendingMidiData = 1
        
                #getting new notes to send
                notelist = self.midiInCmdList.get()
                               
                #Writting it to the socket
                if ( self.sync ):
                    reactor.callFromThread(self.send_packet,notelist)
                    
            time.sleep(0.006)
        return d
        

    def send_empty_packet(self):
        
        chunk = "0"
        self.send_packet(chunk,1)

        
    def send_midi_time(self, time):
        """Send midi time to remote peer
        """
        chunk = "m " + str(time)
        self.send_packet(chunk,1)


    def send_packet(self, chunk, m=0):
        #Creating header
        header = self.generateRTPHeader(0, m)

        #saving packet
        pack = packetTime(self.seqNo, chunk, m)
        self.packetsSentList.to_list(pack)
                              
        if m == 0:
            #marshalling notes 
            chunk = cPickle.dumps(chunk,1)

        #Writting it to the socket
        packet = header + chunk   
        
        #if self.seqNo % 10 != 0:
        self.transport.write(packet,(self.peerAddress, self.port))

        if m == 0 :
            self.sendingMidiData = 0

        
    def stop_streaming(self):
        """Stop RTP streaming 
        """
        self.end_flag = True
        log.info ("INPUT: RTPClient stop streaming midi notes" )
        


    def generateRTPHeader(self, timestamp, mark=0):
        """ generateRTPHeader needed for each packet sent to the server
        """
        if mark == 0:
            # If sequence number is zero apply Mark bit to RTP header
            mbit = 0x00
        else:
            # No mark bit set for subsequent packets
            mbit = 0x01

        if self.seqNo == 65535:
            self.seqNo = 1
        else:  
            self.seqNo += 1 # First packet is #1
        
        hr = pack("!BBHII", mbit, 96, self.seqNo, timestamp, self.ssrc)

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
                    					
            #If the identifier of the source is wrong
            if (ssrc != 12345679):
                log.error( "OUTPUT: Wrong SSRC identifier : bad source of streaming")
                return -1, 0
            
            return M, data

        else:
            return -1, 0


    def __del__(self):
        del self.packetsSentList
        del self.midiInCmdLists
