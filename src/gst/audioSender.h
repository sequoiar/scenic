
// audioSender.h
#ifndef _AUDIO_SENDER_H_
#define _AUDIO_SENDER_H_

#include <string>
#include "defaultAddresses.h"

#include "mediaBase.h"

class AudioSender : public MediaBase
{
    public:
        AudioSender();
        virtual ~AudioSender(); 
        bool init(const int port = DEF_PORT, 
                  const std::string addr = THEIR_ADDRESS,
                  const std::string media = "monoTest");
        virtual bool start();

    private:
        void init_mono_test();
        void init_stereo_test();
        void init_6_ch_test();
        void init_multi_test();
        void init_multi_rtp_test();

        std::string remoteHost_;
        int numChannels_;
};

#endif // _AUDIO_SENDER_H_

