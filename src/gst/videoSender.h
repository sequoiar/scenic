
// videoSender.h
#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include <string>
#include <gst/gst.h>
#include "defaultAddresses.h"

class VideoSender
{
    public:
        VideoSender();
        ~VideoSender();
        bool init(const int port = DEF_PORT, 
                  const std::string addr = THEIR_ADDRESS,
                  const std::string service = "test");
        void start();
        void stop();
        bool isPlaying();
        int port() const { return port_; }

    private:
        void initDv();
        void initTest();

        int port_;
        bool isPlaying_;
        std::string remoteHost_;
        static const int DEF_PORT;
        GstElement *pipeline_;
};

#endif

