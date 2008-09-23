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
#include "hostIP.h"

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


bool GstReceiverThread::video_start(MapMsg& msg)
{
    delete (vreceiver_);
    vreceiver_ = 0;


    try
    {
        //Get the parameter variables or return false
        GET(msg, "codec", std::string, codec_str);
        GET(msg, "port", int, port);
        VideoReceiverConfig config("xvimagesink");
        ReceiverConfig rConfig(codec_str, get_host_ip(), port);
        if(!config.sanityCheck())
            return false;
        vreceiver_ = new VideoReceiver(config, rConfig);
        vreceiver_->init();
        vreceiver_->start();
        return true;
    }
    catch(ErrorExcept e)
    {
        delete(vreceiver_);
        vreceiver_ = 0;
        return false;
    }
}


bool GstReceiverThread::audio_start(MapMsg& msg)
{
    delete (areceiver_);
    areceiver_ = 0;

    try
    {
        //Get the parameter variables or return false
        GET(msg, "codec", std::string, codec_str);
        GET(msg, "port", int, port);
        GET(msg, "channels", int, chan);
        AudioReceiverConfig config("jackaudiosink");
        ReceiverConfig rConfig(codec_str, get_host_ip(), port);
        if(!config.sanityCheck())
            return false;
        areceiver_ = new AudioReceiver(config, rConfig);
        areceiver_->init();
        areceiver_->start();
        return true;
    }
    catch(ErrorExcept e)
    {
        delete(areceiver_);
        areceiver_ = 0;
        return false;
    }
}


