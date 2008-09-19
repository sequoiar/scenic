#include <iostream>

#include "../sdpmedia.h"
#include "../sipsession.h"

using std::cout;
using std::endl;

void codecHandlingAtMediaLevelTest(){
    std::string res;

    cout << " Add GSM and PCMU in a new audio media " << endl;
    sdpMedia *audio_media = new sdpMedia("audio", 12456);
    audio_media->add_codec("GSM");
    audio_media->add_codec("PCMU");

    res = audio_media->to_string();
    cout << res << endl;

    cout << " Remove GSM codec from the audio media " << endl;
    audio_media->remove_codec("GSM");
    res = audio_media->to_string();
    cout << res << endl;

    cout << " Add GSM codec to the audio media " << endl;
    audio_media->add_codec("GSM");
    res = audio_media->to_string();
    cout << res << endl;

    cout << " Remove all the codecs from the audio media and add a new codec" << endl;
    audio_media->clear_codec_list();
    audio_media->remove_codec("PCMU");
    audio_media->remove_codec("GSM");
    audio_media->add_codec("vorbis");
    audio_media->add_codec("PCMA");
    res = audio_media->to_string();
    cout << res << endl;

    cout << " Add H264 in a new video media " << endl;
    sdpMedia *video_media = new sdpMedia("video", 12334);
    video_media->add_codec("H264");

    res = video_media->to_string();
    cout << res << endl;
}


void codecHandlingAtSessionLevelTest( void ){
    std::string res;
    // Use the default SIP port
    SIPSession *session = new SIPSession();

    cout << " Add an audio and a video media with 2 codecs each" << endl;
    session->set_local_media( "audio", "GSM/PCMU/", 12354 );
    session->set_local_media( "video", "H264/H263/", 14321  );
    res = session->media_to_string();
    cout << res << endl;

    cout << " Modify the media offer" << endl;
    session->set_local_media("audio", "vorbis/PCMA/", 12354, "sendonly" );
    res = session->media_to_string();
    cout << res << endl;
}


void directionStreamTest( void ){
    sdpMedia *media = new sdpMedia( "audio", 12345 );
    media->add_codec("GSM");
    cout << " " << media->get_stream_direction_str() << endl;

    media->set_stream_direction(1);
    cout << media->get_stream_direction_str() << endl;

    media->set_stream_direction(2);
    cout << media->get_stream_direction_str() << endl;

    media->set_stream_direction(3);
    cout << media->get_stream_direction_str() << endl;

    media->set_stream_direction(4);
    cout << media->get_stream_direction_str() << endl;
}


void mediaTypeTest( void ){
    sdpMedia *media = new sdpMedia( "audio", 12345, "sendrecv" );
    media->add_codec("GSM");
    cout << media->get_media_type_str() << endl;

    media->set_media_type(1);
    cout << media->get_media_type_str() << endl;

    media->set_media_type(2);
    cout << media->get_media_type_str() << endl;

    media->set_media_type(3);
    cout << media->get_media_type_str() << endl;

    media->set_media_type(4);
    cout << media->get_media_type_str() << endl;

    media->set_media_type(5);
    cout << media->get_media_type_str() << endl;

    media->set_media_type(6);
    cout << media->get_media_type_str() << endl;
}


int main(int argc, char** argv ) {
    (void)argc;
    (void)argv;
    codecHandlingAtSessionLevelTest();
    ///directionStreamTest();
    //mediaTypeTest();

    return 0;
}


