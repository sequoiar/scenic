
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

#include <gst/gst.h>
#include <iostream>
#include <string>
#include <cassert>

#include "videoSender.h"
#include "videoSource.h"
#include "logWriter.h"

VideoSender::VideoSender(const VideoConfig & config) :
    config_(config), 
    session_(), 
    source_(0), colorspc_(0), 
    encoder_(0), payloader_(0), sink_(0),
    stopped_(false)
{
    // empty
}



VideoSender::~VideoSender()
{
    assert(stop());
    pipeline_.remove(sink_);
    pipeline_.remove(payloader_);
    pipeline_.remove(encoder_);
    pipeline_.remove(colorspc_);
    delete source_;
}



void VideoSender::init_source()
{
    source_ = config_.createSource();
    assert(source_);
    source_->init();
}



void VideoSender::init_codec()
{
    if (config_.has_h264()) {
        colorspc_ = gst_element_factory_make("ffmpegcolorspace", NULL);
        assert(colorspc_);
        pipeline_.add(colorspc_);
        encoder_ = gst_element_factory_make("x264enc", NULL);
        assert(encoder_);
        g_object_set(G_OBJECT(encoder_), "bitrate", 2048, "byte-stream", TRUE, "threads", 4, NULL);
        pipeline_.add(encoder_);
        source_->link_element(colorspc_);
        assert(gst_element_link(colorspc_, encoder_));
    }
}



void VideoSender::init_sink()
{
    if (config_.isNetworked()) {
        payloader_ = gst_element_factory_make("rtph264pay", NULL);
        assert(payloader_);
        pipeline_.add(payloader_);
        assert(gst_element_link(encoder_, payloader_));
        session_.add(payloader_, &config_);
    }
    else                        // local test only, no encoding
    {
        sink_ = gst_element_factory_make("xvimagesink", NULL);
        g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
        pipeline_.add(sink_);
        source_->link_element(sink_);
    }
}



bool VideoSender::start()
{
    MediaBase::start();

    pipeline_.wait_until_playing(); // otherwise it doesn't know it's playing
    wait_for_stop();
    return true;
}



void VideoSender::wait_for_stop()
{
    LOG("Waiting for stop message...");

    lo_server_thread st = lo_server_thread_new("7771", liblo_error);

    lo_server_thread_add_method(st, "/video/tx/stop", "", stop_handler, (void *) this);

    lo_server_thread_start(st);

    while (!stopped_)
        usleep(1000);

    lo_server_thread_free(st);
}

void VideoSender::liblo_error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

int VideoSender::stop_handler(const char *path, const char *types, lo_arg ** argv, int argc,
        void *data, void *user_data)
{
    VideoSender *context = static_cast<VideoSender*>(user_data);
    context->stop();

    LOG("STOPPPINNNNNGGGGGGG");
    context->stopped_ = true;

    return 0;
}

