
// videoReceiver.h
#ifndef _VIDEO_RECEIVER_H_
#define _VIDEO_RECEIVER_H_

#include <gst/gst.h>

#include "videoBase.h"

class VideoReceiver : public VideoBase
{
    public:
        VideoReceiver();
        virtual ~VideoReceiver();
        bool init(int port = DEF_PORT);
};

#endif

