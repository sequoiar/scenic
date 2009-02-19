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


const long long Payloader::MAX_PTIME = 2000000;

RtpPay::~RtpPay()
{
    Pipeline::Instance()->remove(&rtpPay_);
}

void Payloader::init()
{
    g_object_set(G_OBJECT(rtpPay_), "max-ptime", Payloader::MAX_PTIME, NULL);
}

void H264Payloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph264pay", NULL);
}


void H264Depayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph264depay", NULL);
}



void H263Payloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph263pay", NULL);
}


void H263Depayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph263depay", NULL);
}


void Mpeg4Payloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmp4vpay", NULL);
}


void Mpeg4Depayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmp4vdepay", NULL);
}


void VorbisPayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpvorbispay", NULL);
    Payloader::init();
}


void VorbisDepayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpvorbisdepay", NULL);
}


void L16Payloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpL16pay", NULL);
    Payloader::init();
}


void L16Depayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpL16depay", NULL);
}


void MpaPayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmpapay", NULL);
}


void MpaDepayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmpadepay", NULL);
}

