/* v4l2util.h
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

#ifndef _V4L2UTIL_H_
#define _V4L2UTIL_H_
 	
namespace v4l2util
{
    bool checkStandard(const std::string &expected, const std::string &device);
    void printCaptureFormat(const std::string &device);
    std::string fcc2s(unsigned int val);
    std::string field2s(int val);
    std::string num2s(unsigned num);
    std::string colorspace2s(int val);
}

#endif // _V4L2UTIL_H_

