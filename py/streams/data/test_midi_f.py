from twisted.trial import unittest
from midi import MidiStream
from twisted.internet import reactor, defer, task
from midiObject import MidiNote
import pypm
import time

def loopUntil(predicate, interval=0):
    d = defer.Deferred()
    
    def check():
        res = predicate()
        if res :
            d.callback(res)
    call = task.LoopingCall(check)
    
    def stop(result):
        #print "stop"
        call.stop()
        return result
        
    d.addCallback(stop)
    d2 = call.start(interval)
    d2.addErrback(d.errback)
    #print "loop until" 
    return d



class TestMidiStream(unittest.TestCase):
    def setUp(self):
        
        self.midiStream = MidiStream()
        self.ps = reactor.listenUDP(44000,self.midiStream.server)
        self.pc = reactor.listenUDP(44010,self.midiStream.midiIn.client)

        #starting loopingCall
        self.midiStream.midiIn.sendTime.start(0.5)
        self.midiStream.midiIn.client.checker.start(1)

        
    def tearDown(self):
        
        #stoping looping call
        self.midiStream.midiIn.sendTime.stop()
        self.midiStream.midiIn.client.checker.stop()
        self.midiStream.server.releaser.stop()
        #self.midiStream.server.midiOut.publy.stop()
        self.midiStream.server.sendTime.stop()

        self.ps.stopListening()
        self.pc.stopListening()

    
    def test_sync(self):
        
        #setting devices
        self.midiStream.get_input_devices()
        res = self.midiStream.set_input_device(3)
        
        
        self.midiStream.get_output_devices()
        self.midiStream.set_output_device(4)

        #wait until sync
        d = defer.gatherResults([loopUntil(lambda: self.midiStream.midiIn.client.sync == 1)])
        
        #On verifie que le server soit lui aussi syncro
        d.addCallback(lambda _: self.assertEquals(self.midiStream.server.sync,1))
        d.addCallback(self.start)
        #d.addCallback(self.stop)
        return d
        
    
    def start(self,result):
        
        #launching receving and sending
        self.midiStream.start_receving()
        self.midiStream.start_sending()

        #putting a note
        note = MidiNote(pypm.Time()+100,128,75,100)
        for i in range(100):
            self.midiStream.midiIn.client.midiInCmdList.put(note)
        
        
        #callback/loopuntil
        d = defer.gatherResults([loopUntil(lambda: self.midiStream.server.nbNotes >= 1)])
        d.addCallback(self.stop)
        return d


    def stop(self,result):
        
        #stopping receiving and sending
        self.midiStream.stop_receiving()
        self.midiStream.stop_sending()
        d = defer.gatherResults([loopUntil(lambda: self.midiStream.server.receivingMidiData == 0)])
    
        return d
        
        
