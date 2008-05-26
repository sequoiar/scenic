
// audioBase.h
#ifndef _AUDIO_BASE_H_
#define _AUDIO_BASE_H_

#include <gst/gst.h>

class AudioBase 
{
    public:
        virtual void start();
        void stop();
        bool isPlaying();
        int port() const { return port_; }

    protected:
        AudioBase();
        virtual ~AudioBase();
        int port_;
        static const int DEF_PORT;
        GstElement *pipeline_;

    private:
};

#endif // _AUDIO_BASE_H_
