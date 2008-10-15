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
#include "hostIP.h"
#include "ports.h"
#include <memory>   // for std::auto_ptr

namespace Factories {
    std::auto_ptr<AudioSender> buildAudioSender(const AudioConfig aConfig, 
            const char* ip, const long port = Ports::A_PORT);
    std::auto_ptr<AudioReceiver> buildAudioReceiver(const char *ip, const long port = Ports::A_PORT);
    std::auto_ptr<VideoReceiver> buildVideoReceiver(const char *ip);
    std::auto_ptr<VideoSender> buildVideoSender(const VideoConfig vConfig, 
            const char *ip);
}

std::auto_ptr<AudioSender> Factories::buildAudioSender(const AudioConfig aConfig, const char* ip, const long port)
{
    SenderConfig rConfig("vorbis", ip, port);
    std::auto_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));
    tx->init();
    return tx;
}

std::auto_ptr<AudioReceiver> Factories::buildAudioReceiver(const char *ip, const long port)
{
    AudioReceiverConfig aConfig("jackaudiosink");
    ReceiverConfig rConfig("vorbis", ip, port, tcpGetCaps(Ports::CAPS_PORT));
    std::auto_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    rx->init();
    return rx;
}

std::auto_ptr<VideoSender> Factories::buildVideoSender(const VideoConfig vConfig, const char *ip)
{
    SenderConfig rConfig("h264", ip, Ports::V_PORT);
    std::auto_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));
    tx->init();
    return tx;
}

std::auto_ptr<VideoReceiver> Factories::buildVideoReceiver(const char *ip)
{
    VideoReceiverConfig vConfig("xvimagesink");
    ReceiverConfig rConfig("h264", ip, Ports::V_PORT, "");
    std::auto_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
    rx->init();
    return rx;
}

#endif // _FACTORIES_H_

