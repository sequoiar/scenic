#include <stdlib.h>

#include "../sipsession.h"

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

sdpCodec* addCodecToSDPTest( void ) {
    sdpCodec* gsm = new sdpCodec( "audio", "GSM" );
    return gsm;
}


int main( int argc, char** argv ){
    Session* sip = new SIPSession(std::atoi(argv[1]));
    if( argc <= 2 ){
        // Listening mode
        sip->startMainloop();
    }
    else {
        // Make call to the specified sip address
        sip->connect( argv[2], std::atoi(argv[3]) );
    }
    return 1;
}


