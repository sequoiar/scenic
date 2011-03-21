/* RTSPClient.h
 * Copyright (C) 2011 Société des arts technologiques (SAT)
 * Copyright (C) 2011 Tristan Matthews
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
#ifndef _RTSP_CLIENT_H_
#define _RTSP_CLIENT_H_

#include "gst/busMsgHandler.h"
#include <string>
#include <tr1/memory>

class _GstElement;
class _GstMessage;
class _GstBus;
class _GParamSpec;
class _GstPad;
class _GstCaps;
class Pipeline;

namespace boost {
    namespace program_options {
        class variables_map;
    }
}

class RTSPClient : private BusMsgHandler
{
    public:
        RTSPClient(const boost::program_options::variables_map &options, bool enableVideo, bool enableAudio);
        ~RTSPClient();
        void run(int timeout);
    private:
        bool handleBusMsg(_GstMessage *msg);
        bool validPortRange(const std::string &ports);
        void linkNewPad(_GstPad *pad, const _GstCaps *caps, const char *queue_name);
        static int timeout();
        static int onNotifySource(_GstElement *uridecodebin, _GParamSpec * /*pspec*/, void *data);
        static void onPadAdded(_GstElement *uridecodebin, _GstPad * newPad, void *data);
        std::tr1::shared_ptr<Pipeline> pipeline_;
        bool latencySet_;
        std::string portRange_;
        int latency_;
        bool enableVideo_;
        bool enableAudio_;
};

#endif // _RTSP_CLIENT_H_

