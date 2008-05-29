
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
        void initMonoTest();
        void initStereoTest();
        void initMultiTest();
        bool connect_audio(); 

        std::string remoteHost_;
        int numChannels_;
};

#endif // _AUDIO_SENDER_H_

