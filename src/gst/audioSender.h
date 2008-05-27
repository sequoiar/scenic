
// audioSender.h
#ifndef _AUDIO_SENDER_H_
#define _AUDIO_SENDER_H_

#include <string>
#include <gst/gst.h>
#include "defaultAddresses.h"

#include "audioBase.h"

class AudioSender : public AudioBase
{
    public:
        AudioSender();
        virtual ~AudioSender(); 
        bool init(const int port = DEF_PORT, 
                  const std::string addr = THEIR_ADDRESS,
                  const std::string media = "monoTest");
        virtual void start();

    private:
        void initMonoTest();
        void initStereoTest();
        void initMultiChannelTest();
        bool connect_audio(); 

        std::string remoteHost_;
        int numChannels_;
};

#endif // _AUDIO_SENDER_H_

