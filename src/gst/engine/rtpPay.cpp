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
#include <gtk/gtk.h>
#include "rtpPay.h"
#include "pipeline.h"


RtpPay::~RtpPay()
{
    Pipeline::Instance()->remove(&rtpPay_);
}

const long long Payloader::MAX_PTIME = 2000000LL;
bool Payloader::controlEnabled_ = false;
GtkWidget *Payloader::control_ = 0;


Payloader::~Payloader()
{
    if (control_)
    {
        gtk_widget_destroy(control_);
        LOG_DEBUG("RTP jitterbuffer control window destroyed");
        control_ = 0;
    }
}

void Payloader::enableControl() 
{ 
    controlEnabled_ = true; 
}



void Payloader::init()
{
    if (controlEnabled_)
        createMTUControl();
}

void Payloader::setMTU(unsigned long long mtu)
{
    if (mtu < MIN_MTU or mtu > MAX_MTU)
        THROW_ERROR("Invalid MTU " << mtu << ", must be in range [" 
                << MIN_MTU << "-" << MAX_MTU << "]");

    tassert(rtpPay_);
    g_object_set(G_OBJECT(rtpPay_), "mtu", mtu, NULL);
}


void Payloader::createMTUControl()
{
    static bool gtk_initialized = false;
    if (!gtk_initialized)
    {
        gtk_init(0, NULL);
        gtk_initialized = true;
    }

    GtkWidget *box1;
    GtkWidget *hscale;
    GtkObject *adj;
    const int WIDTH = 400;
    const int HEIGHT = 70;

    /* Standard window-creating stuff */
    control_ = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(control_), WIDTH, HEIGHT);
    gtk_window_set_title (GTK_WINDOW (control_), "MTU Size");

    box1 = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (control_), box1);

    /* value, lower, upper, step_increment, page_increment, page_size */
    /* Note that the page_size value only makes a difference for
     * scrollbar widgets, and the highest value you'll get is actually
     * (upper - page_size). */

    adj = gtk_adjustment_new (INIT_MTU, MIN_MTU, MAX_MTU + 1, 1.0, 1.0, 1.0);

    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
            GTK_SIGNAL_FUNC(updateMTUCb), static_cast<void*>(this));


    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    // Signal emitted only when value is done changing
    gtk_range_set_update_policy (GTK_RANGE (hscale), 
            GTK_UPDATE_DISCONTINUOUS);
    gtk_box_pack_start (GTK_BOX (box1), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);
    gtk_widget_show (box1);
    gtk_widget_show (control_);
}


void Payloader::updateMTUCb(GtkAdjustment *adj, gpointer data)
{
    Payloader *context = static_cast<Payloader*>(data);
    unsigned val = static_cast<unsigned>(adj->value);
    context->setMTU(val);
}


void H264Payloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph264pay", NULL);
    Payloader::init();
}


void H264Depayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph264depay", NULL);
}



void H263Payloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph263ppay", NULL);
    Payloader::init();
}


void H263Depayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtph263pdepay", NULL);
}


void Mpeg4Payloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmp4vpay", NULL);
    Payloader::init();
}


void Mpeg4Depayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmp4vdepay", NULL);
}


void VorbisPayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpvorbispay", NULL);
    g_object_set(G_OBJECT(rtpPay_), "max-ptime", Payloader::MAX_PTIME, NULL);
    Payloader::init();
}


void VorbisDepayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpvorbisdepay", NULL);
}


void L16Payloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpL16pay", NULL);
    g_object_set(G_OBJECT(rtpPay_), "max-ptime", Payloader::MAX_PTIME, NULL);
    Payloader::init();
}


void L16Depayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpL16depay", NULL);
}


void MpaPayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmpapay", NULL);
    Payloader::init();
}


void MpaDepayloader::init()
{
    rtpPay_ = Pipeline::Instance()->makeElement("rtpmpadepay", NULL);
}

