
// rtpSender.h
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

#ifndef _RTP_SENDER_H_
#define _RTP_SENDER_H_

#include <string>
#include <tr1/memory>
#include "rtpBin.h"
#include "busMsgHandler.h"

class SenderConfig;
class _GstElement;
class _GstPad;
class _GParamSpec;
class _GstMessage;
class RtpPay;

class RtpSender
    : public RtpBin, public BusMsgHandler
{
    public:
        RtpSender() : rtp_sender_(0), config_() {}
        void checkSampleRate();

        static void enableControl();

        ~RtpSender();

        void add(RtpPay * payloader, const SenderConfig & config);

    private:
        _GstElement *rtp_sender_;
        std::tr1::shared_ptr<SenderConfig> config_;
        bool handleBusMsg(_GstMessage *msg);
        static void sendCapsChanged(_GstPad *pad, _GParamSpec *pspec, RtpSender *context);
        virtual void subParseSourceStats(_GstStructure *stats);
        RtpSender(const RtpSender&); //No Copy Constructor
        RtpSender& operator=(const RtpSender&); //No Assignment Operator
};

#endif // _RTP_SENDER_H_

