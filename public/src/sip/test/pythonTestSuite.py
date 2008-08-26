import sys

# app import
import libsip_export as sip

menu = """\n
         c    Connect
         d    Disconnect
         a    Add a media
         l    List all media
         r    Reinvite
         m    Chat
         i    Get state of the connection
         h    Help
         q    Quit
         \n""";

prompt = "<<<< "


port = int(sys.argv[1])
ua = sip.SIPSession( port )
ua.addMedia("audio", "vorbis/PCMA/", 12345)
print menu

def connect():
    ua.connect("<sip:bloup@192.168.1.230:5060>")

def disconnect():
    ua.disconnect()

def quit():
    ua.disconnect()
    ua.shutdown()
    sys.exit(0)

def start_chat():
    msg = raw_input("Type message: ")
    ua.sendInstantMessage(msg)

# Start the loop
while(True):
    cmd = raw_input(prompt)
    if cmd == 'c':
        connect()
    elif cmd == 'd':
        disconnect()
    elif cmd == 'q':
        quit()
    elif cmd == 'm':
        start_chat()
    else:
        print "Unknown command"
    


#if __name__ == "__main__":

    
