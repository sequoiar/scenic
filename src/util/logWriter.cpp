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
#include <sstream>
#include<iomanip>

#define BACKTRACE
#ifdef BACKTRACE
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <stdlib.h>
#endif

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
        case PRINT:
        case WARNING:
        case ERROR:
        case THROW: 
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


std::string log_(const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &/*functionName*/, int lineNum)
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
void cerr_log_throw( const std::string &msg, LogLevel level, const std::string &fileName,
        const std::string &functionName, int lineNum)
{
    std::string strerr = log_(msg, level, fileName, functionName, lineNum);
    
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

#ifdef BACKTRACE
void backtrace()
{
    void *trace[16];
    char **messages = (char **)NULL;
    int status, i, trace_size = 0;
    Dl_info dlinfo;
    const char *symname;
    char *demangled;

    trace_size = backtrace(trace, 16);
    messages = backtrace_symbols(trace, trace_size);
    for (i=0; i < trace_size; ++i)
    {
        if(!dladdr(trace[i], &dlinfo))
            continue;

        symname = dlinfo.dli_sname;
        demangled = abi::__cxa_demangle(symname, NULL, 0, &status);
        if(status == 0 && demangled)
            symname = demangled;

        std::cerr << "object:" << dlinfo.dli_fname << " function:" <<  symname << std::endl;

        if (demangled)
            free(demangled);
    }
}
#else
void backtrace(){}
#endif
void assert_throw(__const char *__assertion, __const char *__file,
        unsigned int __line, __const char *__function)
{
    backtrace();
    cerr_log_throw(__assertion, ASSERT_FAIL, __file, __function, __line);
}
