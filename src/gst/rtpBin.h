
// rtpBin.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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

#ifndef _RTP_BIN_H_
#define _RTP_BIN_H_

#include <map>

#include "noncopyable.h"

class Pipeline;
class _GstElement;
class _GstStructure;
class _GObject;

class RtpBin : boost::noncopyable
{
    public:
        virtual ~RtpBin();
        std::string sessionName() { return sessionName_; }

    protected:
        /// FIXME: this sessionId is all kinds of gross
        explicit RtpBin(const Pipeline& pipeline);
        const char *padStr(const char *padName) const;

        void registerSession(const std::string &codec);
        void unregisterSession();
        const Pipeline &pipeline_;
        static _GstElement *rtpbin_;
        static bool destroyed_;
        static int sessionCount_;
        _GstElement *rtcp_sender_, *rtcp_receiver_;
        int sessionId_;
        std::string sessionName_;
        static std::map<int, RtpBin*> sessions_;
        virtual void subParseSourceStats(_GstStructure *stats) = 0;
        void printStatsVal(const std::string &idStr, const char *key, const std::string &type, 
                const std::string &formatStr, _GstStructure *stats);
        bool printStats_;
        static int createSinkSocket(const char *hostname, int port);
        static int createSourceSocket(int port);
        void startPrintStatsCallback();
    
    private:
        static const int REPORTING_PERIOD_MS = 2000;
        static int printStatsCallback(void * rtpbin);
        static void printSourceStats(_GObject *source);
        static void parseSourceStats(_GObject * source, RtpBin *context);
};

#endif // _RTP_BIN_H_

