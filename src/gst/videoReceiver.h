
// videoReceiver.h
#ifndef _VIDEO_RECEIVER_H_
#define _VIDEO_RECEIVER_H_

#include <gst/gst.h>

class VideoReceiver
{
    public:
        VideoReceiver(int port);
        ~VideoReceiver();
        void start();
        void stop();

    private:
        void init();

        int port_;
        GstElement *pipeline_;
};

#endif

