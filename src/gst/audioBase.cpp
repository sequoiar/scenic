
#include "audioBase.h"

const int AudioBase::DEF_PORT = 10020;

AudioBase::AudioBase() : pipeline_(0)
{
    // empty
}



AudioBase::~AudioBase()
{
    stop();
    gst_object_unref(GST_OBJECT(pipeline_));
}



bool AudioBase::start()
{
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    return isPlaying();
}



bool AudioBase::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    return !isPlaying();
}



bool AudioBase::isPlaying() 
{ 
    if (pipeline_ && GST_STATE(pipeline_) == GST_STATE_PLAYING)
        return true; 
    else
        return false;
}

