/* audioSink.cpp
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
#include <signal.h>
#include "audioSink.h"
#include "audioConfig.h"
#include "jackUtils.h"
#include "pipeline.h"
#include "alsa.h"

/// Constructor 
AudioSink::AudioSink(Pipeline &pipeline) : 
    pipeline_(pipeline),
    sink_(0)
{
    // attach floating point exception signal handler
    static bool signalHandlerAttached = false; // thread-safe in gcc
    if (not signalHandlerAttached)
    {
        signal(SIGFPE, (void (*)(int))FPE_ExceptionHandler);
        signalHandlerAttached = true;
    }
}

/// Destructor 
AudioSink::~AudioSink()
{
    pipeline_.remove(&sink_);
}


/// This is just declared here because if the buffer-time property for our sink is 
/// set to low it causes a system level floating point exception, that can only be
/// caught by a signal hander.
void AudioSink::FPE_ExceptionHandler(int /*nSig*/, int nErrType, int * /*pnReglist*/)
{
    static const std::string jackWarning(": this COULD be because the buffer-time of an audiosink is too low");
    switch(nErrType)
    {
        case FPE_INTDIV:  
            THROW_ERROR("Integer divide by zero" << jackWarning);
            break;
        case FPE_INTOVF:  
            THROW_ERROR("Integer overflow" << jackWarning);
            break;
        case FPE_FLTDIV:  
            THROW_ERROR("Floating-point divide by zero" << jackWarning);
            break;
        case FPE_FLTOVF:  
            THROW_ERROR("Floating-point overflow" << jackWarning);
            break;
        case FPE_FLTUND: 
            THROW_ERROR("Floating-point underflow" << jackWarning);
            break;
        case FPE_FLTRES:
            THROW_ERROR("Floating-point inexact result" << jackWarning);
            break;
        case FPE_FLTINV: 
            THROW_ERROR("Invalid floating-point operation" << jackWarning);
            break;
        case FPE_FLTSUB:
            THROW_ERROR("Subscript out of range" << jackWarning);
            break;
        default:
            THROW_ERROR("Got floating point exception" << jackWarning);
    }
}

void AudioSink::adjustBufferTime(unsigned long long bufferTime)
{
    g_object_set(sink_, "buffer-time", bufferTime, NULL);
    unsigned long long val;
    g_object_get(sink_, "buffer-time", &val, NULL);
    LOG_DEBUG("Buffer time is " << val);
}


/// Constructor 
AudioAlsaSink::AudioAlsaSink(Pipeline &pipeline, const AudioSinkConfig &config) : 
    AudioSink(pipeline),
    aconv_(pipeline_.makeElement("audioconvert", NULL)), 
    config_(config)
{
    if (Jack::is_running())
        THROW_CRITICAL("Jack is running, you must stop jack server before using alsasink");

    sink_ = pipeline_.makeElement("alsasink", NULL);

    g_object_set(G_OBJECT(sink_), "buffer-time", config_.bufferTime(), NULL);
    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(sink_), "device", config_.deviceName(), NULL);
    else
        g_object_set(G_OBJECT(sink_), "device", alsa::DEVICE_NAME, NULL);


    gstlinkable::link(aconv_, sink_);
}

/// Destructor
AudioAlsaSink::~AudioAlsaSink()
{
    pipeline_.remove(&aconv_);
}

/// Constructor 
AudioPulseSink::AudioPulseSink(Pipeline &pipeline, const AudioSinkConfig &config) : 
    AudioSink(pipeline),
    aconv_(pipeline_.makeElement("audioconvert", NULL)), 
    config_(config) 
{
    sink_ = pipeline_.makeElement("pulsesink", NULL);
    g_object_set(G_OBJECT(sink_), "buffer-time", config_.bufferTime(), NULL);
    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(sink_), "device", config_.deviceName(), NULL);
    else
        g_object_set(G_OBJECT(sink_), "device", alsa::DEVICE_NAME, NULL);

    gstlinkable::link(aconv_, sink_);
}

/// Destructor 
AudioPulseSink::~AudioPulseSink()
{
    pipeline_.remove(&aconv_);
}

/// Constructor 
AudioJackSink::AudioJackSink(Pipeline &pipeline, const AudioSinkConfig &config) :  
    AudioSink(pipeline),
    config_(config)
{
    sink_ = pipeline_.makeElement("jackaudiosink", config_.sinkName());

    // uncomment to turn off autoconnect
    //g_object_set(G_OBJECT(sink_), "connect", 0, NULL);
    // use auto-forced connect mode if available
    if (Jack::autoForcedSupported(sink_))
        g_object_set(G_OBJECT(sink_), "connect", 2, NULL);

    if (config_.bufferTime() < Jack::safeBufferTime())
    {
        LOG_WARNING("Buffer time " << config_.bufferTime() << " is too low, using " << Jack::safeBufferTime() << " instead");
        g_object_set(G_OBJECT(sink_), "buffer-time", Jack::safeBufferTime(), NULL);
    }
    else
        g_object_set(G_OBJECT(sink_), "buffer-time", config_.bufferTime(), NULL);

    unsigned long long val;
    g_object_get(sink_, "buffer-time", &val, NULL);
    LOG_DEBUG("Buffer time is " << val);
}

/// Destructor 
AudioJackSink::~AudioJackSink() 
{}


bool AudioJackSink::handleMessage(const std::string &path, const std::string &/*arguments*/)
{
    tassert(sink_);
    if (path == "disable-jack-autoconnect")
    {
        g_object_set(G_OBJECT(sink_), "connect", 0, NULL);
        return true;
    }
    return false;
}

