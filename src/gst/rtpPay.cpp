/* rtpPay.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include <gst/gst.h>
#include "rtpPay.h"
#include "pipeline.h"


RtpPay::~RtpPay()
{
    pipeline_.remove(&rtpPay_);
}


Pay::Pay(const Pipeline &pipeline) : 
    RtpPay(pipeline)
{}

Pay::~Pay()
{
}


void Pay::setMTU(unsigned long long mtu)
{
    if (mtu < MIN_MTU or mtu > MAX_MTU)
        THROW_ERROR("Invalid MTU " << mtu << ", must be in range [" 
                << MIN_MTU << "-" << MAX_MTU << "]");

    tassert(rtpPay_);
    g_object_set(G_OBJECT(rtpPay_), "mtu", mtu, NULL);
}

Depay::Depay(const Pipeline &pipeline) : 
    RtpPay(pipeline)
{}

TheoraPay::TheoraPay(const Pipeline &pipeline) : Pay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtptheorapay", NULL);
}


TheoraDepay::TheoraDepay(const Pipeline &pipeline) : Depay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtptheoradepay", NULL);
}

H264Pay::H264Pay(const Pipeline &pipeline) : Pay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtph264pay", NULL);
    // FIXME: Find out why setting buffer-list to true breaks rtp so badly, DON'T SET THIS TO TRUE
    //g_object_set(rtpPay_, "buffer-list", TRUE, NULL);
}


H264Depay::H264Depay(const Pipeline &pipeline) : Depay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtph264depay", NULL);
}



H263Pay::H263Pay(const Pipeline &pipeline) : Pay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtph263ppay", NULL);
}


H263Depay::H263Depay(const Pipeline &pipeline) : Depay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtph263pdepay", NULL);
}


Mpeg4Pay::Mpeg4Pay(const Pipeline &pipeline) : Pay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtpmp4vpay", NULL);
    // this will send config header in rtp packets
    g_object_set(rtpPay_, "send-config", TRUE, NULL);

    // this means that our payloader will output bufferlists instead of
    // 1 packet per buffer. this will allow downstream elements that are bufferlist aware
    // to avoid unneeded memcpys
    // FIXME: Find out why setting buffer-list to true breaks rtp so badly, DON'T SET THIS TO TRUE
    // this bug might have been fixed with (in gst-plugins-base) on oct. 16th
    // so expected in gst-plugins-base-0.10.26
    // commit 7bca2a001941798c1a4005ee37802708ed13c225
    //
    // rtp: Correct timestamping of buffers when buffer_lists are used
    //         
    // The timestamping of buffers when buffer_lists are used failed if
    // a buffer did not have both a timestamp and an offset.
    //
    // g_object_set(rtpPay_, "buffer-list", TRUE, NULL);
    // The default of true works fine for perfect-rtptime
    // g_object_set(rtpPay_, "perfect-rtptime", FALSE, NULL);
}


bool Mpeg4Pay::handleMessage(const std::string &path, const std::string &/*arguments*/)
{
    if (path == "disable-send-config")
    {
        assert(rtpPay_);
        LOG_DEBUG("setting send-config to false in rtpmp4vpay");
        g_object_set(rtpPay_, "send-config", FALSE, NULL);
        return true;
    }
    return false;
}


Mpeg4Depay::Mpeg4Depay(const Pipeline &pipeline) : Depay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtpmp4vdepay", NULL);
}


VorbisPay::VorbisPay(const Pipeline &pipeline) : Pay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtpvorbispay", NULL);
    g_object_set(G_OBJECT(rtpPay_), "max-ptime", Pay::MAX_PTIME, NULL);
}


VorbisDepay::VorbisDepay(const Pipeline &pipeline) : Depay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtpvorbisdepay", NULL);
}


L16Pay::L16Pay(const Pipeline &pipeline) : Pay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtpL16pay", NULL);
    g_object_set(G_OBJECT(rtpPay_), "max-ptime", Pay::MAX_PTIME, NULL);
}


L16Depay::L16Depay(const Pipeline &pipeline) : Depay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtpL16depay", NULL);
}


MpaPay::MpaPay(const Pipeline &pipeline) : Pay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtpmpapay", NULL);
}


MpaDepay::MpaDepay(const Pipeline &pipeline) : Depay(pipeline)
{
    rtpPay_ = pipeline_.makeElement("rtpmpadepay", NULL);
}

