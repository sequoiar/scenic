
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

VideoSender::VideoSender(const VideoConfig & config) :
	MediaBase(dynamic_cast <const MediaConfig & >(config)), 
    config_(config), 
    source_(0), demux_(0), queue_(0), dvdec_(0), colorspc_(0), 
    encoder_(0), payloader_(0), sink_(0)
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
	pipeline_.remove(dvdec_);
	pipeline_.remove(queue_);
	pipeline_.remove(demux_);
	pipeline_.remove(source_);
}

void VideoSender::init_source()
{
	source_ = gst_element_factory_make(config_.source(), NULL);
	assert(source_);
	pipeline_.add(source_);
	lastLinked_ = source_;

	if (config_.has_dv()) {     // need to demux and decode dv first
		demux_ = gst_element_factory_make("dvdemux", NULL);
		assert(demux_);
		queue_ = gst_element_factory_make("queue", NULL);
		assert(queue_);
		dvdec_ = gst_element_factory_make("dvdec", NULL);
		assert(dvdec_);

		// demux has dynamic pads
		pipeline_.add(demux_);
		pipeline_.add(queue_);
		pipeline_.add(dvdec_);

		// demux srcpad must be linked to queue sink pad at runtime
		g_signal_connect(demux_, "pad-added", G_CALLBACK(cb_new_src_pad), (void *) queue_);

		assert(gst_element_link(source_, demux_));
		assert(gst_element_link(queue_, dvdec_));
		lastLinked_ = dvdec_;
	}
}

void VideoSender::cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, void *data)
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
	if (config_.has_h264()) {
		colorspc_ = gst_element_factory_make("ffmpegcolorspace", NULL);
		assert(colorspc_);
		pipeline_.add(colorspc_);
		encoder_ = gst_element_factory_make("x264enc", NULL);
		assert(encoder_);
		g_object_set(G_OBJECT(encoder_), "bitrate", 2048, "byte-stream", TRUE, "threads", 4, NULL);
		pipeline_.add(encoder_);
		assert(gst_element_link_many(lastLinked_, colorspc_, encoder_, NULL));
		lastLinked_ = encoder_;
	}
}

void VideoSender::init_sink()
{
	if (config_.isNetworked()) {
		payloader_ = gst_element_factory_make("rtph264pay", NULL);
		assert(payloader_);
		pipeline_.add(payloader_);
		assert(gst_element_link(lastLinked_, payloader_));
		session_.add(payloader_, dynamic_cast<const MediaConfig&>(config_));
	}
	else                        // local test only
	{
		sink_ = gst_element_factory_make("xvimagesink", NULL);
		g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
		pipeline_.add(sink_);
		assert(gst_element_link(lastLinked_, sink_));
	}
}

bool VideoSender::start()
{
	if (config_.isNetworked()) {
		std::cout << "Sending video on port " << config_.port() << " to host " << config_.remoteHost()
		          << std::endl;
	}

	return MediaBase::start();
}

