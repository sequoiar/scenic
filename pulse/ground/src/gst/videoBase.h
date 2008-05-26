
// videoBase.h
#ifndef _VIDEO_BASE_H_
#define _VIDEO_BASE_H_

#include <gst/gst.h>

class VideoBase
{
    public:
        virtual void start();
        void stop();
        bool isPlaying();
        int port() const { return port_; }

    protected:
        VideoBase();
        virtual ~VideoBase();
        int port_;
        static const int DEF_PORT;
        GstElement *pipeline_;

    private:
        // FIXME: This info should come directly from pipeline
        //bool isPlaying_;
};

#endif // _VIDEO_BASE_H_
