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
 *      Utility functions for assert, logWriter and hostIP
 */


#include <iostream>
#include <time.h>
#include <sstream>
#include <stdlib.h>
#include "config.h"
#include "logWriter.h"
#include "lassert.h"

#include "hostIP.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

const char *get_host_ip()
{
    int i;
    char *ip = 0;
    int s = socket (PF_INET, SOCK_STREAM, 0);
    if(s == -1)
        return ip;

    for (i = 1;; i++)
    {
        struct ifreq ifr;
        struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;

        ifr.ifr_ifindex = i;
        if (ioctl (s, SIOCGIFNAME, &ifr) < 0)
            break;
        /* now ifr.ifr_name is set */
        if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
            continue;
        ip = inet_ntoa (sin->sin_addr);         // seems to be thread safe
                                                // but not reentrant
                                                // under libc6 2+
    }

    close (s);
    return ip;
}

void assert_throw(__const char *__assertion, __const char *__file,
                           unsigned int __line, __const char *__function)
{
    cerr_log_( __assertion, ASSERT_FAIL, __file, __function, __line);
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

void register_cb(LogFunctor* f)
{
    lf = f;
}
void release_cb()
{
    lf = &emptyLogFunctor;
}

bool logLevelMatch(LogLevel level)
{
    if (level >= LOG_LEVEL && logLevelIsValid(level))
        return true;
    else
        return false;
}


std::string log_(const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, const int lineNum)
{
    std::ostringstream logMsg;
    if (logLevelMatch(level))
    {
        time_t rawtime;
        struct tm * timeinfo;

        time( &rawtime );
        timeinfo = localtime(&rawtime);
        logMsg << logLevelStr(level) << msg << " --" << functionName <<  "() in " << fileName << ":" << " line " << lineNum << "-- " <<asctime(timeinfo); 
        // FIXME: send message to Core
    }

    return logMsg.str();
}

void cerr_log_( const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, const int lineNum)
{
    std::string err = log_(msg,level,fileName,functionName,lineNum);

    lf->cb(level,err);
    
    if(level < ERROR){
        std::cerr << err;
        return;
    }

    if(level < CRITICAL)
        throw(ErrorExcept(err));
    if(level < ASSERT_FAIL)
        throw(CriticalExcept(err));
    throw(AssertExcept(err));

}


