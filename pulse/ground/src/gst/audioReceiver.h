
// audioReceiver.h
#ifndef _AUDIO_RECEIVER_H_
#define _AUDIO_RECEIVER_H_

#include "mediaBase.h"

class AudioReceiver : public MediaBase 
{
    public:
        AudioReceiver();
        virtual ~AudioReceiver();
        bool init(int port = DEF_PORT);

    private:
        static const char * CAPS_STR;
};

#endif // _AUDIO_RECEIVER_H_

