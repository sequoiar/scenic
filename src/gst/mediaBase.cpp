
#include <gst/gst.h>

#include "mediaBase.h"

const int MediaBase::DEF_PORT = 10010;
bool MediaBase::gstInitialized_ = false;

MediaBase::MediaBase() : pipeline_(0)
{
    if (!gstInitialized_)
    {
        gstInitialized_ = true;
        // should only be called once in a process
        gst_init(0, NULL);
    }
}



MediaBase::~MediaBase()
{
    stop();
    gst_object_unref(GST_OBJECT(pipeline_));
}



bool MediaBase::start()
{
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    //return isPlaying();
    return true;
}



bool MediaBase::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    return !isPlaying();
}



bool MediaBase::isPlaying() 
{ 
    if (pipeline_ && GST_STATE(pipeline_) == GST_STATE_PLAYING)
        return true; 
    else
        return false;
}

