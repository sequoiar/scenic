/* videoSource.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include "gstLinkable.h"
#include "videoSize.h"
#include "videoSource.h"
#include "pipeline.h"
#include "videoConfig.h"

#include "dv1394.h"
#include "dc1394.h"
#include "v4l2util.h"

#include "fileSource.h"
#include "videoSize.h"

/// Constructor
VideoSource::VideoSource(const VideoSourceConfig &config) : 
    config_(config), source_(0), capsFilter_(0)
{}


/// Callable by child classes
void VideoSource::init()
{
    source_ = Pipeline::Instance()->makeElement(config_.source(), NULL);
}


/// Destructor
VideoSource::~VideoSource()
{
    Pipeline::Instance()->remove(&capsFilter_);
    Pipeline::Instance()->remove(&source_);
}

std::string VideoSource::defaultSrcCaps() const
{
    std::ostringstream capsStr;
    /*capsStr << "video/x-raw-yuv, format=(fourcc)I420, width=" << WIDTH << ", height=" << HEIGHT << ", pixel-aspect-ratio=" 
        << PIX_ASPECT_NUM << "/" << PIX_ASPECT_DENOM; */
    capsStr << "video/x-raw-yuv, width=" << videosize::WIDTH 
        << ", height=" << videosize::HEIGHT << ", framerate="
        << config_.framerate() << "000/1001";
    return capsStr.str();
}

std::string VideoSource::srcCaps() const
{
    return defaultSrcCaps();
}


/// Sets caps on capsfilter
void VideoSource::setCapsFilter(const std::string &capsStr)
{
    tassert(capsFilter_ != 0);
    if (capsStr.empty())
        THROW_ERROR("Can't set capsfilter to empty string");

    if (capsStr == "ANY")   // don't bother setting caps
        THROW_ERROR("Trying to set caps to dummy value");

    GstCaps *videoCaps = gst_caps_from_string(capsStr.c_str());
    LOG_DEBUG("Setting caps to " << gst_caps_to_string(videoCaps));
    g_object_set(G_OBJECT(capsFilter_), "caps", videoCaps, NULL);

    gst_caps_unref(videoCaps);
}


/// Constructor
VideoTestSource::VideoTestSource(const VideoSourceConfig &config) : 
    VideoSource(config)
{}

void VideoTestSource::init()
{
    VideoSource::init();
    g_object_set(G_OBJECT(source_), "is-live", TRUE, NULL); // necessary for clocked callback to work

    capsFilter_ = Pipeline::Instance()->makeElement("capsfilter", NULL);
    gstlinkable::link(source_, capsFilter_);
    setCapsFilter(srcCaps());
}


/// Destructor
VideoTestSource::~VideoTestSource()
{}


/// Constructor
VideoFileSource::VideoFileSource(const VideoSourceConfig &config) : 
    VideoSource(config), identity_(0)
{}

void VideoFileSource::init()
{
    tassert(config_.locationExists());
    identity_ = Pipeline::Instance()->makeElement("identity", NULL);
    g_object_set(identity_, "silent", TRUE, NULL);

    GstElement * queue = FileSource::acquireVideo(config_.location());
    gstlinkable::link(queue, identity_);
}

/// Destructor
VideoFileSource::~VideoFileSource()
{
    Pipeline::Instance()->remove(&identity_);
    FileSource::releaseVideo(config_.location());
}


/// Constructor
VideoDvSource::VideoDvSource(const VideoSourceConfig &config) : 
    VideoSource(config), queue_(0), dvdec_(0)
{}


/// Destructor
VideoDvSource::~VideoDvSource()
{
    Pipeline::Instance()->remove(&queue_);
    Pipeline::Instance()->remove(&dvdec_);
    Dv1394::Instance()->unsetVideoSink();
}


void VideoDvSource::init()
{
    queue_ = Pipeline::Instance()->makeElement("queue", NULL);
    dvdec_ = Pipeline::Instance()->makeElement("dvdec", NULL);

    Dv1394::Instance()->setVideoSink(queue_);
    gstlinkable::link(queue_, dvdec_);
}


void VideoV4lSource::init()
{
    VideoSource::init();
    // set a v4l2src if given to config as an arg, otherwise use default
    if (config_.hasDeviceName() && config_.deviceExists())
        g_object_set(G_OBJECT(source_), "device", config_.deviceName(), NULL);

    if (!v4l2util::checkStandard(expectedStandard_, deviceStr()))
        LOG_WARNING("V4l2 device " << deviceStr() << " is not set to expected standard " << expectedStandard_);

    LOG_DEBUG("v4l width is " << v4l2util::captureWidth(deviceStr()));
    LOG_DEBUG("v4l height is " << v4l2util::captureHeight(deviceStr()));

    capsFilter_ = Pipeline::Instance()->makeElement("capsfilter", NULL);
    gstlinkable::link(source_, capsFilter_);

    setCapsFilter(srcCaps());
}


std::string VideoV4lSource::deviceStr() const
{
    gchar *device_cstr;
    g_object_get(G_OBJECT(source_), "device", &device_cstr, NULL);    // get actual used device

    std::string deviceString(device_cstr);        // stay safe from memory leaks
    g_free(device_cstr);
    return deviceString;
}


std::string VideoV4lSource::srcCaps() const
{
    std::ostringstream capsStr;
    /*capsStr << "video/x-raw-yuv, format=(fourcc)I420, width=" << WIDTH << ", height=" << HEIGHT << ", pixel-aspect-ratio=" 
      << PIX_ASPECT_NUM << "/" << PIX_ASPECT_DENOM; */

    std::string capsSuffix;
    if (v4l2util::isInterlaced(deviceStr()))
        capsSuffix = "000/1001, interlaced=true";
    else
        capsSuffix = "/1";

    capsStr << "video/x-raw-yuv, width=" << v4l2util::captureWidth(deviceStr()) << ", height=" 
        << v4l2util::captureHeight(deviceStr()) 
        << ", framerate=" << config_.framerate() 
        << capsSuffix;

    return capsStr.str();
}


void VideoDc1394Source::init()
{
    VideoSource::init();

    if (config_.hasGUID())
        g_object_set(G_OBJECT(source_), "camera-number", DC1394::GUIDToCameraNumber(config_.GUID()), NULL);
    else if (config_.hasCameraNumber())
        g_object_set(G_OBJECT(source_), "camera-number", config_.cameraNumber(), NULL);
    else
        LOG_DEBUG("No valid camera-number or guid specified, using default camera number 0");


    capsFilter_ = Pipeline::Instance()->makeElement("capsfilter", NULL);
    gstlinkable::link(source_, capsFilter_);

    setCapsFilter(srcCaps());
}



std::string VideoDc1394Source::srcCaps() const
{
    std::ostringstream capsStr;
    capsStr << "video/x-raw-gray, width=" << videosize::WIDTH 
        << ", height=" << videosize::HEIGHT << ", framerate=" 
        << config_.framerate() <<"/1, bpp=8, depth=8";
    return capsStr.str();
}

