
// rtpBin.h
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

#ifndef _RTP_BIN_H_
#define _RTP_BIN_H_

//#include "/home/tristan/gst-plugins-bad/gst/rtpmanager/rtpsession.h"

class RemoteConfig;
class _GstElement;
class _GObject;

class RtpBin
{
    public:
        virtual ~RtpBin();
        void init();
        //virtual void checkSampleRate() = 0;
        double bandwidth() const;

    protected:
        RtpBin() : rtcp_sender_(0), rtcp_receiver_(0) { ++refCount_; }
        static const char *padStr(const char *padName);

        static _GstElement *rtpbin_;
        static unsigned int refCount_;
        _GstElement *rtcp_sender_, *rtcp_receiver_;

    private:
        static bool requestSession();
        static _GObject *gotInternalSessionCb(_GstElement *rtpBin, unsigned int session, void *data);

        RtpBin(const RtpBin&); //No Copy Constructor
        RtpBin& operator=(const RtpBin&); //No Assignment Operator
};

#endif // _RTP_BIN_H_

