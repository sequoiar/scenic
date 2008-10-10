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
        call.stop()
        return result
        
    d.addCallback(stop)
    d2 = call.start(interval)
    d2.addErrback(d.errback)
    return d



class TestMidiStream(unittest.TestCase):

    def setUp(self):
        self.midiStream = MidiStream('127.0.0.1')
        
    def tearDown(self):        

        #stopping looping call
        if self.midiStream.midiIn.sendTime.running :
            self.midiStream.midiIn.sendTime.stop()
    
        if self.midiStream.midiIn.client.checker.running :
            self.midiStream.midiIn.client.checker.stop()

        if self.midiStream.server.releaser.running :
            self.midiStream.server.releaser.stop()

        if self.midiStream.midiIn.client.continueS.running :
            self.midiStream.midiIn.client.continueS.stop()

        if self.midiStream.server.sendTime.running :
            self.midiStream.server.sendTime.stop()

        #stopping listenning task of reactor
        self.midiStream.midiIn.listen_c.stopListening()
        self.midiStream.listen_s.stopListening()

        del self.midiStream


    def test_set_ip(self):
        res = self.midiStream.set_ip("192.168.0.1")
        assert(res == 0), self.fail("Can't assign a good ip")

        res = self.midiStream.set_ip("256.256.356.232")
        assert(res == -1), self.fail("Can assign a bad formated ip") 

    def test_device_in(self):
        tab = self.midiStream.get_input_devices()
        last_device = tab[len(tab)-1][0]
        
        res = self.midiStream.set_input_device(last_device)
        assert(res == 0), self.fail("Can't set a good input device")

        res = self.midiStream.set_input_device(last_device + 1)
        assert(res == -1), self.fail("Can set a bad input device")


    def test_device_out(self):
        tab = self.midiStream.get_output_devices()
        last_device = tab[len(tab)-1][0]
        
        res = self.midiStream.set_output_device(last_device)
        assert(res == 0), self.fail("Can't set a good output device")

        res = self.midiStream.set_output_device(last_device + 1)
        assert(res == -1), self.fail("Can set a bad output device")


    def test_start_sending(self):
        res = self.midiStream.start_sending()
        assert(res == -1), self.fail("Can start sending without any midi device")

        tab = self.midiStream.get_input_devices()
        last_device = tab[len(tab)-1][0]
        self.midiStream.set_input_device(last_device)
        res = self.midiStream.start_sending()
        assert(res == -1), self.fail("Can start sending without sync with server")

        d = defer.gatherResults([loopUntil(lambda: self.midiStream.midiIn.client.sync == 1)])
        d.addCallback(self.start_sending_2)
        return d
        

    def start_sending_2(self,res):
        res = self.midiStream.start_sending()
        assert(res == 0), self.fail("Can't start sending with sync server and a midi device set")
        self.midiStream.stop_sending()
        

    def test_get_server_sync_witness(self):
        #wait until sync
        d = defer.gatherResults([loopUntil(lambda: self.midiStream.get_server_sync_witness() == 1 )])
        return d


    def test_get_client_sync_witness(self):
        #wait until sync
        d = defer.gatherResults([loopUntil(lambda: self.midiStream.get_client_sync_witness() == 1 )])
        return d
        
     
    
        
    def test_send_note(self):
        
        #setting devices
        tab = self.midiStream.get_input_devices()
        last_device = tab[len(tab)-1][0]
        self.midiStream.set_input_device(last_device)
        
        tab = self.midiStream.get_output_devices()
        last_device = tab[len(tab)-1][0]
        
        self.midiStream.set_output_device(last_device)
        
        #wait until sync
        d = defer.gatherResults([loopUntil(lambda: self.midiStream.midiIn.client.sync == 1)])
        
        #On verifie que le server soit lui aussi syncro
        d.addCallback(lambda _: self.assertEquals(self.midiStream.server.sync,1))
        d.addCallback(self.start)
        
        return d
        
    
    def start(self,result):
        #launching receiving and sending
        self.midiStream.start_receiving()
        self.midiStream.start_sending()

        d = defer.gatherResults([loopUntil(lambda: self.midiStream.server.nbNotes > 0)])

        #putting a note
        note = MidiNote(pypm.Time()+100,128,75,100)
        self.midiStream.midiIn.client.midiInCmdList.put(note)
        self.midiStream.midiIn.client.send_midi_data()
        
        #callback/loopuntil
        d.addCallback(self.stop)
        return d


    def stop(self,result):

        #stopping receiving and sending
        self.midiStream.stop_receiving()
        self.midiStream.stop_sending()
        d = defer.gatherResults([loopUntil(lambda: self.midiStream.server.receivingMidiData == 0)])
    
        return d
        
        
