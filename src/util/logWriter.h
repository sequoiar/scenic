/* logWriter.h
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

/** \file
 *      Log writer macro, usage:
 *
 *      LOG_DEBUG("This is a log");
 *      
 *      LOG_INFO("Got Error errno: " << 100);
 *      
 *      THROW_ERROR(99);
 *
 *      To disable logging, define the macro ENABLE_LOG (in this file) to 0.
 *
 */

#ifndef _LOG_WRITER_H_
#define _LOG_WRITER_H_

#include <string>
#include <sstream>
#include "config.h"


#define ENABLE_LOG 1


#if !ENABLE_LOG
#define LOG(msg, level)
#else
#define LOG(msg, level)     LOG_(msg,level,0)
#endif

/** higher values are more critical */
enum LogLevel {
    NONE = 0,
    DEBUG = 10,
    INFO = 20,
    WARNING = 30,
    ERROR = 40,
    CRITICAL = 50,
    ASSERT_FAIL = 60
};
#define THROW_ERRNO(msg,err)      THROW_(msg, ERROR,err)
#define THROW_ERROR(msg)      THROW_(msg, ERROR,0)
#define THROW_CRITICAL(msg)   THROW_(msg, CRITICAL,0)
#define LOG_INFO(msg)       LOG(msg, INFO)
#define LOG_WARNING(msg)    LOG(msg, WARNING)
#define LOG_DEBUG(msg)      LOG(msg, DEBUG)

#define COUT_LOG(msg)       LOG(msg, NONE)
/** base exception class */
class Except : public std::exception
{
public:
    LogLevel log_;
    std::string msg_;
    int errno_;

    Except(std::string log_msg,int err):log_(WARNING),msg_(log_msg),errno_(err){}
    Except():log_(NONE),msg_(),errno_(0){}
    virtual ~Except() throw(){}
};

/** Recovery is possible */
class ErrorExcept : public Except
{
public:
    ErrorExcept(std::string log_msg, int err=0):Except(log_msg,err){log_ = ERROR;}

};

/** Tries to cleanup before exit */
class CriticalExcept : public Except
{
public:
    CriticalExcept(std::string log_msg, int err=0):Except(log_msg,err){ log_ = CRITICAL;}
};

/** Assertion failed */
class AssertExcept : public CriticalExcept
{
public:
    AssertExcept(std::string log_msg):CriticalExcept(log_msg){log_ = ASSERT_FAIL;}
};

namespace logger
{
    /// log Subscriber 
    class Subscriber
    {
        public:
            void enable();
            void hold();
            Subscriber();
            virtual void operator()(LogLevel&, std::string &){}
            virtual ~Subscriber();
    };
}

#define QUIET_THROW(msg)           LOG_(msg,NONE,-1)
#define THROW_(msg, level,err)     LOG_(msg,level,err)
//Note mangle84579568749576948 varible name so that hiding an existing 
//variable is unlikely
//Do{} while(0) construct to preserve one statement syntax of LOG()

#define LOG_(msg, level, err)                 \
            do{                         \
            std::ostringstream mangle84579568749576948;      \
            mangle84579568749576948 << msg;                  \
            cerr_log_(mangle84579568749576948.str(), level, __FILE__, __FUNCTION__, __LINE__,err);    \
            }                           \
            while(0)

std::string log_(const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, int lineNum);

void cerr_log_( const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, int lineNum,int err);


#endif //  _LOG_WRITER_H_
