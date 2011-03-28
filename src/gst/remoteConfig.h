
// remoteConfig.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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

#ifndef _REMOTE_CONFIG_H_
#define _REMOTE_CONFIG_H_

#include <string>
#include <set>
#include <boost/scoped_ptr.hpp>
#include "portOffsets.h"

#include "busMsgHandler.h"

class Encoder;
class Pipeline;
class VideoEncoder;
class VideoDecoder;
class Decoder;
class CapsServer;

/** 
 *      Immutable class that is used to setup rtp
 */

class RemoteConfig 
{
    public:
        RemoteConfig(const std::string &codec, 
                const std::string &remoteHost, 
                int port);
        
        static bool capsMatchCodec(const std::string &encodingName, const std::string &codec);

        int port() const { return port_; }
        int rtcpFirstPort() const { return port_ + ports::RTCP_FIRST_OFFSET; }
        int rtcpSecondPort() const { return port_ + ports::RTCP_SECOND_OFFSET; }
        int capsPort() const { return port_ + ports::CAPS_OFFSET; }
        const char *remoteHost() const { return remoteHost_.c_str(); }
        bool hasCodec() const { return !codec_.empty(); }
        std::string codec() const { return codec_; }
        void checkPorts() const;
        void cleanupPorts() const;
        std::string identifier() const;

    protected:

        const std::string codec_;
        const std::string remoteHost_;
        const int port_;
        static const int PORT_MIN;
        static const int PORT_MAX;
        static std::set<int> usedPorts_;

    private:
        std::string codecMediaType() const;
};

class SenderConfig : public RemoteConfig, private BusMsgHandler
{
    public:
        SenderConfig(Pipeline &pipeline,
                const std::string &codec, 
                const std::string &remoteHost, 
                int port,
                const std::string &multicastInterface);

        VideoEncoder* createVideoEncoder(const Pipeline &pipeline, int bitrate, int quality) const;
        Encoder* createAudioEncoder(const Pipeline &pipeline, int bitrate, double quality) const;

    private:
        void sendCaps();

        std::string message_;
        bool handleBusMsg(_GstMessage *msg);
        boost::scoped_ptr<CapsServer> capsServer_;
        const std::string multicastInterface_;
};


class ReceiverConfig : public RemoteConfig
{
    public:
        ReceiverConfig(const std::string &codec, 
                const std::string &remoteHost, 
                int port,
                const std::string &multicastInterface,
                bool enableControls);

        VideoDecoder* createVideoDecoder(const Pipeline &pipeline, bool doDeinterlace) const;
        Decoder* createAudioDecoder(const Pipeline &pipeline, int numChannels) const;

        const char *multicastInterface() const { return multicastInterface_.c_str(); }
        const char *caps() const { return caps_.c_str(); }
        bool capsMatchCodec() const;
        bool hasMulticastInterface() const { return multicastInterface_ != ""; }
        // This method will block while waiting for caps
        void receiveCaps();
        bool jitterbufferControlEnabled() const { return jitterbufferControlEnabled_; }

    private:
        static bool isSupportedCodec(const std::string &codec);
        const std::string multicastInterface_;
        std::string caps_;
        bool jitterbufferControlEnabled_;
};

#endif // _REMOTE_CONFIG_H_

