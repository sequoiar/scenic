
// rtpSession.h
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
// Singleton class that multiple streams can connect to

#ifndef _RTP_SESSION_H_
#define _RTP_SESSION_H_

#include <vector>
#include <gst/gst.h>
#include "gstBase.h"

class MediaConfig;

class RtpSession : public GstBase
{
    public:
        virtual bool init();

    protected: 
        RtpSession() {};
        virtual const char *padStr(const char* padName) = 0;
        GstElement *rtpbin_;

};



class RtpSender : public RtpSession
{
    public:
        RtpSender() {};
        void addSource(GstElement *src, MediaConfig &config);    // add a new media input

    private:
        virtual const char *padStr(const char* padName);
        std::vector<GstElement*> rtp_senders_, rtcp_senders_, rtcp_receivers_;
};



class RtpReceiver : public RtpSession
{
    public:
        RtpReceiver() {};
        void addSink(GstElement *sink, MediaConfig &config);  // add a new media output

    private:
        virtual const char *padStr(const char* padName);
        std::vector<GstElement*> rtp_receivers_, rtcp_receivers_, rtcp_senders_;
};

#endif // _RTP_SESSION_H_

