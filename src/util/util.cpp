// util.cpp
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
 *      Utility functions for logWriter 
 */


#include <iostream>
#include <time.h>
#include <sstream>
#include <stdlib.h>
#include "config.h"
#include "logWriter.h"
#include "lassert.h"


void assert_throw(__const char *__assertion, __const char *__file,
                           unsigned int __line, __const char *__function)
{
    cerr_log_( __assertion, ASSERT_FAIL, __file, __function, __line,0);
}

bool logLevelIsValid(LogLevel level)
{
    switch (level)
    {
        case DEBUG:
        case INFO:
        case WARNING:
        case ERROR:
        case CRITICAL:
        case ASSERT_FAIL:
            return true;
            break;
        default:
            return false;
            break;
    }
}


std::string logLevelStr(LogLevel level)
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
        case ASSERT_FAIL:
            return "ASSERT_FAIL: ";
        default:
            return "INVALID LOG LEVEL: ";
    }
}

static LogFunctor emptyLogFunctor;
static LogFunctor* lf = &emptyLogFunctor;
static bool hold = false;

void LOG::register_cb(LogFunctor* f)
{
    lf = f;
}
void LOG::unregister_cb()
{
    lf = &emptyLogFunctor;
}
void LOG::hold_cb()
{
    hold = true;
}

void LOG::release_cb()
{
    hold = false;
}

bool logLevelMatch(LogLevel level)
{
    if (level >= LOG_LEVEL && logLevelIsValid(level))
        return true;
    else
        return false;
}


std::string log_(const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, int lineNum)
{
    std::ostringstream logMsg;
    if (logLevelMatch(level))
    {
        time_t rawtime;
        struct tm * timeinfo;

        time( &rawtime );
        timeinfo = localtime(&rawtime);
        logMsg << logLevelStr(level) << msg << " " << functionName <<  "() in " << fileName << ":" << " line " << lineNum << " " <<asctime(timeinfo); 
        // FIXME: send message to Core
    }

    return logMsg.str();
}

void cerr_log_( const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, int lineNum,int err)
{
    std::string strerr = log_(msg,level,fileName,functionName,lineNum);

    if(!hold)
        (*lf)(level,strerr);
    
    std::cerr << strerr;
    if(level < ERROR)
        return;
    

    if(level < CRITICAL)
        throw(ErrorExcept(strerr,err));
    if(level < ASSERT_FAIL)
        throw(CriticalExcept(strerr,err));
    throw(AssertExcept(strerr));

}


