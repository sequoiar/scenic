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

#include <stdexcept>

/**
 * exception classes 
 * refer to
 * http://www.boost.org/community/error_handling.html
 * for best practices
 * http://www.boost.org/community/exception_safety.html
 * http://www.parashift.com/c++-faq-lite/exceptions.html#faq-17
 *
 */


/// higher values are more severe 
enum LogLevel {
    NONE = 0,
    DEBUG = 10,
    INFO = 20,
    PRINT = 25,
    WARNING = 30,
    THROW = 35,         //those below can throw 
    ERROR = 40,
    CRITICAL = 50,
    ASSERT_FAIL = 60
};

/** base exception class */
class Except : public std::runtime_error
{
    public:
        LogLevel log_;

        Except(const char* log_msg) : std::runtime_error(log_msg),
            log_(WARNING)
    {}
        Except() : std::runtime_error(""),
            log_(NONE)
    {}
};

/** Recovery should be possible */
class ErrorExcept : public Except
{
    public:
        ErrorExcept(const char* log_msg) : Except(log_msg)
    {
        log_ = ERROR;
    }
};

/** Tries to cleanup before exit */
class CriticalExcept : public Except
{
    public:
        CriticalExcept(const char* log_msg) : Except(log_msg)
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
