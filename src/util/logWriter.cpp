/* logWriter.cpp
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

#include "logWriter.h"

#include <iostream>
#include <ctime>
#include <sstream>

#ifdef CONFIG_DEBUG_LOCAL
#define LOG_LEVEL DEBUG
#else
#define LOG_LEVEL INFO
#endif

bool logLevelIsValid(LogLevel level)
{
    switch (level)
    {
        case DEBUG:
        case INFO:
        case WARNING:
        case THROW:
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

#ifdef COLOR_OUTPUT
std::string logLevelStr(LogLevel level)
{
    std::string lstr;
    switch (level)
    {
        case DEBUG:
            lstr = "\r\x1b[19C\x1b[32mDEBUG";
            break;
        case INFO:
            lstr = "\r\x1b[19C\x1b[34mINFO";
            break;
        case THROW:
            lstr = "\r\x1b[19C\x1b[33mTHROW";
            break;
        case WARNING:
            lstr = "\r\x1b[19C\x1b[33mWARNING";
            break;
        case ERROR:
            lstr = "\r\x1b[19C\x1b[31mERROR";
            break;
        case CRITICAL:
            lstr = "\r\x1b[19C\x1b[41mCRITICAL";
            break;
        case ASSERT_FAIL:
            lstr = "\r\x1b[19C\x1b[41mASSERT_FAIL";
            break;
        default:
            lstr = "INVALID LOG LEVEL";
    }
    lstr += "\x1b[0m: ";
    return lstr;
}
#else
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
#endif

static Log::Subscriber emptyLogSubscriber;
static Log::Subscriber* lf = &emptyLogSubscriber;
static bool hold_flag = false;


Log::Subscriber::~Subscriber()
{
    lf = &emptyLogSubscriber;
}


void Log::Subscriber::hold()
{
    hold_flag = true;
}


void Log::Subscriber::enable()
{
    hold_flag = false;
}

Log::Subscriber::Subscriber()
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

#include <iomanip>

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
        logMsg << std::setfill('0') << std::setw(2) 
            << timeinfo->tm_hour <<":"<< std::setw(2) << timeinfo->tm_min 
            <<":" << std::setw(2) << timeinfo->tm_sec 
            << ":line" << std::setfill('0') << std::setw(5) << lineNum  
            << ":" << functionName <<  "():" << fileName 
            << ":" << logLevelStr(level) << ":" << msg << std::endl;
#else
        logMsg <<  msg <<  std::endl;
#endif
    }

    return logMsg.str();
}


void cerr_log_throw( const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, int lineNum,int err)
{
    std::string strerr = log_(msg,level,fileName,functionName,lineNum);

    if(err == -1)
        throw(Except(msg,0));
    if(!hold_flag)
        (*lf)(level,strerr);
     
    if(level == DEBUG || level == INFO)
    {
//        std::cout << strerr;
        return;
    }
//    std::cerr << strerr;
    if(level < THROW)
        return;

    if(level < CRITICAL)
        throw(ErrorExcept(strerr,err));
    if(level < ASSERT_FAIL)
        throw(CriticalExcept(strerr,err));
    throw(AssertExcept(strerr));

}

#ifdef BACKTRACE
#include <signal.h>
#include <execinfo.h>

void assert_throw(__const char *__assertion, __const char *__file,
                           unsigned int __line, __const char *__function)
{
  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;

  trace_size = backtrace(trace, 16);
  std::cout << trace_size << std::endl;
  messages = backtrace_symbols(trace, trace_size);
  for (i=0; i < trace_size; ++i)
        std::cerr << messages[i] << std::endl;
  cerr_log_throw( __assertion, ASSERT_FAIL, __file, __function, __line,0);
}
#else

void assert_throw(__const char *__assertion, __const char *__file,
                           unsigned int __line, __const char *__function)
{
    cerr_log_throw( __assertion, ASSERT_FAIL, __file, __function, __line,0);
}
#endif
