
#include "videoBase.h"

const int VideoBase::DEF_PORT = 10010;

VideoBase::VideoBase() : isPlaying_(false)
{
    // empty
}



VideoBase::~VideoBase()
{
    stop();
    gst_object_unref(GST_OBJECT(pipeline_));
}



void VideoBase::start()
{
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    isPlaying_ = true;
}



void VideoBase::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    isPlaying_ = false;
}

