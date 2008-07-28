
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
#include <string>
#include <stdlib.h>
#include <cassert>
#include <gst/gst.h>

#include "lo/lo.h"

#include "logWriter.h"
#include "mediaBase.h"
#include "audioReceiver.h"
#include "audioConfig.h"
#include "jackUtils.h"

AudioReceiver::AudioReceiver(const AudioConfig & config)
    : config_(config),session_(),gotCaps_(false), depayloader_(0), decoder_(0), sink_(0)
{
    // empty
}

AudioReceiver::~AudioReceiver()
{
    if (isPlaying())
        assert(stop());
    pipeline_.remove(sink_);
    pipeline_.remove(decoder_);
    pipeline_.remove(depayloader_);
}

// FIXME: waiting shouldn't happen here, have one msg waiting loop which takes advantage of
// path to dispatch messages appropriately
void AudioReceiver::wait_for_caps()
{
    LOG("Waiting for caps...");
    lo_server_thread st = lo_server_thread_new("7770", liblo_error);

    lo_server_thread_add_method(st, "/audio/rx/caps", "s", caps_handler, (void *) this);

    lo_server_thread_start(st);

    while (!gotCaps_)
        usleep(10000);

    lo_server_thread_free(st);
}

void AudioReceiver::liblo_error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}

int AudioReceiver::caps_handler(const char *path, const char *types, lo_arg ** argv, int argc,
                                void *data,
                                void *user_data)
{
    AudioReceiver *context = static_cast < AudioReceiver * >(user_data);
    context->session_.set_caps(&argv[0]->s);
    context->gotCaps_ = true;

    return 0;
}

void AudioReceiver::init_codec()
{
    depayloader_ = gst_element_factory_make("rtpvorbisdepay", NULL);
    assert(depayloader_);

    pipeline_.add(depayloader_);

    decoder_ = gst_element_factory_make(config_.codec(), NULL);
    assert(decoder_);

    pipeline_.add(decoder_);

    assert(gst_element_link(depayloader_, decoder_));

    session_.add(depayloader_, &config_);
}

void AudioReceiver::init_sink()
{
    assert(jack_is_running());
    sink_ = gst_element_factory_make("jackaudiosink", NULL);
    assert(sink_);
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);

    pipeline_.add(sink_);

    assert(gst_element_link(decoder_, sink_));
}

bool AudioReceiver::start()
{
    // FIXME: caps are only sent if sender is started after
    wait_for_caps();
    std::cout << "Receiving audio on port " << config_.port() << std::endl;
    MediaBase::start();
    return true;
}

