
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

#include <vector>

class RemoteConfig;
class _GstElement;
class _GObject;

class RtpBin
{
    public:
        virtual ~RtpBin();
        void init();

    protected:
        RtpBin() : rtcp_sender_(0), rtcp_receiver_(0) { ++sessionCount_; }
        static const char *padStr(const char *padName);

        static _GstElement *rtpbin_;
        static bool destroyed_;
        static int sessionCount_;
        _GstElement *rtcp_sender_, *rtcp_receiver_;
        static std::vector<std::string> sessionNames_;

    private:
        static const int REPORTING_PERIOD_MS = 5000;
        static int printStatsCallback(void * rtpbin);
        static void printSourceStats(_GObject *source);
        static void parseSourceStats(_GObject * source, int sessionId);

        RtpBin(const RtpBin&); //No Copy Constructor
        RtpBin& operator=(const RtpBin&); //No Assignment Operator
};

#endif // _RTP_BIN_H_

