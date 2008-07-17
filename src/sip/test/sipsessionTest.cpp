#include "../sipsession.h"

int main( int argc, char** argv ){
	Session* sip = new SIPSession();
	sip->session_connect( 5060, "192.168.1.204");
	return 1;
}
