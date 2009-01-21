/* exports.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
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

/** \file
 * This file exposes class to python 
 *
 * Exposes object modules to python interpreter, available by calling "from libmilhouse import *".
 *
 */

#include <iostream>
#include <Python.h>
#include <boost/python.hpp>
#include <gst/videoFactory.h>
#include <gst/audioFactory.h>
#include "util.h"
#include "gutil.h"


using namespace boost::python;


/// BOOST PYTHON MOD
BOOST_PYTHON_MODULE(libmilhouse)
{
    class_< VideoReceiver, boost::noncopyable, boost::shared_ptr<VideoReceiver> >("VideoReceiver", no_init);
    class_< VideoSender, boost::noncopyable, boost::shared_ptr<VideoSender> >("VideoSender", no_init)
        .def("getCaps", &VideoSender::getCaps);     // methods
        
    def("buildVideoReceiver", videofactory::buildVideoReceiver);
    def("buildVideoSender", videofactory::buildVideoSender);

    class_< AudioReceiver, boost::noncopyable, boost::shared_ptr<AudioReceiver> >("AudioReceiver", no_init);
    class_< AudioSender, boost::noncopyable, boost::shared_ptr<AudioSender> >("AudioSender", no_init)
        .def("getCaps", &AudioSender::getCaps);     // methods
        

    def("buildAudioReceiver", audiofactory::buildAudioReceiver);
    def("buildAudioSender", audiofactory::buildAudioSender);

    class_< VideoSourceConfig >("VideoSourceConfig", init<std::string>()) 
        .def(init<std::string, std::string>()); // overloaded constructor

    class_< AudioSourceConfig >("AudioSourceConfig", init<std::string, int>()) 
        .def(init<std::string, std::string, int>()); // overloaded constructor

    def("tcpSendBuffer", tcpSendBuffer);        

    def("start", playback::start);
    def("stop", playback::stop);
    def("isPlaying", playback::isPlaying);

    def("eventLoop", gutil::runMainLoop);
    def("setHandler", set_handler);

    boost::python::scope().attr("VIDEO_CAPS_PORT") = ports::VIDEO_CAPS_PORT;
    boost::python::scope().attr("AUDIO_CAPS_PORT") = ports::AUDIO_CAPS_PORT;

    boost::python::scope().attr("VIDEO_MSG_ID") = videofactory::MSG_ID;
    boost::python::scope().attr("AUDIO_MSG_ID") = audiofactory::MSG_ID;

    // expose macro to python
    boost::python::scope().attr("PACKAGE_VERSION") = PACKAGE_VERSION;
    boost::python::scope().attr("RELEASE_CANDIDATE") = RELEASE_CANDIDATE;
}

