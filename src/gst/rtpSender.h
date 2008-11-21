
// rtpSender.h
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

#ifndef _RTP_SENDER_H_
#define _RTP_SENDER_H_

#include <string>
#include "rtpBin.h"

class SenderConfig;
class _GstElement;
class RtpPay;

class RtpSender
    : public RtpBin
{
    public:
        RtpSender() : rtp_sender_(0) {}
        void checkSampleRate();

        std::string getCaps() const;

        ~RtpSender();

        void add(RtpPay * payloader, const SenderConfig & config);

    private:
        _GstElement *rtp_sender_;
        RtpSender(const RtpSender&); //No Copy Constructor
        RtpSender& operator=(const RtpSender&); //No Assignment Operator
};

#endif // _RTP_SENDER_H_

