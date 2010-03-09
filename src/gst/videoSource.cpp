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

#include "videoSource.h"
#include <sstream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

#include "util.h"

#include "gstLinkable.h"
#include "pipeline.h"
#include "videoConfig.h"

#include "dv1394.h"
#include "dc1394.h"
#include "v4l2util.h"

#include "fileSource.h"

/// Constructor
VideoSource::VideoSource(const Pipeline &pipeline, const VideoSourceConfig &config)
    :
        pipeline_(pipeline),
        config_(config),
        source_(0),
        capsFilter_(0)
{}


/// Destructor
VideoSource::~VideoSource()
{
    pipeline_.remove(&capsFilter_);
    pipeline_.remove(&source_);
}

std::string VideoSource::defaultSrcCaps() const
{
    std::ostringstream capsStr;
    capsStr << "video/x-raw-yuv, width=" << config_.captureWidth()
        << ", height=" << config_.captureHeight() << ", framerate="
        << config_.framerate() << "000/1001, pixel-aspect-ratio="
        << config_.pixelAspectRatio();
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
    GstCaps *videoCaps = gst_caps_from_string(capsStr.c_str());
    LOG_DEBUG("Setting caps to " << gst_caps_to_string(videoCaps));
    g_object_set(G_OBJECT(capsFilter_), "caps", videoCaps, NULL);

    gst_caps_unref(videoCaps);
}


/// Constructor
VideoTestSource::VideoTestSource(const Pipeline &pipeline,
        const VideoSourceConfig &config)
    :
        VideoSource(pipeline, config)
{
    source_ = pipeline_.makeElement(config_.source(), NULL);
    g_object_set(G_OBJECT(source_), "is-live", TRUE, NULL); // necessary for clocked callback to work

    capsFilter_ = pipeline_.makeElement("capsfilter", NULL);
    gstlinkable::link(source_, capsFilter_);
    setCapsFilter(srcCaps());
}

/// Destructor
VideoTestSource::~VideoTestSource()
{}


/// Constructor
VideoFileSource::VideoFileSource(const Pipeline &pipeline, const VideoSourceConfig &config) 
    :
        VideoSource(pipeline, config),
        identity_(pipeline_.makeElement("identity", NULL))
{
    tassert(config_.locationExists());
    g_object_set(identity_, "silent", TRUE, NULL);

    GstElement * queue = FileSource::acquireVideo(pipeline, config_.location());
    gstlinkable::link(queue, identity_);
}

/// Destructor
VideoFileSource::~VideoFileSource()
{
    pipeline_.remove(&identity_);
    FileSource::releaseVideo(config_.location());
}


/// Constructor
VideoDvSource::VideoDvSource(const Pipeline &pipeline, 
        const VideoSourceConfig &config) :
    VideoSource(pipeline, config),
    queue_(pipeline_.makeElement("queue", NULL)),
    dvdec_(pipeline_.makeElement("dvdec", NULL))
{
    Dv1394::Instance(pipeline_)->setVideoSink(queue_);
    gstlinkable::link(queue_, dvdec_);
}


/// Destructor
VideoDvSource::~VideoDvSource()
{
    pipeline_.remove(&queue_);
    pipeline_.remove(&dvdec_);
    Dv1394::Instance(pipeline_)->unsetVideoSink();
}


bool VideoV4lSource::willModifyCaptureResolution() const
{
    return v4l2util::captureWidth(deviceStr()) != config_.captureWidth() or
        v4l2util::captureHeight(deviceStr()) != config_.captureHeight();
}


VideoV4lSource::VideoV4lSource(const Pipeline &pipeline, 
        const VideoSourceConfig &config)
: VideoSource(pipeline, config), expectedStandard_("NTSC"), actualStandard_("")
{
    source_ = pipeline_.makeElement(config_.source(), NULL);
    // set a v4l2src if given to config as an arg, otherwise use default
    if (config_.hasDeviceName() && config_.deviceExists())
        g_object_set(G_OBJECT(source_), "device", config_.deviceName(), NULL);

    if (!v4l2util::checkStandard(expectedStandard_, actualStandard_, deviceStr()))
        LOG_WARNING("V4l2 device " << deviceStr() << " is not set to expected standard "
                << expectedStandard_ << ", it is " << actualStandard_);

    LOG_DEBUG("v4l width is " << v4l2util::captureWidth(deviceStr()));
    LOG_DEBUG("v4l height is " << v4l2util::captureHeight(deviceStr()));

    if (willModifyCaptureResolution())
    {
        LOG_INFO("Changing v4l resolution to " <<
                config_.captureWidth() << "x" << config_.captureHeight());
        v4l2util::setFormatVideo(deviceStr(), config_.captureWidth(), config_.captureHeight());
    }

    capsFilter_ = pipeline_.makeElement("capsfilter", NULL);
    gstlinkable::link(source_, capsFilter_);

    setCapsFilter(srcCaps());
}


std::string VideoV4lSource::deviceStr() const
{
    gchar *device_cstr;
    g_object_get(G_OBJECT(source_), "device", &device_cstr, NULL);    // get actual used device

    std::string deviceString(device_cstr); // stay safe from memory leaks
    g_free(device_cstr);
    return deviceString;
}


std::string VideoV4lSource::srcCaps() const
{
    std::ostringstream capsStr;

    std::string capsSuffix;
    if (actualStandard_ == "NTSC")
        capsSuffix = "30000/1001"; // NTSC is drop frame
    else if (actualStandard_ == "PAL")
        capsSuffix = "25/1"; // PAL is not drop frame
    else
    {
        capsSuffix = boost::lexical_cast<std::string>(config_.framerate());
        capsSuffix += "/1";
        LOG_WARNING("Unsupported standard, " << actualStandard_
                << "trying to add framerate " << capsSuffix << " to caps");
    }

    if (v4l2util::isInterlaced(deviceStr()))
        capsSuffix +=", interlaced=true";

    capsSuffix += ", pixel-aspect-ratio=";
    capsSuffix += config_.pixelAspectRatio();

    capsStr << "video/x-raw-yuv, width=" << config_.captureWidth() << ", height="
        << config_.captureHeight()
        << ", framerate="
        << capsSuffix;
    LOG_DEBUG("V4l2src caps are " << capsStr.str());

    return capsStr.str();
}


VideoDc1394Source::VideoDc1394Source(const Pipeline &pipeline, const VideoSourceConfig &config) :
    VideoSource(pipeline, config)
{
    if (not Dc1394::areCamerasConnected())
        THROW_CRITICAL("No dc1394 camera connected");

    source_ = pipeline_.makeElement(config_.source(), NULL);
    if (config_.hasGUID())
        g_object_set(G_OBJECT(source_), "camera-number", Dc1394::GUIDToCameraNumber(config_.GUID()), NULL);
    else if (config_.hasCameraNumber())
        g_object_set(G_OBJECT(source_), "camera-number", config_.cameraNumber(), NULL);
    else
        LOG_DEBUG("No valid camera-number or guid specified, using default camera number 0");
    /// TODO: test. this will hopefully help reduce the lag we're seeing with dc1394src
    enum {DMA_BUFFER_SIZE_IN_FRAMES = 2};
    g_object_set(G_OBJECT(source_), "buffer-size", DMA_BUFFER_SIZE_IN_FRAMES, NULL);

    capsFilter_ = pipeline_.makeElement("capsfilter", NULL);
    gstlinkable::link(source_, capsFilter_);

    setCapsFilter(srcCaps());
}


std::string VideoDc1394Source::srcCaps() const
{
    typedef std::vector<std::string> ColourspaceList;
    std::ostringstream capsStr;
    int cameraNumber;
    int mode = 0;
    g_object_get(source_, "camera-number", &cameraNumber, NULL);

    std::string colourSpace;
    ColourspaceList spaces;
    // if we support other colourspaces besides grayscale
    if (not config_.forceGrayscale())
    {
        /// favour rgb because we need to be have that colourspace for shared video buffer
        spaces.push_back("rgb");
        spaces.push_back("yuv");
    } 
    spaces.push_back("gray");

    for (ColourspaceList::iterator space = spaces.begin(); mode == 0 and space != spaces.end(); ++space)
    {
        colourSpace = *space;
        mode = Dc1394::capsToMode(cameraNumber, config_.captureWidth(),
                config_.captureHeight(), colourSpace, config_.framerate());
    }

    // vmode takes into account resolution, bpp, depth
    if (mode != 0)
        capsStr << "video/x-raw-" << colourSpace << ", vmode=" << mode << ",framerate="<< config_.framerate() << "/1";
    else
        THROW_CRITICAL("Could not find appropriate video mode for colourspace "
                << colourSpace  << " and resolution "
                << config_.captureWidth() << "x" << config_.captureHeight());

    if (Dc1394::requiresMoreISOSpeed(mode))
    {
        // FIXME: should set to b-mode too
        LOG_DEBUG("Setting iso speed to 800");
        g_object_set(source_, "iso-speed", Dc1394::MAX_ISO_SPEED, NULL);
    }
    return capsStr.str();
}

