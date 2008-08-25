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
    "  a    Add a media\n"
    "  l    List all media\n"
    "  r    Reinvite\n"
    "  m    Chat\n"
    "  i    Get state of the connection\n"
    "  h    Help\n"
    "  q    Quit\n"
    "\n";

int main(int argc, char** argv){
    char input[50];
    std::string msg;
    std::string type, codecs, port;
    Session *sip;

    // Session creation
    if( argc == 2 )
        sip = new SIPSession( atoi(argv[1]));
    else
        sip = new SIPSession( );
    sip->addMedia("audio", "vorbis/PCMA/", 12345);

    // Console main
    cout << MENU << endl;

    for(;;) {
        cout << "<<<< ";
        cin >> input;
        switch(input[0])
        {
            case 'c':
                sip->connect("<sip:bloup@192.168.1.230:0>");
                break;
            case 'd':
                sip->disconnect();
                break;
            case 'a':
                cout << " <<< media type : ";
                cin >> type;
                cout << " <<< codecs : ";
                cin >> codecs;
                cout << " <<< port : ";
                cin >> port;
                sip->addMedia( type, codecs, atoi(port.c_str()) );
                break;
            case 'l':
                cout << sip->mediaToString() << endl;
                break;
            case 'r':
                sip->reinvite();
                break;
            case 'm':
                cout << "Enter message <<<< ";
                cin >> msg;
                sip->sendInstantMessage(msg);
                break;
            case 'i':
                msg = sip->getConnectionState();
                cout << msg << endl;
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


