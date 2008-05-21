
// videoSender.h
#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include <string>
#include <gst/gst.h>
#include "defaultAddresses.h"

#include "videoBase.h"

class VideoSender : public VideoBase
{
    public:
        VideoSender();
        virtual ~VideoSender(); 
        bool init(const int port = DEF_PORT, 
                  const std::string addr = THEIR_ADDRESS,
                  const std::string service = "test");
        virtual void start();

    private:
        void initDv();
        void initTest();

        std::string remoteHost_;
};

#endif

