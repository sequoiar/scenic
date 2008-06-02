
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
                  const std::string media = "1chTest");
        virtual bool start();

    private:
        void init_1ch_test();
        void init_2ch_test();
        void init_6ch_test();
        void init_8ch_test();
        void init_8ch_comp_rtp_test();
        void init_8ch_uncomp_rtp_test();

        std::string remoteHost_;
        int numChannels_;
};

#endif // _AUDIO_SENDER_H_

