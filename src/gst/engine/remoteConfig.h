
// remoteConfig.h
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

/** \file
 *      Immutable class that is used to setup rtp using objects.
 *
 */

#ifndef _REMOTE_CONFIG_H_
#define _REMOTE_CONFIG_H_

#include <string>

class Encoder;
class Decoder;
const int NUM_CODECS = 4;

class RemoteConfig 
{
    public:
        RemoteConfig(const std::string &codec__, 
                const std::string &remoteHost__,
                int port__); 

        // copy constructor
        RemoteConfig(const RemoteConfig& m)
            : codec_(m.codec_), remoteHost_(m.remoteHost_), port_(m.port_) {}

        virtual ~RemoteConfig() {}

        int port() const { return port_; }
        const char *remoteHost() const { return remoteHost_.c_str(); }
        bool hasCodec() const { return !codec_.empty(); }

    protected:

        const std::string codec_;
        const std::string remoteHost_;
        const int port_;
        static const int PORT_MIN;
        static const int PORT_MAX;
        static const std::string VALID_CODECS[NUM_CODECS];

    private:
        RemoteConfig& operator=(const RemoteConfig&); //No Assignment Operator
};

class SenderConfig : public RemoteConfig
{
    public:
        SenderConfig(const std::string &codec__,
                const std::string &remoteHost__,    
                int port__) : RemoteConfig(codec__, remoteHost__, port__)
    {}

        SenderConfig(const SenderConfig & m) 
            : RemoteConfig(m)
        {}

        Encoder* createEncoder() const;
};


class ReceiverConfig : public RemoteConfig
{
    public:
        ReceiverConfig(const std::string &codec__,
                const std::string &remoteHost__,    
                int port__, const std::string &caps__) : RemoteConfig(codec__, remoteHost__, port__), caps_(caps__)
    {}

        ReceiverConfig(const ReceiverConfig & m) 
            : RemoteConfig(m), caps_(m.caps_)
        {}

        Decoder* createDecoder() const;

        const char *caps() const { return caps_.c_str(); }

    private:
        const std::string caps_;
};

#endif // _REMOTE_CONFIG_H_

