
// mediaBase.h
#ifndef _MEDIA_BASE_H_
#define _MEDIA_BASE_H_

typedef struct _GstElement GstElement;

class MediaBase
{
    public:
        virtual bool start();
        virtual bool stop();
        bool isPlaying();
        int port() const { return port_; }

    protected:
        MediaBase();
        virtual ~MediaBase();
        int port_;
        static const int DEF_PORT;
        GstElement *pipeline_;

    private:
        static bool gstInitialized_;
};

#endif // _MEDIA_BASE_H_
