
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

class RemoteConfig;
class _GstElement;
class _GObject;

class RtpBin
{
    public:
        virtual ~RtpBin();
        void init();

    protected:
        RtpBin() : rtcp_sender_(0), rtcp_receiver_(0) { ++refCount_; }
        static const char *padStr(const char *padName);

        static _GstElement *rtpbin_;
        static unsigned int refCount_;
        _GstElement *rtcp_sender_, *rtcp_receiver_;

    private:
        double bandwidth(unsigned int sessionId) const;
        double jitter(unsigned int sessionId) const;
        void bandwidth(unsigned int sessionId, double newBandwidth);
        void dropOnLatency(unsigned int sessionId);
        static int increaseBandwidth(void * data);
        static int printBandwidth(void * data);
        static int printJitter(void * data);
        static bool printJitter(unsigned int sessionID);
        static int dropOnLatency(void * data);
        static _GObject *session_;
        static bool requestSession(unsigned int sessionId);
        static _GObject *gotInternalSessionCb(_GstElement * /*rtpBin*/, unsigned int session, void *data);

        RtpBin(const RtpBin&); //No Copy Constructor
        RtpBin& operator=(const RtpBin&); //No Assignment Operator
};

#endif // _RTP_BIN_H_

