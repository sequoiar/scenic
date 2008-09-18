
// rtpReceiver.h
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

#ifndef _RTP_RECEIVER_H_
#define _RTP_RECEIVER_H_

#include <list>
#include "rtpSession.h"

class RemoteReceiverConfig;
class _GstElement;
class _GstPad;
class RtpPay;

class RtpReceiver
    : public RtpSession
{
    public:
        RtpReceiver() : rtp_receiver_(0), depayloader_(0) {}
        ~RtpReceiver();
        void set_caps(const char* capsStr);

        //void addDerived(RtpPay * depayloader, const RemoteConfig & config);
        void add(RtpPay * depayloader, const RemoteReceiverConfig & config);

    private:
        static _GstPad *get_matching_sink_pad(_GstPad *srcPad);
        static void cb_new_src_pad(_GstElement * element, _GstPad * srcPad, void *data);

        _GstElement *rtp_receiver_;
        _GstElement *depayloader_;
        static std::list<_GstElement *> usedDepayloaders_;

        RtpReceiver(const RtpReceiver&); //No Copy Constructor
        RtpReceiver& operator=(const RtpReceiver&); //No Assignment Operator
};

#endif // _RTP_RECEIVER_H_

