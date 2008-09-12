/* GTHREAD-QUEUE-PAIR - Library of GstReceiverThread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews
 *
 * This library is free software; you can redisttribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "gstReceiverThread.h"

#include "gst/audioReceiver.h"
#include "gst/videoReceiver.h"

#define A_PORT 10010
#define V_PORT 10110


GstReceiverThread::~GstReceiverThread()
{
    delete areceiver_;
    delete vreceiver_;
}

bool GstReceiverThread::video_stop(MapMsg& /*msg*/)
{
    if(vreceiver_)
        vreceiver_->stop();
    else
        return false;

    return true;
}

bool GstReceiverThread::audio_stop(MapMsg& /*msg*/)
{
    if(areceiver_)
        areceiver_->stop();
    else
        return false;

    return true;
}


bool GstReceiverThread::video_start(MapMsg& /*msg*/)
{
    delete (vreceiver_);
    vreceiver_ = 0;
    
    VideoConfig config("h264", V_PORT);
    if(!config.sanityCheck())
        return false;
    vreceiver_ = new VideoReceiver(config);
    if(vreceiver_)
    {
        vreceiver_->init();
        vreceiver_->start();
        return true;
    }
    else
        return false;
}


bool GstReceiverThread::audio_start(MapMsg& msg)
{
    delete (areceiver_);
    areceiver_ = 0;
    
    std::string codec_str;
    int port;

    MSG("codec",codec_str);
    MSG("port",port);

    
    AudioConfig config(2, codec_str, port);
    if(!config.sanityCheck())
        return false;
    areceiver_ = new AudioReceiver(config);
    areceiver_->init();
    areceiver_->start();
    return true;
}


