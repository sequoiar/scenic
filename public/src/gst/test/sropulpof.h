
// sropulpof.h
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
// // You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _SROPULPOF_H_
#define _SROPULPOF_H_

class Pof 
{
    public:
        Pof(char pid, const char *ip, const char *videoCodec, const char *audioCodec, long videoPort, long audioPort, bool isFullscreen);
        ~Pof(){};
        short run();
        static const short NUM_CHANNELS;

    private:
        char pid_;
        const char *ip_;
        const char *videoCodec_;
        const char *audioCodec_;
        const long videoPort_;
        const long audioPort_;
        bool isFullscreen_;
        Pof(const Pof&);     //No Copy Constructor
        Pof& operator=(const Pof&);     //No Assignment Operator
};

#endif // _SROPULPOF_H_

