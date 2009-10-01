/* 
 * Copyright (C) 2009 Société des arts technologiques (SAT)
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
#ifndef _EXCEPT_H_
#define _EXCEPT_H_

#include <exception>
#include <string>

/**
 * exception classes 
 * refer to
 * http://www.boost.org/community/error_handling.html
 * for best practices
 * http://www.boost.org/community/exception_safety.html
 *
 * FIXME remove types that can throw on copy constructor
 * i.e. string
 */


/// higher values are more severe 
enum LogLevel {
    NONE = 0,
    DEBUG = 10,
    INFO = 20,
    WARNING = 30,
    THROW = 35,         //those below can throw 
    ERROR = 40,
    CRITICAL = 50,
    ASSERT_FAIL = 60
};

/** base exception class */
class Except : public virtual std::exception
{
    public:
        LogLevel log_;
        int errno_; //TODO used?

        Except(const char* log_msg, int err) :
            log_(WARNING), errno_(err)
    {
        int i;
        for(i = 0; i < 255 and log_msg[i]; ++i)
            buff[i] = log_msg[i];
        for(; i < 256; ++i)
            buff[i] = 0;
    }
        Except() : 
            log_(NONE), errno_(0)
    {}
        char buff[256];
        const char * what() { return buff; }
        virtual ~Except() throw() {}
};

/** Recovery should be possible */
class ErrorExcept : public Except
{
    public:
        ErrorExcept(const char* log_msg, int err = 0) : Except(log_msg, err)
    {
        log_ = ERROR;
    }
};

/** Tries to cleanup before exit */
class CriticalExcept : public Except
{
    public:
        CriticalExcept(const char* log_msg, int err = 0) : Except(log_msg,err)
    { 
        log_ = CRITICAL;
    }
};

/** Assertion failed */
class AssertExcept : public CriticalExcept
{
    public:
        AssertExcept(const char* log_msg) : 
            CriticalExcept(log_msg)
    {
        log_ = ASSERT_FAIL;
    }
};

#endif
