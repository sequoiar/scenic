
// mediaBase.h
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

#ifndef _MEDIA_BASE_H_
#define _MEDIA_BASE_H_

#include "gstBase.h"

class MediaBase     // local bins
    : public GstBase
{
    public:
        virtual void init();

    protected:

        MediaBase(){};
        ~MediaBase();
        virtual void init_source() = 0;
        virtual void init_sink() = 0;

    private:

        MediaBase(const MediaBase&);     //No Copy Constructor
        MediaBase& operator=(const MediaBase&);     //No Assignment Operator
};

class SenderBase    
    : public GstBase
{
    public: 
        virtual void init();
        virtual double bandwidth() const = 0;

    protected:

        SenderBase(){};
        ~SenderBase();
        virtual void init_source() = 0;
        virtual void init_codec() = 0;
        virtual void init_payloader() = 0;
    
    private:

        SenderBase(const SenderBase&);     //No Copy Constructor
        SenderBase& operator=(const SenderBase&);     //No Assignment Operator
};

class ReceiverBase
    : public GstBase
{
    public: 
        virtual void init();
        virtual double bandwidth() const = 0;

    protected:

        ReceiverBase(){};
        ~ReceiverBase();
        virtual void init_codec() = 0;
        virtual void init_depayloader() = 0;
        virtual void init_sink() = 0;
    
    private:

        ReceiverBase(const ReceiverBase&);     //No Copy Constructor
        ReceiverBase& operator=(const ReceiverBase&);     //No Assignment Operator
};

#endif // _MEDIA_BASE_H_

