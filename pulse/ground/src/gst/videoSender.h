
// videoSender.h
#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include <gst/gst.h>

class VideoSender
{
    public:
        VideoSender(int port);
        ~VideoSender();
        void start();
        void stop();

    private:
        int port_;
        GstElement *pipeline_;
};

#endif

