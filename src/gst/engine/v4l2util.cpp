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

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>  // for O_RDWR
#include <cerrno>
#include <iostream>
#include <map>
#include <cassert>

#include "util.h"
#include "v4l2util.h"

static int doioctl(int fd, int request, void *parm, const std::string &name)
{
    int retVal;

    retVal = ioctl(fd, request, parm);
    if (retVal < 0)
        THROW_ERROR("IOCTL " << name << " failed: ");// << strerror(errno) << std::endl);

    return retVal;
}

/// Check current standard of v4l2 device to make sure it is what we expect
bool v4l2util::checkStandard(const std::string &expected, const std::string &device)
{
    bool result = false;
    v4l2_std_id std;
    int fd = -1;

    // map of format coes
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
                result = (result || (expected == (*iter).first)); // can have multiple positives, hence the or
    }

    return result;
}

