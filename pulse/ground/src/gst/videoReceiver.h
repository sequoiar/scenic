
// videoReceiver.h
#ifndef _VIDEO_RECEIVER_H_
#define _VIDEO_RECEIVER_H_

#include <gst/gst.h>

class VideoReceiver
{
    public:
        VideoReceiver();
        ~VideoReceiver();
        bool init(int port = DEF_PORT);
        void start();
        void stop();
        bool isPlaying();
        int port() const { return port_; }

    private:
        int port_;
        bool isPlaying_;
        static const int DEF_PORT;
        GstElement *pipeline_;
};

#endif

