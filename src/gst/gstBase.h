
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

#include "pipeline.h"

class GstBase
{
    public:
        virtual bool start();
        virtual bool stop();

        bool isPlaying() { return pipeline_.isPlaying(); }

    protected:
        typedef std::vector<GstElement *>::iterator GstIter;

        GstBase();
        virtual ~GstBase();

        Pipeline & pipeline_;

    private:
        //static int refCount_;
};

#endif // _GST_BASE_H_

