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

using std::cout;
using std::endl;
using std::cerr;


void initiateSessionTest( int argc, char** argv ) {
    std::string media;
    int nbMedia;
    //int i;

    /*

       if( argc <= 1 ) {
         cerr << "Wrong usage : ./uasTestSuite [lport] [media1] ... [media2] " << endl;
         return;
       } */
    Session* sip = new SIPSession(atoi(argv[1]));
    nbMedia = argc - 2;
    sip->setMedia("audio", "GSM/PCMU/", 12345);
    /*
       // argv[1] -> listening port
       // argv[2] ... argv[n] -> media to add to the session
       // like this: a=GSM/vorbis/ or v=H264/H263/
       for( i=2 ; i<2+nbMedia ; i++) {
        media = argv[i];
        sip->setMedia( media );
       }
     */
    //sip->startMainloop();
}


void createSDPBodyTest() {
    Session *sip = new SIPSession( 5060 );
    sip->build_sdp();
}


void reconstructURITest() {
    URI *addr = new URI("<sip:hello@192.168.1.230:50060>");
    cout << "test URI " << endl;
    addr->toString();

    cout << addr->getAddress() << endl;
}


void localURITest() {
    URI *local = new URI(5060);
    cout << local->getAddress() << endl;
}


int main( int argc, char** argv ){
    initiateSessionTest( argc, argv );
    return 1;
}


