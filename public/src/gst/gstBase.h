
// gstBase.h
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

#ifndef _GST_BASE_H_
#define _GST_BASE_H_

#include <vector>

class _GstElement;
class Pipeline;

typedef std::vector<_GstElement *>::iterator GstIter;

class GstBase
{
    public:
        virtual bool start();
        virtual bool stop();
        virtual bool init() = 0;

        bool isPlaying();

    protected:

        // this initializes pipeline only once/process
        GstBase();

        virtual ~GstBase();

        Pipeline & pipeline_;

    private:
        GstBase(const GstBase&);     //No Copy Constructor
        GstBase& operator=(const GstBase&);     //No Assignment Operator
        static int refCount_;
};

#endif // _GST_BASE_H_

