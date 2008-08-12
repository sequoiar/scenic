#include <stdlib.h>
#include <iostream>

#include "../sipsession.h"
#include "../uri.h"

/*
 * Usage to test the communication into the local loop:
 *
 * Launch a `listener` like this: ./sipsessionTester 5060
 * The SIP user agent will listen on the port 5060.
 *
 * Launch an other instance with a SIP uri to connect with like this: ./sipsessionTester 50060 sip:test@127.0.0.1:5060 777777
 * The first port must be different than the one the first instance is listening on, otherwise it will not be able to
 * initialize the transport layer.
 *
 */


void initiateSessionTest( int argc, char** argv ) {
    if( argc == 1 )
        return;

    Session* sip = new SIPSession(std::atoi(argv[1]));
    // A way to add media to the session
    //sip->addMedia( MIME_TYPE_AUDIO, "vorbis/GSM/PCMU/PCMA/" );
    //sip->addMedia( MIME_TYPE_VIDEO, "H263/H264/");
    
    if( argc <= 2 ){
        // Listening mode
        sip->addMedia( MIME_TYPE_AUDIO, "PCMU/GSM/vorbis" );
        sip->addMedia( MIME_TYPE_VIDEO, "H264/H263/");
        sip->startMainloop();
    }
    else {
        // Make call to the specified sip address
        sip->addMedia( MIME_TYPE_AUDIO, "vorbis/PCMA/" );
        sip->addMedia( MIME_TYPE_VIDEO, "H263/");
        sip->connect( argv[2] );
    }
}


void createSDPBodyTest() {
    Session *sip = new SIPSession( 5060 );
    sip->build_sdp();
}

void reconstructURITest() {

    using std::cout;
    using std::endl;

    URI *addr = new URI("<sip:hello@192.168.1.230:50060>");
    cout << "test URI " << endl;
    addr->toString();

    cout << addr->getAddress() << endl;

}

void localURITest() {

    using std::cout;
    using std::endl;

    URI *local = new URI(5060);
    cout << local->getAddress() << endl;

}

int main( int argc, char** argv ){
    initiateSessionTest( argc, argv );
    //createSDPBodyTest();
    //reconstructURITest();
    //localURITest();
    return 1;
}


