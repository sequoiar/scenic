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
    "  m    Say hello to peer\n"
    "  q    Quit\n"
    "\n";

int main(int argc, char** argv){
    char input[50];
    std::string msg;

    // Session creation
    Session *sip = new SIPSession( atoi(argv[1]));
    sip->addMedia("a=vorbis/PCMA/:12345");

    // Console main
    cout << MENU << endl;

    for(;;) {
        cout << "<<<< ";
        cin >> input;
        switch(input[0])
        {
            case 'c':
                sip->connect("<sip:bloup@192.168.1.230:5060>");
                break;
            case 'd':
                sip->disconnect();
                break;
            case 'm':
                cout << "Enter message <<<< ";
                cin >> msg;
                sip->sendInstantMessage(msg);
                break;
            case 'q':
                exit(0);
            default:
                cout << "unknown command" << endl;
                break;
        }
    }
}


