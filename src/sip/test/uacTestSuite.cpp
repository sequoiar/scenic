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

SIPSession *sip;


void sendInstantMessageTest( ) {
    int status;

    std::string msg = "Hello World";
    status = sip->sendInstantMessage( msg );

    if( status == 0 )
        printf(" Message sent successfully...\n");
    else
        printf(" Message sending failed...\n");
}


void initiateSessionTest( int argc, char** argv ) {
    std::string media;
    std::string peer;
    int nbMedia;
    //int i;

/*    if( argc <= 2 ) {
        cerr << "Wrong usage : ./uacTestSuite [lport] [uri_to_call] [media1] ... [media2] " <<
        endl;
        return;
    }*/
    sip = new SIPSession(atoi(argv[1]));
    nbMedia = argc - 3;

    // argv[1] -> listening port
    // argv[2] -> SIP uri to call
    // argv[3] ... argv[n] -> media to add to the session
    // like this: a=GSM/vorbis/ or v=H264/H263/
    /*peer = argv[2];
       for( i=3 ; i<3+nbMedia ; i++) {
        media = argv[i];
        sip->addMedia( media );
       }*/
    sip->addMedia("audio", "GSM/PCMU/", 12567);

    cout << "Connecting to peer" << endl;
    //sip->connect( peer );
    sip->connect("<sip:manu@192.168.1.104:5064>");

    cout << "Connection to peer done" << endl;
    //sendInstantMessageTest( );

    //sip->disconnect(  );

    printf("Done\n");
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


