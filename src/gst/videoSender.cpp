
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

#include "videoSender.h"
#include "logWriter.h"

VideoSender::VideoSender(const VideoConfig& config) : config_(config)
{
    // empty
}



VideoSender::~VideoSender() 
{
    // empty
}


// FIXME: make this more like AudioSender::init()
bool VideoSender::init_old()
{
    GError* error = NULL;
    std::string launchStr = config_.source();

    if (!launchStr.compare("dv1394src")) // need to demux and decode dv first
        launchStr += " ! dvdemux name=demux demux. ! queue ! dvdec";

    if (!std::string("h264").compare(config_.codec()))
        launchStr += " ! ffmpegcolorspace ! x264enc bitrate=2048 byte-stream=true threads=4";
    
    if (config_.isNetworked())
    {
        launchStr += " ! rtph264pay ! udpsink host="; 
        launchStr += config_.remoteHost(); 

        std::stringstream istream;
        istream << " port = " << config_.port();           
        launchStr += istream.str();     // get port number into launch string
    }
    else // local test only
        launchStr += " ! xvimagesink sync=false"; 

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);

    make_verbose();

    // FIXME: this method should actually check the pipeline, currently does nothing
    return true;
}



// FIXME: make this more like AudioSender::init()
bool VideoSender::init()
{
    init_pipeline();

    init_source();

    init_codec();
    
    init_sink();
    
    // FIXME: this method should actually check the pipeline, currently does nothing
    return true;
}



void VideoSender::init_source()
{
    source_ = gst_element_factory_make(config_.source(), NULL);
    assert(source_);
    gst_bin_add(GST_BIN(pipeline_), source_);
    lastLinked_ = source_;

    // FIXME this is ugly
    if (!std::string("dv1394src").compare(config_.source())) // need to demux and decode dv first
    {
        demux_ = gst_element_factory_make("dvdemux", NULL);
        assert(demux_);
        queue_ = gst_element_factory_make("queue", NULL);
        assert(queue_);
        dvdec_ = gst_element_factory_make("dvdec", NULL);
        assert(dvdec_);

        // demux has dynamic pads
        gst_bin_add_many(GST_BIN(pipeline_), demux_, queue_, dvdec_, NULL);

        // demux srcpad must be linked to queue sink pad at runtime
        g_signal_connect(demux_, "pad-added", G_CALLBACK(cb_new_src_pad), (void *) queue_);

        assert(gst_element_link(source_, demux_));
        assert(gst_element_link(queue_, dvdec_));
        //launchStr += " ! dvdemux name=demux demux. ! queue ! dvdec";
        lastLinked_ = dvdec_;
    }
}



void VideoSender::cb_new_src_pad(GstElement *srcElement, GstPad *srcPad, void * data)
{
    // ignore audio from dvsrc
    if (!std::string("audio").compare(gst_pad_get_name(srcPad)))
        return;

    GstElement *sinkElement = (GstElement *) data;
    GstPad *sinkPad;
    LOG("Dynamic pad created, linking new srcpad and sinkpad.");
    
    sinkPad = gst_element_get_static_pad(sinkElement, "sink");
    assert(gst_pad_link(srcPad, sinkPad) == GST_PAD_LINK_OK);
    gst_object_unref(sinkPad);
}


void VideoSender::init_codec()
{
    if (!std::string("h264").compare(config_.codec()))
    {
    //    launchStr += " ! ffmpegcolorspace ! x264enc bitrate=2048 byte-stream=true threads=4";
        colorspc_ = gst_element_factory_make("ffmpegcolorspace", NULL);
        assert(colorspc_);
        encoder_ = gst_element_factory_make("x264enc", NULL);
        assert(encoder_);
        g_object_set(G_OBJECT(encoder_), "bitrate", 2048, "byte-stream", TRUE, "threads", 4, NULL);
        gst_bin_add_many(GST_BIN(pipeline_), colorspc_, encoder_, NULL);
        assert(gst_element_link_many(lastLinked_, colorspc_, encoder_, NULL));
        lastLinked_ = encoder_;
    }
}



void VideoSender::init_sink()
{
    if (config_.isNetworked())
    {
        payloader_ = gst_element_factory_make("rtph264pay", NULL);
        assert(payloader_);
        sink_ = gst_element_factory_make("udpsink", NULL);
        g_object_set(G_OBJECT(sink_), "host", config_.remoteHost(), "port", config_.port(), NULL);
        gst_bin_add_many(GST_BIN(pipeline_), payloader_, sink_, NULL);
        assert(gst_element_link_many(lastLinked_, payloader_, sink_, NULL));
    }
    else // local test only
    {
        sink_ = gst_element_factory_make("xvimagesink", NULL);
        g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
        gst_bin_add(GST_BIN(pipeline_), sink_);
        assert(gst_element_link(lastLinked_, sink_));
    }
}


bool VideoSender::start()
{
    if (config_.isNetworked())
    {
        std::cout << "Sending video on port " << config_.port() << " to host " << config_.remoteHost()
            << std::endl;
    }

    return MediaBase::start();
}

