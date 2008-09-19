
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

#include <sstream>
#include <cassert>

//#ifdef USE_OSC
#include "lo/lo.h"
#include "logWriter.h"
//#endif

#include "gstLinkable.h"
#include "audioReceiver.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "rtpPay.h"
#include "codec.h"
#include "audioSink.h"


AudioReceiver::~AudioReceiver()
{
    assert(stop());
    delete sink_;
    delete decoder_;
    delete depayloader_;
}


// FIXME: get rid of this!!!!
//#ifdef USE_OSC
void AudioReceiver::wait_for_caps()
{
    LOG("Waiting for caps...", DEBUG);
    lo_server_thread st = lo_server_thread_new("7770", liblo_error);

    lo_server_thread_add_method(st, "/audio/rx/caps", "s", caps_handler,
            static_cast<void *>(this));

    lo_server_thread_start(st);

    while (!gotCaps_)
        usleep(10000);

    lo_server_thread_free(st);
}


//#endif


// FIXME: get rid of this!!!!
void AudioReceiver::liblo_error(int num, const char *msg, const char *path)
{
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stdout);
}


// FIXME: get rid of this!!!!
int AudioReceiver::caps_handler(const char * /*path*/, const char * /*types*/, lo_arg ** argv,
        int /*argc*/, void * /*data*/,
        void *user_data)
{
    AudioReceiver *context = static_cast < AudioReceiver * >(user_data);
    context->session_.set_caps(&argv[0]->s);
    context->gotCaps_ = true;

    return 0;
}


void AudioReceiver::init_codec()
{
    assert(decoder_ = remoteConfig_.createDecoder());
    decoder_->init();
}


void AudioReceiver::init_depayloader()
{
    assert(depayloader_ = decoder_->createDepayloader());
    depayloader_->init();
    GstLinkable::link(*depayloader_, *decoder_);
    session_.add(depayloader_, remoteConfig_);
}


void AudioReceiver::init_sink()
{
    assert(sink_ = audioConfig_.createSink());
    sink_->init();
    GstLinkable::link(*decoder_, *sink_);   
}


bool AudioReceiver::start()
{
    // FIXME: caps are only sent if sender is started after receiver
    //#ifdef USE_OSC
//    wait_for_caps();
    //#endif
    std::stringstream logstr;       // FIXME: need a better printf style logwriter, 
                                    // shouldn't need stringstream
    logstr << "Receiving audio on port " << remoteConfig_.port() << " from host " << remoteConfig_.remoteHost();
    LOG(logstr.str(), DEBUG); 
    GstBase::start();
    return true;
}

