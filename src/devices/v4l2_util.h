/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _V4L2UTIL_H_
#define _V4L2UTIL_H_

#include <string>

namespace v4l2util
{
    bool checkStandard(const std::string &expected,
            std::string &actual, const std::string &device);
    void setFormatVideo(const std::string &device, int width, int height);
    unsigned captureWidth(const std::string &device);
    unsigned captureHeight(const std::string &device);
    bool listCameras();
    bool isInterlaced(const std::string &device);
    void setStandard(const std::string &device, const std::string &standard);
    void setInput(const std::string &device, int input);
}

#endif // _V4L2UTIL_H_
