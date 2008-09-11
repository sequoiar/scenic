from twisted.trial import unittest
from twisted.internet import defer
#from twisted.python import failure, log
import pyCallbacks

# app import
import libsip_export as sip

    

class TestConnection(unittest.TestCase):
        
    def connect(self):
        result = self.uas.connect("<sip:bloup@127.0.0.1:5060>")
        self.assertEqual( result, 0 )
        d = defer.Deferred()
        return d
    
    def setUp(self):
        self.uas = sip.SIPSession(5060)
        self.callback_result = 'CONNECTION_STATE_NULL' 

    def _callback(self, *args, **kw):
        state = self.uas.state()
        #self.assertEqual(state, 'CONNECTION_STATE_DISCONNECTED')
        self.callback_result = state

    #def test_connectionDefaultPeer(self, *args, **kw):
        #defer = self.connect()
        #defer.addCallback( self._callback )
        #self.failUnlessEqual(self.callback_result, ('CONNECTION_STATE_CONNECTED'))

    #def test_callbackWithArgs(self):
      #  deferred = defer.Deferred()
        #deferred.addCallback(self._callback, "world")
        #deferred.callback("hello")
        #self.failUnlessEqual(self.callback_result, (('hello', 'world'), {}))

    def tearDown(self):
        result = self.uas.shutdown()
        self.assertEqual( result, 0 )
        #self.port.stopListening()

class TestInitialisation(unittest.TestCase):

    def setUp(self):
        self.session = sip.SIPSession(5060)

    def test_answerMode(self):
        """ The answer mode defines the behaviour on incoming invite 
            request outside an existing dialog 
            """
        self.assertEqual(self.session.getAnswerMode(), 'auto')
        # Change the answer mode from auto to manual
        self.session.setAnswerMode(1)
        self.assertEqual(self.session.getAnswerMode(), 'manual')

    def test_connectionStateAfterInit(self):
        state = self.session.state()
        self.assertEqual(state, 'CONNECTION_STATE_READY')

    def test_errorStatusAfterInit(self):
        error = self.session.errorReason()
        self.assertEqual(error, 'NO_ERROR')

    def test_disconnectIfNotConnected(self):
        result = self.session.disconnect()
        # Should fail because disconnection can't be done if not connected
        self.assertEqual( result, 1 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_READY')

    def test_sendMessageIfNotConnected(self):
        msg = "Hey!"
        result = self.session.message(msg)
        # Should fail because sending a message is allowed only if a connection is established
        self.assertEqual( result, 1 )

    def test_reinviteIfNotConnected(self):
        result = self.session.reinvite()
        # Should fail because reinvite is allowed only inside an existing dialog
        self.assertEqual( result, 1 )

    def tearDown(self):
        result = self.session.shutdown()
        self.assertEqual( result, 0 )


class TestMedia( unittest.TestCase):

    def setUp(self):
        self.session = sip.SIPSession(5060)

    def test_audioMedia(self):
        result = self.session.setMedia("audio", "PCMA/", 12345, "sendrecv")
        result = self.session.mediaToString()
        self.assertEqual(result, 'audio:12345:PCMA/:sendrecv\n\n')

    def test_videoMedia(self):
        result = self.session.setMedia("video", "H264/", 12345, "sendrecv")
        result = self.session.mediaToString()
        self.assertEqual(result, 'video:12345:H264/:sendrecv\n\n')

    def test_applicationMedia(self):
        result = self.session.setMedia("application", "test/", 12345, "sendrecv")
        result = self.session.mediaToString()
        self.assertEqual(result, 'application:12345:test/:sendrecv\n\n')

    def test_sendOnlyStream(self):
        result = self.session.setMedia("video", "H264/", 12345, "sendonly")
        result = self.session.mediaToString()
        self.assertEqual(result, 'video:12345:H264/:sendonly\n\n')

    def test_receiveOnlyStream(self):
        result = self.session.setMedia("video", "H264/", 12345, "recvonly")
        result = self.session.mediaToString()
        self.assertEqual(result, 'video:12345:H264/:recvonly\n\n')

    def test_inactiveStream(self):
        result = self.session.setMedia("video", "H264/", 12345, "inactive")
        result = self.session.mediaToString()
        self.assertEqual(result, 'video:12345:H264/:inactive\n\n')

    def test_defaultStreamDirection(self):
        result = self.session.setMedia("audio", "PCMA/", 12345)
        result = self.session.mediaToString()
        self.assertEqual(result, 'audio:12345:PCMA/:sendrecv\n\n')

    def tearDown(self):
        result = self.session.shutdown()
        self.assertEqual( result, 0 )


