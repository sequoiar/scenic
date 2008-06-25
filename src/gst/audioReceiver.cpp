
// audioReceiver.cpp
// Copyright 2008 Koya Charles Tristan Matthews 
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


#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <cassert>
#include <gst/gst.h>

#include "lo/lo.h"

#include "logWriter.h"
#include "mediaBase.h"
#include "audioReceiver.h"
#include "audioConfig.h"



AudioReceiver::AudioReceiver(const AudioConfig& config) : config_(config), gotCaps_(false)
{
   // empty 
}



AudioReceiver::~AudioReceiver() 
{
    // empty
}



void AudioReceiver::wait_for_caps() 
{
    LOG("Waiting for caps...");

    lo_server_thread st = lo_server_thread_new("7770", liblo_error);

    lo_server_thread_add_method(st, "/audio/rx", "s", caps_handler, (void *) this); 

    lo_server_thread_start(st);

    while (!gotCaps_)
        usleep(1000);

    lo_server_thread_free(st);
}

void AudioReceiver::liblo_error(int num, const char *msg, const char *path)
{
        printf("liblo server error %d in path %s: %s\n", num, path, msg);
            fflush(stdout);
}


int AudioReceiver::caps_handler(const char *path, const char *types, lo_arg **argv, int argc, 
        void *data, void *user_data)
{
    static_cast<AudioReceiver*>(user_data)->set_caps(&argv[0]->s);
    return 0;
}



void AudioReceiver::set_caps(const char *capsStr)
{
    GstCaps *caps;
    caps = gst_caps_from_string(capsStr);
    assert(caps);
    g_object_set(G_OBJECT(source_), "caps", caps, NULL);
    gst_caps_unref(caps);
    gotCaps_ = true;
}



void AudioReceiver::init_source()
{
    source_= gst_element_factory_make("udpsrc", "source");
    assert(source_);
    
    g_object_set(G_OBJECT(source_), "port", config_.port(), NULL);

    gst_bin_add(GST_BIN(pipeline_), source_);
}



void AudioReceiver::init_codec()
{
    depayloader_ = gst_element_factory_make("rtpvorbisdepay", NULL);
    assert(depayloader_);

    decoder_ = gst_element_factory_make("vorbisdec", NULL);
    assert(decoder_);

    gst_bin_add_many(GST_BIN(pipeline_), depayloader_, decoder_, NULL);

    assert(gst_element_link_many(source_, depayloader_, decoder_, NULL));
}



void AudioReceiver::init_sink()
{
    sink_ = gst_element_factory_make("jackaudiosink", NULL);
    assert(sink_);
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    
    gst_bin_add(GST_BIN(pipeline_), sink_); 

    assert(gst_element_link(decoder_, sink_));
}

#if 0
bool AudioReceiver::init_uncomp(int port, int numChannels)
{
    GError *error = NULL;

    if (numChannels != 1)
    {
        std::cout << __FILE__ << __LINE__ << ":I'm not yet built for " << numChannels << std::endl; 
        return false;
    }

    numChannels_ = numChannels;

    if (port < 1000)
        port_ = DEF_PORT;
    else
        port_ = port;
    
    // FIXME: caps shouldn't be hardcoded
    std::string capsString = "application/x-rtp,media=(string)audio,clock-rate=(int)8000,"
                              "encoding-name=(string)PCMA"; 
    std::stringstream port1, port2, port3;
    port1  << port_; 
    port2 << port_ + 1; 
    port3 << port + 5;

    std::string launchStr = " gstrtpbin name=rtpbin \\ " 
       "udpsrc caps=" + capsString + " port=" + port1.str() + " ! rtpbin.recv_rtp_sink_0 \\ "
       "rtpbin. ! rtppcmadepay ! alawdec ! audioconvert ! audioresample ! jackaudiosink \\ "
       "udpsrc port=" + port2.str() + " ! rtpbin.recv_rtcp_sink_0 \\ "
       "rtpbin.send_rtcp_src_0 ! udpsink port=" + port3.str() + " host=localhost sync=false async=false";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);
    make_verbose();

#if 0
    //  Create receiver pipeline
    GstElement *rtpbin, *udpSrc1, *udpSrc2, *depayloader, *decoder, *aConv, *aResample, *aSink, 
               *udpSink;
    GstCaps *caps;
    GstPad *recv_rtp_sink, *recv_rtcp_sink, *send_rtcp_src, *tempPad;

    pipeline_ = gst_pipeline_new("pipeline_");
    assert(pipeline_);

    make_verbose();
    
/*----------------------------------------------*/ 
// create elements
/*----------------------------------------------*/ 
    
    rtpbin = gst_element_factory_make("gstrtpbin", "rtpbin");
    assert(rtpbin);

    // pads from rtpbin
    
    recv_rtp_sink = gst_element_get_request_pad(rtpbin, "recv_rtp_sink_0"); 
    assert(recv_rtp_sink);
    recv_rtcp_sink = gst_element_get_request_pad(rtpbin, "recv_rtcp_sink_0"); 
    assert(recv_rtcp_sink);
    send_rtcp_src = gst_element_get_request_pad(rtpbin, "send_rtcp_src_0"); 
    assert(send_rtcp_src);

    udpSrc1 = gst_element_factory_make("udpsrc", "udpSrc1");
    assert(udpSrc1);
    

    caps = gst_caps_from_string(capsString);
    assert(caps);
    g_object_set(G_OBJECT(udpSrc1), "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(G_OBJECT(udpSrc1), "port", port_, NULL);

    depayloader = gst_element_factory_make("rtppcmadepay", "depayloader");
    assert(depayloader);

    decoder = gst_element_factory_make("alawdec", "decoder");
    assert(decoder);

    aConv = gst_element_factory_make("audioconvert", "aConv");
    assert(aConv);
    
    aResample = gst_element_factory_make("audioresample", "aResample");
    assert(aResample);

    aSink = gst_element_factory_make("alsasink", "aSink");
    assert(aSink);

    udpSrc2 = gst_element_factory_make("udpsrc", "udpSrc2");
    assert(udpSrc2);
    g_object_set(G_OBJECT(udpSrc1), "port", port_ + 1, NULL);

    udpSink = gst_element_factory_make("udpsink", "udpSink");
    assert(udpSink);
    // FIXME: host ip should be a private member
    g_object_set(G_OBJECT(udpSink), "port", port_ + 5, "host", "localhost", "sync", FALSE, "async", 
            FALSE, NULL);

    gst_bin_add_many(GST_BIN(pipeline_), rtpbin, udpSrc1, depayloader, decoder, aConv, aResample, aSink,
            udpSrc2, udpSink, NULL); 

    gst_element_link_many(depayloader, decoder, aConv, aResample, aSink, NULL);
   
    // link rtpbin pads
    tempPad = gst_element_get_pad(udpSrc1, "src");
    assert(tempPad);
    gst_pad_link(tempPad, recv_rtp_sink);
    gst_object_unref(GST_OBJECT(tempPad));

    tempPad = gst_element_get_pad(udpSrc2, "src");
    assert(tempPad);
    gst_pad_link(tempPad, recv_rtcp_sink);
    gst_object_unref(GST_OBJECT(tempPad));

    tempPad = gst_element_get_pad(udpSink, "sink");
    assert(tempPad);
    gst_pad_link(send_rtcp_src, tempPad);
    gst_object_unref(GST_OBJECT(tempPad));

    std::cout << "Receiving media on port : " << port_ << std::endl;

    // release requested pads in reverse order
    gst_element_release_request_pad(rtpbin, send_rtcp_src);
    gst_element_release_request_pad(rtpbin, recv_rtcp_sink);
    gst_element_release_request_pad(rtpbin, recv_rtp_sink);

#endif
    return true;
}
#endif



bool AudioReceiver::start()
{
    wait_for_caps();
    std::cout << "Receiving audio on port " << config_.port() << std::endl;
    MediaBase::start();
    return true;
}

