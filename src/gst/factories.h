#ifndef _FACTORIES_H_
#define _FACTORIES_H_

#include "gst/engine.h"
#include <memory>   // for std::auto_ptr
#include "tcp/singleBuffer.h"


namespace ports {
        const long A_PORT = 10000;
        const long V_PORT = 11000;
        const long CAPS_PORT = 12000;
}

namespace factories 
{
    std::auto_ptr<AudioSender> buildAudioSender(const AudioSourceConfig aConfig, 
            const char* ip, const char *codec, const long port = ports::A_PORT);

    std::auto_ptr<AudioReceiver> buildAudioReceiver(const char *ip, const char * codec, const long port = ports::A_PORT);

    std::auto_ptr<VideoReceiver> buildVideoReceiver(const char *ip, const char * codec, const long port = ports::V_PORT, int screen_num = 0);
    
    std::auto_ptr<VideoSender> buildVideoSender(const VideoSourceConfig vConfig, 
            const char *ip, const char *codec, const long port = ports::V_PORT);
}

std::auto_ptr<AudioSender> 
factories::buildAudioSender(const AudioSourceConfig aConfig, const char* ip, const char *codec, const long port)
{
    SenderConfig rConfig(codec, ip, port);
    std::auto_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));
    tx->init();
    return tx;
}

std::auto_ptr<AudioReceiver> 
factories::buildAudioReceiver(const char *ip, const char *codec, const long port)
{
    AudioSinkConfig aConfig("jackaudiosink");
    ReceiverConfig rConfig(codec, ip, port, tcpGetBuffer(ports::CAPS_PORT));
    std::auto_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    rx->init();
    return rx;
}

std::auto_ptr<VideoSender> factories::buildVideoSender(const VideoSourceConfig vConfig, const char *ip, const char *codec, const long port)
{
    SenderConfig rConfig(codec, ip, port);
    std::auto_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));
    tx->init();
    return tx;
}

std::auto_ptr<VideoReceiver> factories::buildVideoReceiver(const char *ip, const char *codec, const long port, const int screen_num)
{
    VideoSinkConfig vConfig("glimagesink",screen_num);
    ReceiverConfig rConfig(codec, ip, port, "");
    std::auto_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
    rx->init();
    return rx;
}

#endif // _FACTORIES_H_

