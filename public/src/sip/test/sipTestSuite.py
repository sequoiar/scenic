from twisted.trial import unittest
from twisted.internet import defer, reactor
from twisted.internet.protocol import Protocol, Factory

# app import
import libsip_export as sip

class TestConnection(unittest.TestCase):

    def setUp(self):
        """
        Code factorisation
        Creation of the SIP session, user agent initialization
            """
        self.session = sip.SIPSession(5060)
        self.assertEqual(self.session.state() , 'CONNECTION_STATE_READY')
        self.assertEqual( self.session.errorReason(), 'NO_ERROR')

    def test_connect_without_media(self):
        """
        Connection test without any media set on the user agent client side. There is no SDP negociation.
            """
        result = self.session.connect()
        self.assertEqual( result , 0 )
        state = self.session.state()
        self.assertEqual(state, 'CONNECTION_STATE_CONNECTED')
        error = self.session.errorReason()
        self.assertEqual(error, 'NO_ERROR')
     
    def test_connect_with_1_media_compatible(self):
        """
        Connection test with one media set on the user agent side.
        The selected codec should the only compatible codec
            """
        self.session.setMedia("audio", "PCMU/GSM/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_CONNECTED')
        self.assertEqual(self.session.errorReason(), 'NO_ERROR')
        # The sdp negociation have to return the payload of the only compatible codec
        self.assertEqual( self.session.getFinalCodec() , 3)
     
    def test_connect_with_2_media_compatible(self):
        """
        Connection test with two codecs set on the user agent side.
        The selected codec should the first compatible codec in the list
            """
        self.session.setMedia("audio", "GSM/vorbis/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_CONNECTED')
        self.assertEqual(self.session.errorReason(), 'NO_ERROR')
        # The sdp negociation have to return the payload of the first compatible codec
        self.assertEqual( self.session.getFinalCodec() , 3)
      
    def test_connect_with_2_media_compatible_inverse(self):
        """
        Connection test with two codecs set on the user agent side.
        The selected codec should the first compatible codec in the list
            """
        self.session.setMedia("audio", "vorbis/GSM/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_CONNECTED')
        self.assertEqual(self.session.errorReason(), 'NO_ERROR')
        # The sdp negociation have to return the payload of the first compatible codec
        self.assertEqual( self.session.getFinalCodec() , 103)
      
    def test_connect_with_3_media_compatible(self):
        """
        Connection test with three codecs set on the user agent side.
        The selected codec should the first compatible codec in the list
            """
        self.session.setMedia("audio", "PCMA/vorbis/GSM/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_CONNECTED')
        self.assertEqual(self.session.errorReason(), 'NO_ERROR')
        # The sdp negociation have to return the payload of the first compatible codec
        self.assertEqual( self.session.getFinalCodec() , 103)
      
    def test_connect_without_compatible_media(self):
        """
        Connection test without any compatible codec on the user agent side
        The connection has to failed
            """
        self.session.setMedia("audio", "speex/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 4 )  # 4 corresponds to the error state: ERROR_NO_COMPATIBLE_MEDIA
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_NOT_ACCEPTABLE')
        self.assertEqual(self.session.errorReason(), 'ERROR_NO_COMPATIBLE_MEDIA')

    def test_reinvite_without_compatible_media(self):
        """
        RFC 3261 - Section 14:  If a UA receives a non-2xx final response to a re-INVITE, the session
        parameters MUST remain unchanged, as if no re-INVITE had been issued
            """
        self.session.setMedia("audio", "GSM/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_CONNECTED')
        self.assertEqual(self.session.errorReason(), 'NO_ERROR')
        #self.assertEqual( self.session.getFinalCodec() , 3)
        # Change the media for a non compatible one
        self.session.setMedia("audio", "speex/", 12345, "sendrecv")
        result = self.session.reinvite()
        self.assertEqual( result , 0 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_CONNECTED')
        self.assertEqual(self.session.errorReason(), 'NO_ERROR')
        # Media parameters unchanged
        #self.assertEqual( self.session.getFinalCodec() , 3)
     
    def test_reinvite_with_compatible_media(self):
        self.session.setMedia("audio", "GSM/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_CONNECTED')
        self.assertEqual(self.session.errorReason(), 'NO_ERROR')
        #self.assertEqual( self.session.getFinalCodec() , 3)
        # Change the media for an other compatible one
        self.session.setMedia("audio", "vorbis/", 12345, "sendrecv")
        result = self.session.reinvite()
        self.assertEqual( result , 0 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_CONNECTED')
        self.assertEqual(self.session.errorReason(), 'NO_ERROR')
        # Media parameter changed
        #self.assertEqual( self.session.getFinalCodec() , 103)
     
    def test_disconnect(self):
        """
        Disconnection test. The user agent client send a BYE message to the connected peer
        The connection state should be in DISCONNECTED after the operation
            """
        self.session.connect()
        result = self.session.disconnect()
        self.assertEqual( result , 0 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_DISCONNECTED')
        self.assertEqual(self.session.errorReason(), 'NO_ERROR')
        
    def tearDown(self):
        """
        Code factorisation.
        Shutdown the library.
            """
        result = self.session.shutdown()
        self.assertEqual( result, 0 )
        self.assertEqual( self.session.state() , 'CONNECTION_STATE_NULL')
        self.assertEqual( self.session.errorReason(), 'NO_ERROR')

class TestInitialisation(unittest.TestCase):

    def setUp(self):
        """
        Code factorisation
        Creation of the SIP session, user agent initialization
            """
        self.session = sip.SIPSession(5060)
        self.assertEqual(self.session.state() , 'CONNECTION_STATE_READY')
        self.assertEqual( self.session.errorReason(), 'NO_ERROR')

    def test_answerMode(self):
        """ The answer mode defines the behaviour on incoming invite 
            request outside an existing dialog 
            """
        self.assertEqual(self.session.getAnswerMode(), 'auto')
        # Change the answer mode from auto to manual
        self.session.setAnswerMode(1)
        self.assertEqual(self.session.getAnswerMode(), 'manual')

    def test_connectionStateAfterInit(self):
        """ 
        Test the state of the connection right after the initialization
            """
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_READY')

    def test_errorStatusAfterInit(self):
        """ 
        Test the error status right after the initialization
            """
        self.assertEqual(self.session.errorReason(), 'NO_ERROR')

    def test_disconnectIfNotConnected(self):
        """
        Try a disconnection outside a connection.
        Should fail
            """
        result = self.session.disconnect()
        # Should fail because disconnection can't be done if not connected
        self.assertEqual( result, 1 )
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_READY')

    def test_sendMessageIfNotConnected(self):
        """
        Try to send an instant text message outside a connection.
        Should fail
            """
        msg = "Hey!"
        result = self.session.message(msg)
        # Should fail because sending a message is allowed only if a connection is established
        self.assertEqual( result, 1 )

    def test_reinviteIfNotConnected(self):
        """
        Try to send a re-invite outside a connection.
        Should fail
            """
        result = self.session.reinvite()
        # Should fail because reinvite is allowed only inside an existing dialog
        self.assertEqual( result, 1 )

    def tearDown(self):
        """
        Code factorisation.
        Shutdown the library.
            """
        result = self.session.shutdown()
        self.assertEqual( result, 0 )
        self.assertEqual( self.session.state() , 'CONNECTION_STATE_NULL')
        self.assertEqual( self.session.errorReason(), 'NO_ERROR')


class TestMedia( unittest.TestCase):

    def setUp(self):
        """
        Code factorisation
        Creation of the SIP session, user agent initialization
            """
        self.session = sip.SIPSession(5060)
        self.assertEqual(self.session.state() , 'CONNECTION_STATE_READY')
        self.assertEqual( self.session.errorReason(), 'NO_ERROR')

    def test_audioMedia_1(self):
        """
        Add an audio media with one codec
            """
        result = self.session.setMedia("audio", "PCMA/", 12345, "sendrecv")
        result = self.session.mediaToString()
        self.assertEqual(result, 'audio:12345:PCMA/:sendrecv\n\n')

    def test_audioMedia_2(self):
        """
        Add an audio media with one codec, without / char to finish the string
            """
        result = self.session.setMedia("audio", "PCMA", 12345, "sendrecv")
        result = self.session.mediaToString()
        self.assertEqual(result, 'audio:12345:PCMA/:sendrecv\n\n')

    def test_videoMedia(self):
        """
        Add a video media with one codec
            """
        result = self.session.setMedia("video", "H264/", 12345, "sendrecv")
        result = self.session.mediaToString()
        self.assertEqual(result, 'video:12345:H264/:sendrecv\n\n')

    def test_applicationMedia(self):
        """
        Add an application media
            """
        result = self.session.setMedia("application", "test/", 12345, "sendrecv")
        result = self.session.mediaToString()
        self.assertEqual(result, 'application:12345:test/:sendrecv\n\n')

    def test_sendOnlyStream(self):
        """
        Add a send-only video stream.  
            """
        result = self.session.setMedia("video", "H264/", 12345, "sendonly")
        result = self.session.mediaToString()
        self.assertEqual(result, 'video:12345:H264/:sendonly\n\n')

    def test_receiveOnlyStream(self):
        """
        Add a receive-only video stream.  
            """
        result = self.session.setMedia("video", "H264/", 12345, "recvonly")
        result = self.session.mediaToString()
        self.assertEqual(result, 'video:12345:H264/:recvonly\n\n')

    def test_inactiveStream(self):
        """
        Add an inactive video stream. Doesn't support the data transfer 
            """
        result = self.session.setMedia("video", "H264/", 12345, "inactive")
        result = self.session.mediaToString()
        self.assertEqual(result, 'video:12345:H264/:inactive\n\n')

    def test_defaultStreamDirection(self):
        """
        Add a default direction video stream. 
        Should be bidirectional (sendrecv)
            """
        result = self.session.setMedia("audio", "PCMA/", 12345)
        result = self.session.mediaToString()
        self.assertEqual(result, 'audio:12345:PCMA/:sendrecv\n\n')

    def tearDown(self):
        """
        Code factorisation.
        Shutdown the library.
            """
        result = self.session.shutdown()
        self.assertEqual( result, 0 )
        self.assertEqual( self.session.state() , 'CONNECTION_STATE_NULL')
        self.assertEqual( self.session.errorReason(), 'NO_ERROR')

