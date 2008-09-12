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
        ~RtpPay();

    protected:

        _GstElement *rtpPay_;

    private:

        _GstElement *srcElement() { return rtpPay_; }
        _GstElement *sinkElement() { return rtpPay_; }

        RtpPay(const RtpPay&);     //No Copy Constructor
        RtpPay& operator=(const RtpPay&);     //No Assignment Operator
};


class H264Payloader : public RtpPay
{
    public: 

        H264Payloader() {}
        bool init();

    private:

        H264Payloader(const H264Payloader&);     //No Copy Constructor
        H264Payloader& operator=(const H264Payloader&);     //No Assignment Operator
};


class H264Depayloader : public RtpPay
{
    public: 

        bool init();
};


class VorbisPayloader : public RtpPay
{
    public: 

        bool init();
};


class VorbisDepayloader : public RtpPay
{
    public: 

        bool init();
};


#endif //_PAYLOADER_H_

