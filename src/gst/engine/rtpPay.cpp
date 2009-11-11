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
    Pipeline::Instance()->remove(&rtpPay_);
}

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


TheoraPay::TheoraPay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtptheorapay", NULL);
}


TheoraDepay::TheoraDepay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtptheoradepay", NULL);
}

H264Pay::H264Pay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph264pay", NULL);
    // FIXME: Find out why setting buffer-list to true breaks rtp so badly, DON'T SET THIS TO TRUE
    //g_object_set(rtpPay_, "buffer-list", TRUE, NULL);
}


H264Depay::H264Depay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph264depay", NULL);
}



H263Pay::H263Pay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph263ppay", NULL);
}


H263Depay::H263Depay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph263pdepay", NULL);
}


Mpeg4Pay::Mpeg4Pay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmp4vpay", NULL);
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


Mpeg4Depay::Mpeg4Depay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmp4vdepay", NULL);
}


VorbisPay::VorbisPay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpvorbispay", NULL);
    g_object_set(G_OBJECT(rtpPay_), "max-ptime", Pay::MAX_PTIME, NULL);
}


VorbisDepay::VorbisDepay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpvorbisdepay", NULL);
}


L16Pay::L16Pay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpL16pay", NULL);
    g_object_set(G_OBJECT(rtpPay_), "max-ptime", Pay::MAX_PTIME, NULL);
}


L16Depay::L16Depay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpL16depay", NULL);
}


MpaPay::MpaPay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmpapay", NULL);
}


MpaDepay::MpaDepay()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmpadepay", NULL);
}

