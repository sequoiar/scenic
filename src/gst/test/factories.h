#ifndef _FACTORIES_H_
#define _FACTORIES_H_

#include "audioSender.h"
#include "audioReceiver.h"
#include "videoSender.h"
#include "videoReceiver.h"
#include "audioConfig.h"
#include "videoConfig.h"
#include "remoteConfig.h"
#include "capsHelper.h"
#include "ports.h"
#include <memory>   // for std::auto_ptr

namespace Factories {
    std::auto_ptr<AudioSender> buildAudioSender(const AudioConfig aConfig, 
            const char* ip, const char *codec, const long port = Ports::A_PORT);

    std::auto_ptr<AudioReceiver> buildAudioReceiver(const char *ip, const char * codec, const long port = Ports::A_PORT);

    std::auto_ptr<VideoReceiver> buildVideoReceiver(const char *ip, const char * codec, const long port = Ports::V_PORT, int screen_num = 0);
    
    std::auto_ptr<VideoSender> buildVideoSender(const VideoConfig vConfig, 
            const char *ip, const char *codec, const long port = Ports::V_PORT);
}

std::auto_ptr<AudioSender> 
Factories::buildAudioSender(const AudioConfig aConfig, const char* ip, const char *codec, const long port)
{
    SenderConfig rConfig(codec, ip, port);
    std::auto_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));
    tx->init();
    return tx;
}

std::auto_ptr<AudioReceiver> 
Factories::buildAudioReceiver(const char *ip, const char *codec, const long port)
{
    AudioReceiverConfig aConfig("jackaudiosink");
    ReceiverConfig rConfig(codec, ip, port, tcpGetCaps(Ports::CAPS_PORT));
    std::auto_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    rx->init();
    return rx;
}

std::auto_ptr<VideoSender> Factories::buildVideoSender(const VideoConfig vConfig, const char *ip, const char *codec, const long port)
{
    SenderConfig rConfig(codec, ip, port);
    std::auto_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));
    tx->init();
    return tx;
}

std::auto_ptr<VideoReceiver> Factories::buildVideoReceiver(const char *ip, const char *codec, const long port, const int screen_num)
{
    VideoReceiverConfig vConfig("glimagesink",screen_num);
    ReceiverConfig rConfig(codec, ip, port, "");
    std::auto_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
    rx->init();
    return rx;
}

#endif // _FACTORIES_H_

