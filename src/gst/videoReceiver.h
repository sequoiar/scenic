
// videoReceiver.h
#ifndef _VIDEO_RECEIVER_H_
#define _VIDEO_RECEIVER_H_

#include "mediaBase.h"

class VideoReceiver : public MediaBase 
{
    public:
        VideoReceiver();
        virtual ~VideoReceiver();
        bool init(int port = DEF_PORT);
};

#endif

