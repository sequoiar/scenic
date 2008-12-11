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
#include <ctime>
#include <sstream>
#include "util.h"

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
    std::string lstr;
    switch (level)
    {
        case DEBUG:
            lstr = "\r\x1b[32mDEBUG";
            break;
        case INFO:
            lstr = "\r\x1b[34mINFO";
            break;
        case WARNING:
            lstr = "\r\x1b[33mWARNING";
            break;
        case ERROR:
            lstr = "\r\x1b[31mERROR";
            break;
        case CRITICAL:
            lstr = "\r\x1b[41mCRITICAL";
            break;
        case ASSERT_FAIL:
            lstr = "\r\x1b[41mASSERT_FAIL";
            break;
        default:
            lstr = "INVALID LOG LEVEL";
    }
    lstr += "\x1b[0m: ";
    return lstr;
}

static logger::Subscriber emptyLogSubscriber;
static logger::Subscriber* lf = &emptyLogSubscriber;
static bool hold_flag = false;


logger::Subscriber::~Subscriber()
{
    lf = &emptyLogSubscriber;
}


void logger::Subscriber::hold()
{
    hold_flag = true;
}


void logger::Subscriber::enable()
{
    hold_flag = false;
}

logger::Subscriber::Subscriber()
{
    lf = this;
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
#ifdef CONFIG_DEBUG_LOCAL
        time_t rawtime;
        time( &rawtime );
        struct tm * timeinfo = localtime(&rawtime);
        //asctime adds a linefeed
        logMsg << logLevelStr(level) << msg << " " << functionName <<  "() in " << fileName
            << ":" << " line " << lineNum << " " <<asctime(timeinfo);
#else
        logMsg <<  msg << std::endl;
#endif
    }

    return logMsg.str();
}


void cerr_log_( const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, int lineNum,int err)
{
    std::string strerr = log_(msg,level,fileName,functionName,lineNum);

    if(err == -1)
        throw(Except(msg,0));
    if(!hold_flag)
        (*lf)(level,strerr);
     
    if(level == DEBUG || level == INFO)
    {
        std::cout << strerr;
        return;
    }
    std::cerr << strerr;
    if(level < ERROR)
        return;

    if(level < CRITICAL)
        throw(ErrorExcept(strerr,err));
    if(level < ASSERT_FAIL)
        throw(CriticalExcept(strerr,err));
    throw(AssertExcept(strerr));

}


