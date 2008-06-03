
// audioReceiver.h
#ifndef _AUDIO_RECEIVER_H_
#define _AUDIO_RECEIVER_H_

#include <string>

#include "mediaBase.h"

class AudioReceiver : public MediaBase 
{
    public:
        AudioReceiver();
        virtual ~AudioReceiver();
        bool init(int port = DEF_PORT);

    private:
        static const std::string CAPS_STR;
};

#endif // _AUDIO_RECEIVER_H_

