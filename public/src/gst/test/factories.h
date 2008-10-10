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

static std::auto_ptr<AudioSender> buildAudioSender(const AudioConfig aConfig)
{
    SenderConfig rConfig("vorbis", get_host_ip(), Ports::A_PORT);
    std::auto_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));
    tx->init();
    return tx;
}


static std::auto_ptr<AudioReceiver> buildAudioReceiver()
{
    AudioReceiverConfig aConfig("jackaudiosink");
    ReceiverConfig rConfig("vorbis", get_host_ip(), Ports::A_PORT, tcpGetCaps(Ports::CAPS_PORT));
    std::auto_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    rx->init();
    return rx;
}


static std::auto_ptr<VideoReceiver> buildVideoReceiver()
{
    VideoReceiverConfig vConfig("xvimagesink");
    ReceiverConfig rConfig("h264", get_host_ip(), Ports::V_PORT, "");
    std::auto_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
    rx->init();
    return rx;
}


static std::auto_ptr<VideoSender> buildVideoSender(const VideoConfig vConfig)
{
    SenderConfig rConfig("h264", get_host_ip(), Ports::V_PORT);
    std::auto_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));
    tx->init();
    return tx;
}

#endif // _FACTORIES_H_

