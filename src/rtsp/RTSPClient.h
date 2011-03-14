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

class _GstElement;
class _GstMessage;
class _GstBus;

namespace boost {
    namespace program_options {
        class variables_map;
    }
}

class RTSPClient
{
    public:
        RTSPClient(const boost::program_options::variables_map &options, bool enableVideo, bool enableAudio);
        void run(int timeout);
    private:
        static int busCall(_GstBus * /*bus*/, _GstMessage *msg, void *user_data);
        static int timeout(RTSPClient *client);
        _GstElement *rtpbin_;
        _GstElement *pipeline_;
        bool latencySet_;
};

#endif // _RTSP_CLIENT_H_

