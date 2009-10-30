// v4l2util.cpp
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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

// see v4l2-ctl.cpp for reference

#include "config.h"

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>  // for O_RDWR
#include <cerrno>
#include <map>
#include <cstdio>
#include <vector>
#include <string>

// for filesystem ops
#ifdef HAVE_BOOST_FILESYSTEM
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#endif

#include "util.h"
#include "v4l2util.h"

static int doioctl(int fd, long request, void *parm, const std::string &name)
{
    int retVal;

    retVal = ioctl(fd, request, parm);
    if (retVal < 0)
        LOG_WARNING("IOCTL " << name << " failed: ");// << strerror(errno) << std::endl);

    return retVal;
}

static v4l2_format captureFormat(const std::string &device)
{
    v4l2_format vfmt;
    vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int fd = -1;

    if ((fd = open(device.c_str(), O_RDWR)) < 0) 
        THROW_ERROR("Failed to open " << device);

    if (!(doioctl(fd, VIDIOC_G_FMT, &vfmt, "VIDIOC_G_FMT") == 0))
        LOG_WARNING("IOctl VIDIOC_G_FMT failed");

    close(fd);
    return vfmt;
}

/// Check current standard of v4l2 device to make sure it is what we expect
bool v4l2util::checkStandard(const std::string &expected, const std::string &device)
{
    bool result = false;
    v4l2_std_id std;
    int fd = -1;

    // map of format codes
    std::map<std::string, unsigned long long> FORMATS;
    FORMATS["PAL"] = 0xfff;
    FORMATS["NTSC"] = 0xf000;
    FORMATS["SECAM"] = 0xff0000;
    FORMATS["ATSC/HDTV"] =  0xf000000;
    
    if ((fd = open(device.c_str(), O_RDWR)) < 0) 
        THROW_ERROR("Failed to open " << device << ": ");// << strerror(errno));

    if (doioctl(fd, VIDIOC_G_STD, &std, "VIDIOC_G_STD") == 0) 
    {
        std::map<std::string, unsigned long long>::const_iterator iter;
        for (iter = FORMATS.begin(); iter != FORMATS.end(); ++iter)
            if (std & (*iter).second)    // true if current format matches this iter's key
                result = (result or (expected == (*iter).first)); // can have multiple positives, hence the or
    }

    close(fd);
    return result;
}


std::string v4l2util::field2s(int val)
{
        switch (val) {
        case V4L2_FIELD_ANY:
                return "Any";
        case V4L2_FIELD_NONE:
                return "None";
        case V4L2_FIELD_TOP:
                return "Top";
        case V4L2_FIELD_BOTTOM:
                return "Bottom";
        case V4L2_FIELD_INTERLACED:
                return "Interlaced";
        case V4L2_FIELD_SEQ_TB:
                return "Sequential Top-Bottom";
        case V4L2_FIELD_SEQ_BT:
                return "Sequential Bottom-Top";
        case V4L2_FIELD_ALTERNATE:
                return "Alternating";
        case V4L2_FIELD_INTERLACED_TB:
                return "Interlaced Top-Bottom";
        case V4L2_FIELD_INTERLACED_BT:
                return "Interlaced Bottom-Top";
        default:
                return "Unknown (" + num2s(val) + ")";
        }
}

std::string v4l2util::num2s(unsigned num)
{
    char buf[10];

    sprintf(buf, "%08x", num);
    return buf;
}

std::string v4l2util::fcc2s(unsigned int val)
{
    std::string s;

    s += val & 0xff;
    s += (val >> 8) & 0xff;
    s += (val >> 16) & 0xff;
    s += (val >> 24) & 0xff;
    return s;
}

void v4l2util::printCaptureFormat(const std::string &device)
{
    v4l2_format vfmt = captureFormat(device);

    LOG_PRINT("\nVideo4Linux Camera " << device << ":" << std::endl);
    LOG_PRINT("\tWidth/Height  : " << vfmt.fmt.pix.width << "/" << vfmt.fmt.pix.height << "\n");
    LOG_PRINT("\tPixel Format  : " << fcc2s(vfmt.fmt.pix.pixelformat) << "\n");
    LOG_PRINT("\tField         : " << field2s(vfmt.fmt.pix.field) << "\n");
    LOG_PRINT("\tBytes per Line: " << vfmt.fmt.pix.bytesperline << "\n");
    LOG_PRINT("\tSize Image    : " << vfmt.fmt.pix.sizeimage << "\n");
    LOG_PRINT("\tColorspace    : " << colorspace2s(vfmt.fmt.pix.colorspace) << "\n");
}


unsigned v4l2util::captureWidth(const std::string &device)
{
    v4l2_format vfmt = captureFormat(device);
    return vfmt.fmt.pix.width;
}


unsigned v4l2util::captureHeight(const std::string &device)
{
    v4l2_format vfmt = captureFormat(device);
    return vfmt.fmt.pix.height;
}

std::string v4l2util::colorspace2s(int val)
{
    switch (val) {
        case V4L2_COLORSPACE_SMPTE170M:
            return "Broadcast NTSC/PAL (SMPTE170M/ITU601)";
        case V4L2_COLORSPACE_SMPTE240M:
            return "1125-Line (US) HDTV (SMPTE240M)";
        case V4L2_COLORSPACE_REC709:
            return "HDTV and modern devices (ITU709)";
        case V4L2_COLORSPACE_BT878:
            return "Broken Bt878";
        case V4L2_COLORSPACE_470_SYSTEM_M:
            return "NTSC/M (ITU470/ITU601)";
        case V4L2_COLORSPACE_470_SYSTEM_BG:
            return "PAL/SECAM BG (ITU470/ITU601)";
        case V4L2_COLORSPACE_JPEG:
            return "JPEG (JFIF/ITU601)";
        case V4L2_COLORSPACE_SRGB:
            return "SRGB";
        default:
            return "Unknown (" + num2s(val) + ")";
    }
}

typedef std::vector<std::string> DeviceList;
#ifdef HAVE_BOOST_FILESYSTEM
DeviceList getDevices()
{
    namespace fs = boost::filesystem;

    fs::path full_path("/dev/");


    if ( !fs::exists(full_path) )
        THROW_CRITICAL("\nPath " << full_path << " not found");

    DeviceList deviceList;

    fs::directory_iterator end_iter;
    for (fs::directory_iterator dir_itr(full_path);
            dir_itr != end_iter;
            ++dir_itr )
    {
        try
        {
            std::string pathString(dir_itr->path().string());
            if (pathString.find("video") != std::string::npos)  // devices matching video
                if (pathString.find("1394") == std::string::npos)   // that don't contain 1394
                   deviceList.push_back(pathString);
        }
        catch ( const std::exception & ex )
        {
            THROW_CRITICAL(dir_itr->path()<< " " << ex.what());
        }
    }
    return deviceList;
}
#endif



void v4l2util::listCameras()
{
#ifdef HAVE_BOOST_FILESYSTEM
    DeviceList names(getDevices());
#else
    LOG_WARNING("Boost filesystem not installed, just guessing what are devices are");
    DeviceList names;
    names.push_back("/dev/video0");
    names.push_back("/dev/video1");
#endif

    for (DeviceList::const_iterator deviceName = names.begin(); deviceName != names.end(); ++deviceName)
        if (fileExists(*deviceName))
            printCaptureFormat(*deviceName);
}

bool v4l2util::isInterlaced(const std::string &device)
{
    if (fileExists(device))
    {
        v4l2_format vfmt = captureFormat(device);
        if (vfmt.fmt.pix.field == V4L2_FIELD_INTERLACED)
            return true;
        else
            return false;
    }
    else
    {
        LOG_ERROR("No device " << device);
        return false;
    }
}
