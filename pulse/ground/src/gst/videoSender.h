
// videoSender.h
#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include <gst/gst.h>

class VideoSender
{
    public:
        VideoSender(int port = DEF_PORT);
        ~VideoSender();
        void start();
        void stop();

    private:
        void init();
        void initDv();
        void initTest();

        int port_;
        static const int DEF_PORT;
        GstElement *pipeline_;
};

#endif

