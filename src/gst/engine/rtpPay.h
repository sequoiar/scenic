// rtpPay.h
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

#ifndef _PAYLOADER_H_
#define _PAYLOADER_H_

#include "gstLinkable.h"

class _GstElement;

class RtpPay : public GstLinkableFilter
{
    public:

        RtpPay() : rtpPay_(0) {}
        virtual void init() = 0;
        ~RtpPay();
        
        _GstElement *srcElement() { return rtpPay_; }
        _GstElement *sinkElement() { return rtpPay_; }

    protected:

        _GstElement *rtpPay_;

    private:

        RtpPay(const RtpPay&);     //No Copy Constructor
        RtpPay& operator=(const RtpPay&);     //No Assignment Operator
};


class Payloader : public RtpPay
{
    protected: 
        // amount of time to let Use max-ptime to limit the amount of 
        // vorbis packets in an RTP packet. Reduces latency
        static const long long MAX_PTIME;
        void init() = 0;
};

class Depayloader : public RtpPay
{
};


class H264Payloader : public Payloader
{
    private: 
        void init();
};


class H264Depayloader : public Depayloader
{
    private: 
        void init();
};


class Mpeg4Payloader : public Payloader
{
    private: 
        void init();
};


class Mpeg4Depayloader : public Depayloader
{
    private: 
        void init();
};


class VorbisPayloader : public Payloader
{
    private: 
        void init();
};


class VorbisDepayloader : public Depayloader 
{
    private: 
        void init();
};


class L16Payloader : public Payloader
{
    private: 
        void init();
};

class L16Depayloader : public Depayloader 
{
    private: 
        void init();
};


class MpaPayloader : public Payloader
{
    private: 
        void init();
};

class MpaDepayloader : public Depayloader
{
    private: 
        void init();
};

#endif //_PAYLOADER_H_

