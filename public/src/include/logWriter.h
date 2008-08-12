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
#include <sstream>

#define ENABLE_LOG 1

#if ENABLE_LOG
#define LOG(msg, level)                                                              \
    log(msg, level, __FILE__, __FUNCTION__, __LINE__);
#else
#define LOG(msg, level)
#endif

enum LogLevel {
    DEBUG = 10,
    INFO = 20,
    WARNING = 30,
    ERROR = 40,
    CRITICAL = 50
};

static const LogLevel LOG_LEVEL = DEBUG;

static bool logLevelIsValid(LogLevel level)
{
    switch (level)
    {
        case DEBUG:
        case INFO:
        case WARNING:
        case ERROR:
        case CRITICAL:
            return true;
            break;
        default:
            return false;
            break;
    }
}


static bool logLevelMatch(LogLevel level)
{
    if (level >= LOG_LEVEL && logLevelIsValid(level))
        return true;
    else
        return false;
}


static void log(const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName,
                const int lineNum)
{
    if (logLevelMatch(level))
    {
        time_t rawtime;
        struct tm * timeinfo;

        time( &rawtime );
        timeinfo = localtime(&rawtime);
        std::ostringstream logMsg;
        logMsg << std::endl << fileName << ":" << functionName << ":" << lineNum << ": " <<
        msg << " "
               << asctime(timeinfo) << std::endl;

        // FIXME: send message to Core
        std::cerr << logMsg.str();
    }
}


#endif //  _LOG_WRITER_H_
