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
    session->setMedia( "audio", "GSM/PCMU/", 12354 );
    session->setMedia( "video", "H264/H263/", 14321  );
    res = session->mediaToString();
    cout << res << endl;

    cout << " Modify the media offer" << endl;
    session->setMedia("audio", "vorbis/PCMA/", 12354, "sendonly" );
    res = session->mediaToString();
    cout << res << endl;
}


void directionStreamTest( void ){
    sdpMedia *media = new sdpMedia( "audio", 12345 );
    media->addCodec("GSM");
    cout << " " << media->getStreamDirectionStr() << endl;

    media->setStreamDirection(1);
    cout << media->getStreamDirectionStr() << endl;

    media->setStreamDirection(2);
    cout << media->getStreamDirectionStr() << endl;

    media->setStreamDirection(3);
    cout << media->getStreamDirectionStr() << endl;

    media->setStreamDirection(4);
    cout << media->getStreamDirectionStr() << endl;
}


void mediaTypeTest( void ){
    sdpMedia *media = new sdpMedia( "audio", 12345, "sendrecv" );
    media->addCodec("GSM");
    cout << media->getMediaTypeStr() << endl;

    media->setMediaType(1);
    cout << media->getMediaTypeStr() << endl;

    media->setMediaType(2);
    cout << media->getMediaTypeStr() << endl;

    media->setMediaType(3);
    cout << media->getMediaTypeStr() << endl;

    media->setMediaType(4);
    cout << media->getMediaTypeStr() << endl;

    media->setMediaType(5);
    cout << media->getMediaTypeStr() << endl;

    media->setMediaType(6);
    cout << media->getMediaTypeStr() << endl;
}


int main(int argc, char** argv ) {
    (void)argc;
    (void)argv;
    codecHandlingAtSessionLevelTest();
    ///directionStreamTest();
    //mediaTypeTest();

    return 0;
}


