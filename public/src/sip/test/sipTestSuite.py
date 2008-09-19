import unittest
import time

# app import
import libsip_export as sip
import sip_test

class TestConnection(unittest.TestCase):

    def setUp(self):
        """ Code factorisation
            Creation of the SIP session, user agent initialization
            """
        #self.session = sip_test.Sip(5060)
        self.session = sip.SIPSession(50060)
        #self.pool = self
        #self.session.set_python_instance(self)
        #self.state_ = 'NULL'
        self.assertEqual(self.session.state() , 'CONNECTION_STATE_READY')
        self.assertEqual( self.session.error_reason(), 'NO_ERROR')

    def tearDown(self):
        """ Code factorisation.
            Shutdown the library.
            """
        result = self.session.shutdown()
        self.assertEqual( result, 0 )
        self.assertEqual( self.session.state() , 'CONNECTION_STATE_NULL')
        self.assertEqual( self.session.error_reason(), 'NO_ERROR')

    def connection_callback(self, state):
        #self.state_ = state
        print 'toto'

    def wait(self, len):
        time.sleep(len)

    def check_connection_up(self):
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_CONNECTED') 
        self.assertEqual(self.session.error_reason(), 'NO_ERROR')

    def check_connection_down(self):
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_DISCONNECTED') 
        self.assertEqual(self.session.error_reason(), 'NO_ERROR')

    def test_connect_without_media(self):
        """ Connection test without any media set on the user agent client side. 
            There is no SDP negociation.
            """
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.wait(0.1) 
        self.check_connection_up()

    def test_connect_with_1_compatible_media(self):
        """ Connection test with one media set on the user agent side.
            The selected codec should the only compatible codec
            """
        self.session.set_media("audio", "PCMU/GSM/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.wait(0.1) 
        self.check_connection_up()
        # The sdp negociation have to return the payload of the only compatible codec
        # self.assertEqual( self.session.getFinalCodec() , 3)
     
    def test_connect_with_2_compatible_media(self):
        """ Connection test with two codecs set on the user agent side.
            The selected codec should the first compatible codec in the list
            """
        self.session.set_media("audio", "GSM/vorbis/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.wait(0.1) 
        self.check_connection_up()
        # The sdp negociation have to return the payload of the first compatible codec
        # self.assertEqual( self.session.getFinalCodec() , 3)
      
    def test_connect_with_2_compatible_media_inverse(self):
        """ Connection test with two codecs set on the user agent side.
            The selected codec should the first compatible codec in the list
            """
        self.session.set_media("audio", "vorbis/GSM/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.wait(0.1) 
        self.check_connection_up()
        # The sdp negociation have to return the payload of the first compatible codec
        # self.assertEqual( self.session.getFinalCodec() , 103)
      
    def test_connect_with_3_compatible_media(self):
        """ Connection test with three codecs set on the user agent side.
            The selected codec should the first compatible codec in the list
            """
        self.session.set_media("audio", "PCMA/vorbis/GSM/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.wait(0.1) 
        self.check_connection_up()
        # The sdp negociation have to return the payload of the first compatible codec
        # self.assertEqual( self.session.getFinalCodec() , 103)
      
    def test_connect_without_compatible_media(self):
        """ Connection test without any compatible codec on the user agent side
            The connection has to failed
            """
        self.session.set_media("audio", "speex/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.wait(0.1)
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_NOT_ACCEPTABLE')
        self.assertEqual(self.session.error_reason(), 'ERROR_NO_COMPATIBLE_MEDIA')

    def test_connect_timeout(self):
        """ Test the host unreachable case. The user agent server should be connected on the 
            port 5060. We try to estbalish a connection through the port 50600. Hopefully there
            is no user agent here listening.
            """
        result = self.session.connect("<sip:bloub@localhost:50600>")
        self.assertEqual( result, 0 )
        time.sleep(35)
        self.assertEqual( self.session.state(), 'CONNECTION_STATE_TIMEOUT' )
        self.assertEqual( self.session.error_reason(), 'ERROR_HOST_UNREACHABLE')

    def test_connect_when_already_connected(self):
        """ Dummy test. Try to connect as the connection is already up.
            Nothing should happens
            """
        self.session.connect()
        result = self.session.connect()
        self.assertEqual( result, 1 )
        self.wait(0.1)
        self.check_connection_up()

    def test_reinvite_without_compatible_media(self):
        """ RFC 3261 - Section 14:  If a UA receives a non-2xx final response to a re-INVITE, the session
            parameters MUST remain unchanged, as if no re-INVITE had been issued
            """
        self.session.set_media("audio", "GSM/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.wait(0.1)
        self.check_connection_up()
        #self.assertEqual( self.session.getFinalCodec() , 3)
        # Change the media for a non compatible one
        self.session.set_media("audio", "speex/", 12345, "sendrecv")
        result = self.session.reinvite()
        self.assertEqual( result , 0 )
        self.wait(0.1)
        self.check_connection_up()
        # Media parameters unchanged
        #self.assertEqual( self.session.getFinalCodec() , 3)
     
    def test_reinvite_with_compatible_media(self):
        self.session.set_media("audio", "GSM/", 12345, "sendrecv")
        result = self.session.connect()
        self.assertEqual( result , 0 )
        self.wait(0.1)
        self.check_connection_up()
        #self.assertEqual( self.session.getFinalCodec() , 3)
        # Change the media for an other compatible one
        self.session.set_media("audio", "vorbis/", 12345, "sendrecv")
        result = self.session.reinvite()
        self.assertEqual( result , 0 )
        self.wait(0.1)
        self.check_connection_up()
        # Media parameter changed
        #self.assertEqual( self.session.getFinalCodec() , 103)

    def test_connect_disconnect(self):
        """ Disconnection test. The user agent client send a BYE message to the connected peer
            The connection state should be in DISCONNECTED after the operation
            """
        self.session.connect()
        self.wait(0.1)
        self.check_connection_up()
        result = self.session.disconnect()
        self.assertEqual( result , 0 )
        self.wait(0.1)
        self.check_connection_down()
        
    def test_send_message(self):
        result = self.session.connect("<sip:bloup@localhost:50060>")
        self.assertEqual(result, 0)
        self.wait(0.1)
        self.check_connection_up()
        result= self.session.message("hello")
        self.assertEqual(result, 0)
        self.wait(0.1)
        self.assertEqual( self.session.get_message() , "hello")
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_CONNECTED')

class TestInitialisation(unittest.TestCase):

    def setUp(self):
        """
        Code factorisation
        Creation of the SIP session, user agent initialization
            """
        self.session = sip.SIPSession(50060)
        self.assertEqual(self.session.state() , 'CONNECTION_STATE_READY')
        self.assertEqual( self.session.error_reason(), 'NO_ERROR')

    def test_answerMode(self):
        """ The answer mode defines the behaviour on incoming invite 
            request outside an existing dialog 
            """
        self.assertEqual(self.session.get_answer_mode(), 'auto')
        # Change the answer mode from auto to manual
        self.session.set_answer_mode(1)
        self.assertEqual(self.session.get_answer_mode(), 'manual')

    def test_connectionStateAfterInit(self):
        """ 
        Test the state of the connection right after the initialization
            """
        self.assertEqual(self.session.state(), 'CONNECTION_STATE_READY')

    def test_errorStatusAfterInit(self):
        """ 
        Test the error status right after the initialization
            """
        self.assertEqual(self.session.error_reason(), 'NO_ERROR')

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

    def test_checkConnectionPort(self):
        """ Check if the connection port has been succesfully assigned to our client
            """
        port = self.session.get_connection_port()
        self.assertEqual( port, 50060 )

    def tearDown(self):
        """
        Code factorisation.
        Shutdown the library.
            """
        result = self.session.shutdown()
        self.assertEqual( result, 0 )
        self.assertEqual( self.session.state() , 'CONNECTION_STATE_NULL')
        self.assertEqual( self.session.error_reason(), 'NO_ERROR')


class TestMedia( unittest.TestCase):

    def setUp(self):
        """
        Code factorisation
        Creation of the SIP session, user agent initialization
            """
        self.session = sip.SIPSession(5060)
        self.assertEqual(self.session.state() , 'CONNECTION_STATE_READY')
        self.assertEqual( self.session.error_reason(), 'NO_ERROR')

    def test_audioMedia_1(self):
        """
        Add an audio media with one codec
            """
        result = self.session.set_media("audio", "PCMA/", 12345, "sendrecv")
        result = self.session.media_to_string()
        self.assertEqual(result, 'audio:12345:PCMA/:sendrecv\n\n')

    def test_audioMedia_2(self):
        """
        Add an audio media with one codec, without / char to finish the string
            """
        result = self.session.set_media("audio", "PCMA", 12345, "sendrecv")
        result = self.session.media_to_string()
        self.assertEqual(result, 'audio:12345:PCMA/:sendrecv\n\n')

    def test_videoMedia(self):
        """
        Add a video media with one codec
            """
        result = self.session.set_media("video", "H264/", 12345, "sendrecv")
        result = self.session.media_to_string()
        self.assertEqual(result, 'video:12345:H264/:sendrecv\n\n')

    def test_applicationMedia(self):
        """
        Add an application media
            """
        result = self.session.set_media("application", "test/", 12345, "sendrecv")
        result = self.session.media_to_string()
        self.assertEqual(result, 'application:12345:test/:sendrecv\n\n')

    def test_sendOnlyStream(self):
        """
        Add a send-only video stream.  
            """
        result = self.session.set_media("video", "H264/", 12345, "sendonly")
        result = self.session.media_to_string()
        self.assertEqual(result, 'video:12345:H264/:sendonly\n\n')

    def test_receiveOnlyStream(self):
        """
        Add a receive-only video stream.  
            """
        result = self.session.set_media("video", "H264/", 12345, "recvonly")
        result = self.session.media_to_string()
        self.assertEqual(result, 'video:12345:H264/:recvonly\n\n')

    def test_inactiveStream(self):
        """
        Add an inactive video stream. Doesn't support the data transfer 
            """
        result = self.session.set_media("video", "H264/", 12345, "inactive")
        result = self.session.media_to_string()
        self.assertEqual(result, 'video:12345:H264/:inactive\n\n')

    def test_defaultStreamDirection(self):
        """
        Add a default direction video stream. 
        Should be bidirectional (sendrecv)
            """
        result = self.session.set_media("audio", "PCMA/", 12345)
        result = self.session.media_to_string()
        self.assertEqual(result, 'audio:12345:PCMA/:sendrecv\n\n')

    def tearDown(self):
        """
        Code factorisation.
        Shutdown the library.
            """
        result = self.session.shutdown()
        self.assertEqual( result, 0 )
        self.assertEqual( self.session.state() , 'CONNECTION_STATE_NULL')
        self.assertEqual( self.session.error_reason(), 'NO_ERROR')

