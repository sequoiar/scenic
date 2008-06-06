
// mediaBase.cpp
// Copyright 2008 Koya Charles & Tristan Matthews 
//     
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#include <gst/gst.h>

#include "mediaBase.h"

const int MediaBase::DEF_PORT = 10010;
bool MediaBase::gstInitialized_ = false;

MediaBase::MediaBase() : pipeline_(0), verbose_(false)
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
    return true;
}



bool MediaBase::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    return !isPlaying();
}



bool MediaBase::isPlaying() 
{ 
    if (pipeline_ && (GST_STATE(pipeline_) == GST_STATE_PLAYING))
        return true; 
    else
        return false;
}



int MediaBase::port() const
{
    return port_;
}

