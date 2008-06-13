
// videoSender.cpp
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


#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <gst/gst.h>

#include "mediaBase.h"
#include "videoSender.h"

VideoSender::VideoSender(const VideoSession& session) : session_(session)
{
    // empty
}



VideoSender::~VideoSender() 
{
    // empty
}



bool VideoSender::init()
{
    GError* error = NULL;
    std::string launchStr = session_.source();

    if (!launchStr.compare("dv1394src")) // need to demux and decode dv
        launchStr += " ! dvdemux name=demux demux. ! queue ! dvdec";

    if (!session_.codec().compare("h264"))
        launchStr += " ! ffmpegcolorspace ! x264enc bitrate=2048 byte-stream=true threads=4";
    
    if (session_.port() != 0)
    {
        launchStr += " ! rtph264pay ! udpsink host=" + session_.remoteHost(); 
        //"x264enc bitrate=10000 byte-stream=true threads=4 ! rtph264pay ! "

        std::stringstream istream;
        istream << " port = " << session_.port();           
        launchStr += istream.str();     // get port number into launch string
        //launchStr += " demux. ! queue ! fakesink";
    }
    else // local test only
        launchStr += " ! xvimagesink sync=false"; 

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);

    make_verbose();

    return check_pipeline();
}

#if 0
bool VideoSender::init(const std::string media,const int port, const std::string addr) 
{
    if (port < 1000)
        port_ = DEF_PORT;
    else
        port_ = port;

    remoteHost_ = std::string(addr);

    //  Create sender pipeline
    //  TODO: should these be subclasses of VideoSender?
    if (!media.compare("dv"))
    {
            initDv();
            return true;
    }
    else if (!media.compare("dvRtp"))
    {
            initDvRtp();
            return true;
    }
    else if (!media.compare("v4l"))
    {
            initV4l();
            return true;
    }
    else if (!media.compare("v4lRtp"))
    {
            initV4lRtp();
            return true;
    }
    else if (!media.compare("test"))
    {
            initTest();
            return true;
    }
    else
    {
        std::cout << "Invalid service type." << std::endl;
        return false;
    }
}



void VideoSender::initDv()
{
    GError* error = NULL;
    std::string launchStr = "dv1394src ! dvdemux ! dvdec ! xvimagesink sync=false";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);

    make_verbose();
}



void VideoSender::initDvRtp()
{
    GError* error = NULL;
    std::string launchStr = "dv1394src ! dvdemux name=demux demux. ! queue ! dvdec ! ffmpegcolorspace !"
                            "x264enc byte-stream=true threads=4 ! rtph264pay ! "
                            "udpsink host="; 
                            //"x264enc bitrate=10000 byte-stream=true threads=4 ! rtph264pay ! "
                                  
    std::stringstream istream;
    istream << remoteHost_ << " port = " << port_;           
    launchStr += istream.str();     // get port number into launch string
    //launchStr += " demux. ! queue ! fakesink";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);

    make_verbose();
}



void VideoSender::initV4l()
{
    GError *error = NULL;
    std::string launchStr = "v4l2src ! ffmpegcolorspace ! xvimagesink";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);
    
    make_verbose();
}



void VideoSender::initV4lRtp()
{
    GError *error = NULL;
    std::string launchStr = "v4l2src ! ffmpegcolorspace ! x264enc bitrate=12000 byte-stream=true" 
                            " threads=4 ! rtph264pay !  udpsink host="; 
    std::stringstream istream;
    istream << remoteHost_ << " port = " << port_;           
    launchStr += istream.str();     // get port number into launch string
    //launchStr += " ! queue ! fakesink";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);

    make_verbose();
}



void VideoSender::initTest()
{
    GstElement *txSrc, *txSink, *txCsp, *x264enc, *rtph264pay;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    make_verbose();

    txSrc = gst_element_factory_make("videotestsrc", "txSrc");
    assert(txSrc);

    txCsp = gst_element_factory_make("ffmpegcolorspace", "txCsp");
    assert(txCsp);

    x264enc = gst_element_factory_make("x264enc", "x264enc");
    assert(x264enc);

    rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    assert(rtph264pay);

    txSink = gst_element_factory_make("udpsink", "txSink");
    assert(txSink);

    g_object_set(G_OBJECT(x264enc),"bitrate", 1500, NULL);
    g_object_set(G_OBJECT(x264enc),"byte-stream", TRUE, NULL);
    g_object_set(G_OBJECT(x264enc),"threads", 4, NULL);
    
    g_object_set(G_OBJECT(txSink), "host", remoteHost_.c_str(), "port", port_, NULL);
    

    gst_bin_add_many(GST_BIN(pipeline_), txSrc, txCsp, x264enc, rtph264pay, 
                     txSink, NULL);
 
    // links testsrc, colorspace converter, encoder, payloader and udpsink
    gst_element_link_many(txSrc, txCsp, x264enc, rtph264pay, txSink, NULL);
}


#endif

bool VideoSender::start()
{
    if (session_.port())
    {
        std::cout << "Sending media on port " << session_.port() << " to host " << session_.remoteHost()
            << std::endl;
    }

    return MediaBase::start();
}
