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

#include "log_writer.h"

#include <iostream>
#include <sstream>
#include<iomanip>

#ifdef CONFIG_DEBUG_LOCAL
#define LOG_LEVEL DEBUG
#else
#define LOG_LEVEL INFO
#endif

std::string logLevelStr(LogLevel level)
{
    std::string lstr;
    switch (level)
    {
        case DEBUG:
            lstr = "DEBUG";
            break;
        case INFO:
            lstr = "INFO";
            break;
        case THROW:
            lstr = "THROW";
            break;
        case WARNING:
            lstr = "WARNING";
            break;
        case ERROR:
            lstr = "ERROR";
            break;
        case CRITICAL:
            lstr = "CRITICAL";
            break;
        case ASSERT_FAIL:
            lstr = "ASSERT_FAIL";
            break;
        default:
            lstr = "INVALID LOG LEVEL";
    }
    return lstr;
}

static Log::Subscriber emptyLogSubscriber;
static Log::Subscriber* lf = &emptyLogSubscriber;

Log::Subscriber::~Subscriber()
{
    lf = &emptyLogSubscriber;
}

Log::Subscriber::Subscriber()
{
    lf = this;
}


std::string log_(const std::string &msg, LogLevel level,
        const std::string &fileName, int lineNum)
{
    std::ostringstream logMsg;
    if (level == PRINT) // for normal printing without log formatting
    {
        logMsg << msg;
        return logMsg.str();
    }

#ifdef CONFIG_DEBUG_LOCAL
    if(level >= INFO and level < WARNING)
        logMsg << logLevelStr(level) << ":" << msg << std::endl;
    else
        logMsg << fileName << ":" <<
            lineNum << ":" << logLevelStr(level) << ":" << msg << std::endl;
#else
    logMsg <<  logLevelStr(level) << ":" << msg <<  std::endl;
#endif

    return logMsg.str();
}

//TODO DOCUMENT THIS
void cerr_log_throw(const std::string &msg, LogLevel level,
        const std::string &fileName, int lineNum)
{
    std::string strerr = log_(msg, level, fileName, lineNum);

    if (level < THROW)
    {
        (*lf)(level, strerr);  // log it now if not a throw
        return;
    }

    if (level < CRITICAL)
        throw (ErrorExcept(strerr.c_str()));
    else if (level < ASSERT_FAIL)
        throw (CriticalExcept(strerr.c_str()));
    else
        throw (AssertExcept(strerr.c_str()));
}

