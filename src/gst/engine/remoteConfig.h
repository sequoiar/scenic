
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
#include "../ports.h"

class Encoder;
class VideoEncoder;
class VideoDecoder;
class Decoder;

/** 
 *      Immutable class that is used to setup rtp
 */

class RemoteConfig 
{
    public:
        RemoteConfig(const std::string &codec__, 
                const std::string &remoteHost__,
                int port__,
                int msgId__); 

        // copy constructor
        RemoteConfig(const RemoteConfig& m)
            : codec_(m.codec_), remoteHost_(m.remoteHost_), port_(m.port_), msgId_(m.msgId_) {}

        virtual ~RemoteConfig(){};

        int port() const { return port_; }
        int rtcpFirstPort() const { return port_ + ports::RTCP_FIRST_OFFSET; }
        int rtcpSecondPort() const { return port_ + ports::RTCP_SECOND_OFFSET; }
        int capsPort() const { return port_ + ports::CAPS_OFFSET; }
        const char *remoteHost() const { return remoteHost_.c_str(); }
        bool hasCodec() const { return !codec_.empty(); }
        std::string codec() const { return codec_; }
        void checkPorts() const;
        void cleanupPorts() const;

    protected:

        const std::string codec_;
        const std::string remoteHost_;
        const int port_;
        const int msgId_;
        static const int PORT_MIN;
        static const int PORT_MAX;
        static std::set<int> usedPorts_;

    private:
        RemoteConfig& operator=(const RemoteConfig&); //No Assignment Operator
};

class SenderConfig : public RemoteConfig
{
    public:
        SenderConfig(const std::string &codec__,
                const std::string &remoteHost__,    
                int port__,
                int msgId__) : RemoteConfig(codec__, remoteHost__, port__, msgId__)
    {}

        SenderConfig(const SenderConfig & m) 
            : RemoteConfig(m)
        {}

        VideoEncoder* createVideoEncoder() const;
        Encoder* createAudioEncoder() const;

        void sendMessage(const std::string &msg);
};


class ReceiverConfig : public RemoteConfig
{
    public:
        ReceiverConfig(const std::string &codec__,
                const std::string &remoteHost__,    
                int port__, 
                const std::string &multicastInterface__,
                const std::string &caps__,
                int msgId__) : RemoteConfig(codec__, remoteHost__, port__, msgId__), 
        multicastInterface_(multicastInterface__), caps_(caps__)
    {}

        ReceiverConfig(const ReceiverConfig & m) 
            : RemoteConfig(m), multicastInterface_(m.multicastInterface_), caps_(m.caps_)
        {}

        VideoDecoder* createVideoDecoder() const;
        Decoder* createAudioDecoder() const;

        const char *multicastInterface() const { return multicastInterface_.c_str(); }
        const char *caps() const { return caps_.c_str(); }
        bool capsMatchCodec() const;
        bool hasMulticastInterface() const { return multicastInterface_ != ""; }
        void receiveCaps();

    private:
        const std::string multicastInterface_;
        std::string caps_;
};

#endif // _REMOTE_CONFIG_H_

