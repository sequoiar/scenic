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
 *      To disable logging, define the macro ENABLE_LOG (in this file) to 1.
 *
 */

#ifndef _LOG_WRITER_H_
#define _LOG_WRITER_H_

#include <string>
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

#define LOG(msg, level)         \
            cerr_log_(msg, level, __FILE__, __FUNCTION__, __LINE__)

enum LogLevel {
    DEBUG = 10,
    INFO = 20,
    WARNING = 30,
    ERROR = 40,
    CRITICAL = 50,
    ASSERT_FAIL = 60
};

class except
{
public:
    LogLevel log_;
    std::string log_msg_;

    except(LogLevel log,std::string log_msg):log_(log),log_msg_(log_msg){}
    except(std::string log_msg):log_(ERROR),log_msg_(log_msg){}
};



#define LOG_ERROR(msg)      LOG(msg, ERROR)
#define LOG_CRITICAL(msg)   LOG(msg, CRITICAL)
#define LOG_INFO(msg)       LOG(msg, INFO)
#define LOG_WARNING(msg)    LOG(msg, WARNING)
#define LOG_DEBUG(msg)      LOG(msg, DEBUG)




bool logLevelIsValid(LogLevel level);


const std::string logLevelStr(LogLevel level);



bool logLevelMatch(LogLevel level);


const std::string log_(const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, const int lineNum);

void cerr_log_( const std::string &msg, LogLevel level, const std::string &fileName,
                const std::string &functionName, const int lineNum);

#endif // !ENABLE_LOG

#endif //  _LOG_WRITER_H_

