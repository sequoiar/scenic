#include "../sipsession.h"

int main( int argc, char** argv ){
	Session* sip = new SIPSession();
	sip->connect( 5060, "<sip:136@asterix.savoirfairelinux.net>");
	return 1;
}
