
// videoReceiver.h
#ifndef _VIDEO_RECEIVER_H_
#define _VIDEO_RECEIVER_H_

#include <gst/gst.h>

class VideoReceiver
{
    public:
        VideoReceiver(int port = DEF_PORT);
        ~VideoReceiver();
        void start();
        void stop();

    private:
        void init();

        int port_;
        static const int DEF_PORT;
        GstElement *pipeline_;
};

#endif

