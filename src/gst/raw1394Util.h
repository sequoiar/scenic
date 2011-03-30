
// raw1394util.h
// Based on raw1394util.h, part of dvgrab available at <http://www.kinodv.org/>
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
// This file is part of Scenic.
//
// Scenic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Scenic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _RAW1394UTIL_H_
#define _RAW1394UTIL_H_ 

#include "config.h"
#include <string>
#include <vector>

#ifdef CONFIG_IEEE1394
#include <libraw1394/raw1394.h>
#endif

class Raw1394 
{
public:
#ifdef CONFIG_IEEE1394
    static bool listCameras();
    static bool cameraIsReady();
#else
    static bool listCameras() { return false; }
    static bool cameraIsReady() { return false; }
#endif
private:
    static std::vector<std::string> getDeviceList();
};

#endif // RAW1394UTIL_H 
