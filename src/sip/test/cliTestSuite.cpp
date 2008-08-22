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
    "  a    Add media\n"
    "  u    Reinvite\n"
    "  m    Chat\n"
    "  h    Help\n"
    "  q    Quit\n"
    "\n";

int main(int argc, char** argv){
    char input[50];
    std::string msg;
    Session *sip;

    // Session creation
    if( argc == 2 )
        sip = new SIPSession( atoi(argv[1]));
    else
        sip = new SIPSession( );
    sip->addMedia("a=vorbis/PCMA/:12345");

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
                cout << " Add a media: <<<< ";
                cin >> msg;
                sip->addMedia(msg);
                break;
            case 'u':
		sip->updateMedia();
                break;
            case 'm':
                cout << "Enter message <<<< ";
                cin >> msg;
                sip->sendInstantMessage(msg);
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


