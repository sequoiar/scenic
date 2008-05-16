
// videoSender.h
#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include <gst/gst.h>

class VideoSender
{
    public:
        VideoSender();
        ~VideoSender();
        void init(int port = DEF_PORT);
        void start();
        void stop();

    private:
        void initDv();
        void initTest();

        int port_;
        static const int DEF_PORT;
        GstElement *pipeline_;
};

#endif

