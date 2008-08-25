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
#include <stdlib.h>
#include "config.h"

#define ENABLE_LOG 1
#define ENABLE_GLOG 0


#if !ENABLE_LOG
#define LOG(msg, level)
#else
    #if ENABLE_GLOG
#include <glib.h>
        #define LOG(msg, level)         \
            glog(log_(msg, level, __FILE__, __FUNCTION__, __LINE__),level);
        #else
        #define LOG(msg, level)         \
            std::cerr << log_(msg, level, __FILE__, __FUNCTION__, __LINE__);if(level == CRITICAL) abort(); 
    #endif

enum LogLevel {
    DEBUG = 10,
    INFO = 20,
    WARNING = 30,
    ERROR = 40,
    CRITICAL = 50
};

#define LOG_ERROR(msg)      LOG(msg, ERROR);
#define LOG_CRITICAL(msg)   LOG(msg, CRITICAL);
#define LOG_INFO(msg)       LOG(msg, INFO);
#define LOG_WARNING(msg)    LOG(msg, WARNING);
#define LOG_DEBUG(msg)      LOG(msg, DEBUG);

#ifdef CONFIG_DEBUG
static const LogLevel LOG_LEVEL = DEBUG;
#else
static const LogLevel LOG_LEVEL = INFO;
#endif

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


static const std::string logLevelStr(LogLevel level)
{
    switch (level)
    {
        case DEBUG:
            return "DEBUG: ";
        case INFO:
            return "INFO: ";
        case WARNING:
            return "WARNING: ";
        case ERROR:
            return "ERROR: ";
        case CRITICAL:
            return "CRITICAL: ";
        default:
            return "INVALID LOG LEVEL: ";
    }
}



static bool logLevelMatch(LogLevel level)
{
    if (level >= LOG_LEVEL && logLevelIsValid(level))
        return true;
    else
        return false;
}


static const std::string log_(const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, const int lineNum)
{
    std::ostringstream logMsg;
    if (logLevelMatch(level))
    {
        time_t rawtime;
        struct tm * timeinfo;

        time( &rawtime );
        timeinfo = localtime(&rawtime);
        logMsg << std::endl << logLevelStr(level) << fileName << ":" << functionName << ":" << lineNum << ": " <<
            msg << " " << asctime(timeinfo) << std::endl;

        // FIXME: send message to Core
    }

    return logMsg.str();
}

#if ENABLE_GLOG
static void glog(const std::string& str, LogLevel level)
{
    if(str.empty())
        return;

    switch(level)
    {
        case DEBUG: 
            g_debug(str.c_str()); break;

        case WARNING: 
            g_warning(str.c_str()); break;

        case INFO: 
            g_message(str.c_str()); break;

        case ERROR: 
            g_critical(str.c_str()); break;

        case CRITICAL: 
            g_error(str.c_str()); break;
                    
        default:
            break;


    }

}
#endif
#endif

#endif //  _LOG_WRITER_H_
