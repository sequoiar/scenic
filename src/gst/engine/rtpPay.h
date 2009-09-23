// rtpPay.h
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

#ifndef _PAYLOADER_H_
#define _PAYLOADER_H_

#include "gstLinkable.h"
#include "messageHandler.h"

class _GstElement;

class RtpPay : public GstLinkableFilter
{
    public:
        RtpPay() : rtpPay_(0) {}
        virtual void init() = 0;
        virtual ~RtpPay();
        _GstElement *srcElement() { return rtpPay_; }
        _GstElement *sinkElement() { return rtpPay_; }

    protected:
        _GstElement *rtpPay_;

    private:
        RtpPay(const RtpPay&);     //No Copy Constructor
        RtpPay& operator=(const RtpPay&);     //No Assignment Operator
};

class _GtkAdjustment;
class _GtkWidget;

class Payloader : public RtpPay
{
    public:
        static void enableControl();
        virtual ~Payloader();
        void init() = 0;
    protected:
        static const long long MAX_PTIME = 2000000LL;

    private:
        // Use max-ptime to limit the amount of 
        // encoded media packets in an RTP packet. Reduces latency
        void setMTU(unsigned long long mtu);
        // hardcoded in gst-plugins-base/gst-libs/gst/rtp/gstbasertppayload.c
        static const unsigned long long INIT_MTU = 1400;    
        static const unsigned long long MIN_MTU = 28;    
        static const unsigned long long MAX_MTU = 14000;

        void createMTUControl();
        static void updateMTUCb(_GtkAdjustment *adj, void *data);

        static bool controlEnabled_;
        static _GtkWidget *control_;
};


class Depayloader : public RtpPay
{
};


class TheoraPayloader : public Payloader
{
    private: 
        void init();
};


class TheoraDepayloader : public Depayloader
{
    private: 
        void init();
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


class H263Payloader : public Payloader
{
    private: 
        void init();
};


class H263Depayloader : public Depayloader
{
    private: 
        void init();
};


class Mpeg4Payloader : public Payloader, public MessageHandler
{
    private: 
        void init();
        bool handleMessage(const std::string &path); 
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

