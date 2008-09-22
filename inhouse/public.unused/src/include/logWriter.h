// logWriter.h
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

/** \file
 *      Log writer macro, usage:
 *
 *      LOG("This is a log");
 *      To enable logging, set the macro LOGGING (in this file) to 1.
 *
 */

#ifndef _LOG_WRITER_H_
#define _LOG_WRITER_H_

#include <iostream>
#include <time.h>

#define LOGGING 1


#if LOGGING
#define LOG(x,e)                                                              \
    {                                                                           \
        time_t rawtime;                                                         \
        struct tm * timeinfo;                                                   \
        time ( &rawtime );                                                      \
        timeinfo = localtime ( &rawtime );                                      \
        std::cerr << std::endl;                                                 \
        std::cerr << __FILE__ << ":" << __LINE__ << ": " << x;                  \
        std::cerr << " " << asctime(timeinfo) << std::endl;                  \
    }
#else
#define LOG(x)
#endif

#define LOG_ERROR(msg)      LOG(msg, ERROR)
#define LOG_CRITICAL(msg)   LOG(msg, CRITICAL)
#define LOG_INFO(msg)       LOG(msg, INFO)
#define LOG_WARNING(msg)    LOG(msg, WARNING)
#define LOG_DEBUG(msg)      LOG(msg, DEBUG)

#endif //  _LOG_WRITER_H_
