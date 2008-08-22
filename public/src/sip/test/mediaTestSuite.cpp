#include <iostream>

#include "../sdpmedia.h"
#include "../sipsession.h"

using std::cout;
using std::endl;

void codecHandlingTest(){

    std::string res;

    cout << " Add GSM and PCMU in a new audio media " << endl;
    sdpMedia *audio_media = new sdpMedia("audio", 12456);
    audio_media->addCodec("GSM");
    audio_media->addCodec("PCMU");

    res = audio_media->toString();
    cout << res << endl;

    cout << " Remove GSM codec from the audio media " << endl;
    audio_media->removeCodec("GSM");
    res = audio_media->toString();
    cout << res << endl;

    cout << " Add GSM codec to the audio media " << endl;
    audio_media->addCodec("GSM");
    res = audio_media->toString();
    cout << res << endl;

    cout << " Remove all the codecs from the audio media and add a new codec" << endl;
    audio_media->clearCodecList();
    audio_media->removeCodec("PCMU");
    audio_media->removeCodec("GSM");
    audio_media->addCodec("vorbis");
    audio_media->addCodec("PCMA");
    res = audio_media->toString();
    cout << res << endl;

    cout << " Add H264 in a new video media " << endl;
    sdpMedia *video_media = new sdpMedia("video", 12334);
    video_media->addCodec("H264");

    res = video_media->toString();
    cout << res << endl;

}

void codecHandlingAtSessionLevelTest( void ){

    std::string res;
    // Use the default SIP port
    SIPSession *session = new SIPSession();
    
    cout << " Add an audio and a video media with 2 codecs each" << endl;
    session->addMedia( "audio" , "GSM/PCMU/" ,12354 );
    session->addMedia( "video", "H264/H263/", 14321 );
    res = session->mediaToString();
    cout << res << endl;

    cout << " Modify the media offer" << endl;
    session->addMedia("audio", "vorbis/PCMA/",12354 );
    res = session->mediaToString();
    cout << res << endl;

}

int main(int argc, char** argv ) {

    codecHandlingAtSessionLevelTest();
    
    return 0;

}
