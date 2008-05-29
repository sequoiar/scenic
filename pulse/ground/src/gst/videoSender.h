
// videoSender.h
#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include <string>
#include "defaultAddresses.h"

#include "mediaBase.h"

class VideoSender : public MediaBase
{
    public:
        VideoSender();
        virtual ~VideoSender(); 
        bool init(const int port = DEF_PORT, 
                  const std::string addr = THEIR_ADDRESS,
                  const std::string media = "test");
        virtual bool start();

    private:
        void initDv();
        void initTest();

        std::string remoteHost_;
};

#endif

