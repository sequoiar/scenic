#include <iostream>
#include "../sipsession.h"

using std::cout;
using std::cin;
using std::endl;

static std::string MENU =
    "\n"
    "Enter menu character:\n"
    "  c    Connect\n"
    "  d    Disconnect\n"
    "  a    Add a media ( sendrecv )\n"
    "  b    Add a media (with direction)\n"
    "  l    List all media\n"
    "  r    Reinvite\n"
    "  m    Chat\n"
    "  i    Get state of the connection\n"
    "  x    Enable auto invite answer mode\n"
    "  y    Disable auto invite answer mode\n"
    "  h    Help\n"
    "  q    Quit\n"
    "\n";

int main(int argc, char** argv){
    char input[50];
    std::string msg;
    std::string type, codecs, port, dir;
    Session *sip;

    // Session creation
    if( argc == 2 )
        sip = new SIPSession( atoi(argv[1]));
    else
        sip = new SIPSession( );
    sip->setMedia("audio", "vorbis/PCMA/", 12345, "sendrecv");

    // Console main
    cout << MENU << endl;

    for(;;) {
        if( sip->incomingInvite() ){
            cout << "Incoming call: ";
            cout << "accept(a) or refuse (r) ";
            cin >> input;

            switch(input[0])
            {
                case 'a':
                    sip->accept();
                    break;
                case 'r':
                    sip->refuse();
                    break;
                default:
                    sip->accept();
                    break;

            }
        }
        cout << "<<<< ";
        cin >> input;
        switch(input[0])
        {
            case 'c':
                if (sip->connect("<sip:bloup@192.168.1.230:0>") == 1 ){
                    msg = sip->getConnectionState();
                    cout << msg << endl;
                }
                break;
            case 'd':
                if( sip->disconnect() == 1 ){
                    msg = sip->getConnectionState();
                    cout << msg << endl;
                }
                break;
            case 'a':
                cout << " <<< media type : ";
                cin >> type;
                cout << " <<< codecs : ";
                cin >> codecs;
                cout << " <<< port : ";
                cin >> port;
                sip->setMedia( type, codecs, atoi(port.c_str()), "sendrecv" );
                break;
            case 'b':
                cout << " <<< media type : ";
                cin >> type;
                cout << " <<< codecs : ";
                cin >> codecs;
                cout << " <<< port : ";
                cin >> port;
                cout << " <<< direction : ";
                cin >> dir;
                sip->setMedia( type, codecs, atoi(port.c_str()), dir );
                break;
            case 'l':
                cout << sip->mediaToString() << endl;
                break;
            case 'r':
                if( sip->reinvite() == 1 ){
                    msg = sip->getConnectionState();
                    cout << msg << endl;
                }
                break;
            case 'm':
                cout << "Enter message <<<< ";
                cin >> msg;
                if( sip->sendInstantMessage(msg) == 1 ){
                    msg = sip->getConnectionState();
                    cout << msg << endl;
                }
                break;
            case 'i':
                msg = sip->getConnectionState();
                cout << msg << endl;
                break;
            case 'x':
                sip->setAutoAnswer( true );
                break;
            case 'y':
                sip->setAutoAnswer( false );
                break;
            case 'h':
                cout << MENU << endl;
                break;
            case 'q':
                sip->shutdown();
                exit(0);
            default:
                cout << "unknown command" << endl;
                break;
        }
    }
}


