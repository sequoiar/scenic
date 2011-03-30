
/* milhouseLogger.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _MILHOUSE_LOGGER_H_
#define _MILHOUSE_LOGGER_H_

#include "ConcurrentQueue.h"
#include "util/logWriter.h"

class MilhouseLogger
    : public Log::Subscriber
{
    public:
        MilhouseLogger(const std::string &logLevel);
        ~MilhouseLogger();
        void operator()(LogLevel&, std::string& msg);
        bool gstDebug() { return gstDebug_; }
    private:
        ConcurrentQueue<std::string> printQueue_;
        boost::thread printThread_;
        void printMessages();   // this runs in printThread
        LogLevel argToLogLevel(const std::string &logLevel);
        bool gstDebug_;
        const LogLevel level_;
};

#endif // _MILHOUSE_LOGGER_H_
