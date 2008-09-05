import sys

# app import
import libsip_export as libsip

intro = """\n
           Unitary tests.
           Will be tested:
              - library initialisation
              - connection to an UAS in AUTO mode
              - disconnection
              - connection to an UAS in MANUAL mode 
              - list media
              - set media
              - list media
              - reinvite with the new SDP
              - chat test
              - disconnection
              - shutdown
           \n""";

success = " [OK]\n\n "
failure = " [failed]\n\n  "

print intro 

step = " \nLibrary initialisation and user agent instance creation  ....................................................... "
port = int(sys.argv[1])
uac = libsip.SIPSession( port )
uac.setMedia("audio", "vorbis/PCMA/", 12345, "sendrecv")
print step+success

step = " \nDialog invite session creation ................................................................................. "
status = uac.connect("<sip:bloup@192.168.1.230:5060>")
if(status == 0):
    print step+success
else:
    print step+failure

step = " \nConnection to a UAS runnning on the local host on the port 5060 in AUTO answer mode  ..........................  "
while( uac.getConnectionState() != 'CONNECTION_STATE_CONNECTED' ):
    if( uac.getConnectionState() == 'CONNECTION_STATE_TIMEOUT' ):
        print step+failure;
        sys.exit(0)
    pass
print step+success

step = " \nList the enabled media list ....................................................................................  "
list = uac.mediaToString();
print list
print step+success

step = " \nChange the media and list it ...................................................................................  "
uac.setMedia("audio", "PCMA/vorbis/", 12345, "sendrecv")
list = uac.mediaToString()
print list
print step+success

step = " \nReinvite the UAS with the new media description .................................................................."
status = uac.reinvite()
if(status == 0):
    print step+success
else:
    print step+failure

step = " \nChat test. Envoi du message 'SALUT' .............................................................................. "
uac.sendInstantMessage("SALUT")
print step+success

step = " \nDeconnection from the UAS. Send BYE message .....................................................................  "
status = uac.disconnect()
if(status == 0):
    print step+success
else:
    print step+failure
    sys.exit(0)

step = " \nShutdown pjsip, free allocated memory ...........................................................................  "
status = uac.shutdown()
if(status == 0):
    print step+success
else:
    print step+failure
    sys.exit(0)

print " \n Done! Unitary tests successful\n"
