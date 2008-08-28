
// audioSender.cpp
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
//
// TODO: Client code should just call this with some kind of parameter object which
// specifies number of channels, how to compress it (if at all), and host and port info.

#include <cassert>
#include <gst/gst.h>

#ifdef USE_OSC
#include "lo/lo.h"
#include "logWriter.h"
#endif

#include "audioSender.h"
#include "audioSource.h"
#include "jackUtils.h"

AudioSender::AudioSender(const AudioConfig & config)
    : config_(config), session_(), source_(0), encoder_(0), payloader_(0), sink_(0)
{
    // empty
}


AudioSender::~AudioSender()
{
    assert(stop());
    pipeline_.remove(sink_);
    pipeline_.remove(payloader_);
    pipeline_.remove(encoder_);
    delete source_;
}


void AudioSender::init_source()
{
    assert(source_ = config_.createSource());
    source_->init();
}


void AudioSender::init_codec()
{
    if (config_.hasCodec()) {
        assert(encoder_ = gst_element_factory_make(config_.codec(), NULL));
        pipeline_.add(encoder_);
    }
}


void AudioSender::init_sink()
{
    if (config_.isNetworked()) {     // remote version
        assert(payloader_ = gst_element_factory_make("rtpvorbispay", NULL));
        pipeline_.add(payloader_);

        GstLinkable::link(*source_, encoder_);
        GstLinkable::link(encoder_, payloader_);

        session_.add(payloader_, &config_);
    }
    else {                       // local version
        assert(jack_is_running());
        assert(sink_ = gst_element_factory_make("jackaudiosink", NULL));
        g_object_set(G_OBJECT(sink_), "connect", 1, NULL); // turn on autoconnect
        g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL); // important for latency
        pipeline_.add(sink_);

        GstLinkable::link(*source_, sink_);
    }
}


void AudioSender::send_caps() const
{
#ifdef USE_OSC
    //CapsSender::send(session_.caps_str());
    
    // returns caps for last sink, needs to be sent to receiver for rtpvorbisdepay
    LOG("Sending caps...", DEBUG);

    lo_address t = lo_address_new(NULL, "7770");
    if (lo_send(t, "/audio/rx/caps", "s", session_.getCaps().c_str()) == -1)
        std::cerr << "OSC error " << lo_address_errno(t) << ": " << lo_address_errstr(t)
                  << std::endl;
#endif
}


bool AudioSender::start()
{
    MediaBase::start();

    if (config_.isNetworked())
    {
        std::cout << "Sending audio to host " << config_.remoteHost() << " on port "
                  << config_.port() << std::endl;

        pipeline_.wait_until_playing();
        send_caps();
    }
    return true;
}


