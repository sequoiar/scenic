// rtpPay.cpp
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

#include <gst/gst.h>
#include <cassert>
#include "rtpPay.h"
#include "pipeline.h"

void RtpPay::init()
{
    assert(rtpPay_);
    pipeline_.add(rtpPay_);
}

RtpPay::~RtpPay()
{
    stop();
    pipeline_.remove(&rtpPay_);
}


void H264Payloader::init()
{
    assert(rtpPay_ = gst_element_factory_make("rtph264pay", NULL));
    RtpPay::init();
}


void H264Depayloader::init()
{
    assert(rtpPay_ = gst_element_factory_make("rtph264depay", NULL));
    RtpPay::init();
}

const int VorbisPayloader::MAX_PTIME = 200000;

void VorbisPayloader::init()
{
    assert(rtpPay_ = gst_element_factory_make("rtpvorbispay", NULL));
    g_object_set(G_OBJECT(rtpPay_), "max-ptime", VorbisPayloader::MAX_PTIME, NULL);
    RtpPay::init();
}


void VorbisDepayloader::init()
{
    assert(rtpPay_ = gst_element_factory_make("rtpvorbisdepay", NULL));
    RtpPay::init();
}


void L16Payloader::init()
{
    assert(rtpPay_ = gst_element_factory_make("rtpL16pay", NULL));
    RtpPay::init();
}


void L16Depayloader::init()
{
    assert(rtpPay_ = gst_element_factory_make("rtpL16depay", NULL));
    RtpPay::init();
}


void MpaPayloader::init()
{
    assert(rtpPay_ = gst_element_factory_make("rtpmpapay", NULL));
    RtpPay::init();
}


void MpaDepayloader::init()
{
    assert(rtpPay_ = gst_element_factory_make("rtpmpadepay", NULL));
    RtpPay::init();
}

