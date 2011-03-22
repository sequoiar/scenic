
/* milhouse.h
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

#ifndef _MILHOUSE_H_
#define _MILHOUSE_H_

// forward declarations
namespace boost {
    namespace program_options {
        class options_description;
        class variables_map;
    }
}

class Milhouse {
    private:
        short usage(const boost::program_options::options_description &desc);
        boost::program_options::options_description createOptionsDescription();
        void runAsRTSPClient(const boost::program_options::variables_map &options);
        void runAsRTSPServer(const boost::program_options::variables_map &options);
        void runAsReceiver(const boost::program_options::variables_map &options, bool enableVideo, bool enableAudio);
        void runAsSender(const boost::program_options::variables_map &options, bool enableVideo, bool enableAudio);
        void runAsLocal(const boost::program_options::variables_map &options, bool enableVideo, bool enableAudio);

    public:
        short run(int argc, char **argv);
};

#endif // _MILHOUSE_H_
