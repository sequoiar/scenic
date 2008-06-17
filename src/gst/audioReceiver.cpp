
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

#include "mediaBase.h"
#include "audioReceiver.h"
#include "audioConfig.h"

AudioReceiver::AudioReceiver() : config_(0)
{
    // empty
}




AudioReceiver::AudioReceiver(const AudioConfig& config) : config_(config)
{
   // empty 
}



AudioReceiver::~AudioReceiver() 
{
    // empty
}




bool AudioReceiver::init()
{
    //  Create receiver pipeline
    GstElement *rxSrc, *depayloader, *decoder, *rxSink;
    GstCaps *caps;

    pipeline_ = gst_pipeline_new("rxPipeline");
    assert(pipeline_);

    make_verbose();

    rxSrc = gst_element_factory_make("udpsrc", "rxSrc");
    assert(rxSrc);
    
    // FIXME: caps shouldn't be hardcoded
    if (config_.numChannels() == 2)
        caps = gst_caps_from_string(AudioReceiver::CAPS_STR[0].c_str());
    else if (config_.numChannels() == 8)
        caps = gst_caps_from_string(AudioReceiver::CAPS_STR[1].c_str());

    assert(caps);
    g_object_set(G_OBJECT(rxSrc), "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(G_OBJECT(rxSrc), "port", config_.port(), NULL);

    depayloader = gst_element_factory_make("rtpvorbisdepay", "depayloader");
    assert(depayloader);

    decoder = gst_element_factory_make("vorbisdec", "decoder");
    assert(decoder);

    rxSink = gst_element_factory_make("jackaudiosink", "rxSink");
    assert(rxSink);
    g_object_set(G_OBJECT(rxSink), "sync", FALSE, NULL);

    gst_bin_add_many(GST_BIN(pipeline_), rxSrc, depayloader, 
            decoder, rxSink, NULL); 

    std::cout << "Receiving media on port : " << config_.port() << std::endl;
    gst_element_link_many(rxSrc, depayloader, decoder, rxSink, NULL);

    return true;
}



#if 0
bool AudioReceiver::init(int port, int numChannels)
{
    numChannels_ = numChannels;

    if (port < 1000)
        port_ = DEF_PORT;
    else
        port_ = port;

    //  Create receiver pipeline
    GstElement *rxSrc, *depayloader, *decoder, *rxSink;
    GstCaps *caps;

    pipeline_ = gst_pipeline_new("rxPipeline");
    assert(pipeline_);

    make_verbose();

    rxSrc = gst_element_factory_make("udpsrc", "rxSrc");
    assert(rxSrc);
    
    // FIXME: caps shouldn't be hardcoded
    if (numChannels_ == 2)
        caps = gst_caps_from_string(AudioReceiver::CAPS_STR[0].c_str());
    else if (numChannels_ == 8)
        caps = gst_caps_from_string(AudioReceiver::CAPS_STR[1].c_str());

    assert(caps);
    g_object_set(G_OBJECT(rxSrc), "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(G_OBJECT(rxSrc), "port", port_, NULL);

    depayloader = gst_element_factory_make("rtpvorbisdepay", "depayloader");
    assert(depayloader);

    decoder = gst_element_factory_make("vorbisdec", "decoder");
    assert(decoder);

    rxSink = gst_element_factory_make("jackaudiosink", "rxSink");
    assert(rxSink);
    g_object_set(G_OBJECT(rxSink), "sync", FALSE, NULL);

    gst_bin_add_many(GST_BIN(pipeline_), rxSrc, depayloader, 
            decoder, rxSink, NULL); 

    std::cout << "Receiving media on port : " << port_ << std::endl;
    gst_element_link_many(rxSrc, depayloader, decoder, rxSink, NULL);

    return true;
}
#endif



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
    std::cout << "Receiving audio on port " << config_.port() << std::endl;
    MediaBase::start();
    return true;
}

// FIXME: Shouldn't hardcode this string, should get it from sender somehow.

const std::string AudioReceiver::CAPS_STR[2] = 
{
    // 2ch
"application/x-rtp, media=(string)audio, clock-rate=(int)44100, encoding-name=(string)VORBIS, encoding-params=(string)2, configuration=(string)\"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\\=\\=\", delivery-method=(string)inline, payload=(int)96, ssrc=(guint)509063580, clock-base=(guint)2636551156, seqnum-base=(guint)8663"
    ,
    // 8ch
"application/x-rtp, media=(string)audio, clock-rate=(int)44100, encoding-name=(string)VORBIS, encoding-params=(string)8, configuration=(string)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, delivery-method=(string)inline, payload=(int)96, ssrc=(guint)9048990, clock-base=(guint)626026394, seqnum-base=(guint)41905"
};
