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
 *      or
 *      LOG("Got Error errno: " << 100);
 *      To disable logging, define the macro ENABLE_LOG (in this file) to 0.
 *
 */

#ifndef _LOG_WRITER_H_
#define _LOG_WRITER_H_

#include <string>
#include <sstream>
#include "config.h"

#ifdef CONFIG_DEBUG
#define LOG_LEVEL DEBUG
#else
#define LOG_LEVEL INFO
#endif

#define ENABLE_LOG 1


#if !ENABLE_LOG
#define LOG(msg, level)
#else


//Note mangle84579568749576948 varible name so that hiding an existing variable is unlikely
//
//Do{} while(0) construct to preserve one statement syntax of LOG()

#define LOG(msg, level)                 \
            do{                         \
            std::ostringstream mangle84579568749576948;      \
            mangle84579568749576948 << msg;                  \
            cerr_log_(mangle84579568749576948.str(), level, __FILE__, __FUNCTION__, __LINE__);    \
            }                           \
            while(0)

enum LogLevel {
    DEBUG = 10,
    INFO = 20,
    WARNING = 30,
    ERROR = 40,
    CRITICAL = 50,
    ASSERT_FAIL = 60
};

class Except
{
public:
    LogLevel log_;
    std::string msg_;

    Except(std::string log_msg):log_(WARNING),msg_(log_msg){}
    virtual ~Except(){}
};

class ErrorExcept : public Except
{
public:
    ErrorExcept(std::string log_msg):Except(log_msg){log_ = ERROR;}

};

class CriticalExcept : public ErrorExcept
{
public:
    CriticalExcept(std::string log_msg):ErrorExcept(log_msg){ log_ = CRITICAL;}
};

class AssertExcept : public CriticalExcept
{
public:
    AssertExcept(std::string log_msg):CriticalExcept(log_msg){log_ = ASSERT_FAIL;}
};



#define LOG_ERROR(msg)      LOG(msg, ERROR)
#define LOG_CRITICAL(msg)   LOG(msg, CRITICAL)
#define LOG_INFO(msg)       LOG(msg, INFO)
#define LOG_WARNING(msg)    LOG(msg, WARNING)
#define LOG_DEBUG(msg)      LOG(msg, DEBUG)




bool logLevelIsValid(LogLevel level);


std::string logLevelStr(LogLevel level);



bool logLevelMatch(LogLevel level);


std::string log_(const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, const int lineNum);

void cerr_log_( const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, const int lineNum);

#endif // !ENABLE_LOG

#endif //  _LOG_WRITER_H_

