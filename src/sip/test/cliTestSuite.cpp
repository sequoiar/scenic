#include <iostream>
#include "../sipsession.h"

using std::cout;
using std::cin;
using std::endl;

static std::string MENU =
    "\n"
    "Enter menu character:\n"
    "  c    Local connection\n"
    "  d    Disconnect\n"
    "  a    Add a media\n"
    "  l    List all media\n"
    "  r    Reinvite\n"
    "  m    Chat\n"
    "  i    Get state of the connection\n"
    "  e    Get the error status report\n"
    "  x    Change invite answer mode\n"
    "  s    Shutdown library\n"
    "  t    Init library\n"
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
        sip = new SIPSession(1111,  atoi(argv[1]));
    else
        sip = new SIPSession(11111 );
    sip->setMedia("audio", "vorbis/PCMA/", 12345);

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
                if (sip->connect() == 1 ){
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
                cout << " <<< direction : ";
                cin >> dir;
                sip->setMedia( type, codecs, atoi(port.c_str()), dir );
                break;
            case 'l':
                cout << sip->mediaToString() << endl;
                break;
            case 'r':
                if( sip->reinvite() == 1 ){
                    msg = sip->getErrorReason();
                    cout << msg << endl;
                }
                break;
            case 'm':
                cout << "Enter message <<<< ";
                cin >> msg;
                if( sip->sendInstantMessage(msg) == 1 ){
                    msg = sip->getErrorReason();
                    cout << msg << endl;
                }
                break;
            case 'i':
                msg = sip->getConnectionState();
                cout << msg << endl;
                break;
            case 'e':
                msg = sip->getErrorReason();
                cout << msg << endl;
                break;
            case 'x':
                cout << "Actual mode: " << sip->getAnswerMode() << endl;
                cout << "auto (0) or manual (1): ";
                cin >> msg;
                sip->setAnswerMode( atoi(msg.c_str()) );
                cout << "Actual mode: " << sip->getAnswerMode() << endl;
                break;
            case 'h':
                cout << MENU << endl;
                break;
            case 's':
                sip->shutdown();
                break;
            case 't':
                sip->init();
                msg = sip->getErrorReason();
                cout << msg << endl;
                sip->setMedia("audio", "vorbis/PCMA/", 12345);
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


