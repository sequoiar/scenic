
// videoBase.h
#ifndef _VIDEO_BASE_H_
#define _VIDEO_BASE_H_

#include <gst/gst.h>

class VideoBase
{
    public:
        VideoBase();
        virtual ~VideoBase();
        bool isPlaying() { return isPlaying_; }
        virtual void start();
        void stop();
        int port() const { return port_; }

    protected:
        int port_;
        static const int DEF_PORT;
        GstElement *pipeline_;

    private:
        bool isPlaying_;
};

#endif // _VIDEO_BASE_H_
