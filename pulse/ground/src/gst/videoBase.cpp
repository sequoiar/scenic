
#include <gst/gst.h>
#include "videoBase.h"

const int VideoBase::DEF_PORT = 10010;

VideoBase::VideoBase() : pipeline_(0)
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
//    isPlaying_ = true;
}



void VideoBase::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
 //   isPlaying_ = false;
}



bool VideoBase::isPlaying() 
{ 
    if (pipeline_ && GST_STATE(pipeline_) == GST_STATE_PLAYING)
        return true; 
    else
        return false;
}

