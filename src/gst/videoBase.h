
// videoBase.h
#ifndef _VIDEO_BASE_H_
#define _VIDEO_BASE_H_

typedef struct _GstElement GstElement;

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
};

#endif // _VIDEO_BASE_H_
