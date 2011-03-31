
/* audio_config.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _AUDIO_LOCAL_CONFIG_H_
#define _AUDIO_LOCAL_CONFIG_H_

#include <string>
#include <gdk/gdktypes.h> // for GdkNativeWindow

// forward declarations
class Pipeline;
class AudioSource;
class AudioLevel;
class AudioSink;

namespace boost
{
    namespace program_options
    {
        class variables_map;
    }
}

/// Immutable class that is used to parameterize AudioSender objects.
class AudioSourceConfig
{
    public:

        AudioSourceConfig(const boost::program_options::variables_map &options);
        static int maxChannels(const std::string &codec);

        const char *source() const;

        int numChannels() const;

        bool hasDeviceName() const { return !deviceName_.empty(); }
        bool hasLocation() const { return !location_.empty(); }

        double quality() const;
        int bitrate() const;
        const char *deviceName() const;
        const char *location() const;

        bool locationExists() const;

        AudioSource* createSource(Pipeline &pipeline) const;
        AudioLevel* createLevel(Pipeline &pipeline, const std::string &title) const;
        unsigned long long bufferTime() const;
        std::string sourceString() const { return source_; }
        const char* sourceName() const;

    private:
        const std::string source_;
        const int bitrate_;
        const double quality_;
        const std::string sourceName_;
        const std::string deviceName_;
        const std::string location_;
        const int numChannels_;
        const unsigned long long bufferTime_;
        GdkNativeWindow socketID_;
        const bool disableAutoConnect_;
};

///  Immutable class that is used to parametrize AudioReceiver objects.
class AudioSinkConfig
{
    public:
        AudioSinkConfig(const boost::program_options::variables_map &options);

        AudioLevel* createLevel(Pipeline &pipeline, const std::string &title) const;
        AudioSink* createSink(Pipeline &pipeline) const;
        bool hasDeviceName() const { return !deviceName_.empty(); }
        const char *sinkName() const;
        const char *deviceName() const;
        int numChannels() const;
        unsigned long long bufferTime() const;

    private:
        const std::string sink_;
        const std::string sinkName_;
        const std::string deviceName_;
        const unsigned long long bufferTime_;
        GdkNativeWindow socketID_;
        const int numChannels_;
        const bool disableAutoConnect_;
};

#endif // _AUDIO_LOCAL_CONFIG_H_

