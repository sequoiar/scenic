// mediaBase.h
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

#ifndef _MEDIA_BASE_H_
#define _MEDIA_BASE_H_


class LocalBase     // local bins
{
    public:
        virtual void init();
        LocalBase() : initialized_(false) {};
        virtual ~LocalBase(){};

    private:
        virtual void init_source() = 0;
        virtual void init_sink() = 0;
        bool initialized_;

        LocalBase(const LocalBase&);     //No Copy Constructor
        LocalBase& operator=(const LocalBase&);     //No Assignment Operator
};


class SenderBase 
{
    public: 
        void init();
        SenderBase() : initialized_(false) {};
        virtual ~SenderBase(){};

        virtual std::string getCaps() const = 0;
    private:
        virtual void init_source() = 0;
        virtual void init_codec() = 0;
        virtual void init_payloader() = 0;
        bool initialized_;

        SenderBase(const SenderBase&);     //No Copy Constructor
        SenderBase& operator=(const SenderBase&);     //No Assignment Operator
};

class ReceiverBase 
{
    public: 
        void init();
        ReceiverBase() : initialized_(false) {};
        virtual ~ReceiverBase(){};

    private:
        virtual void init_codec() = 0;
        virtual void init_depayloader() = 0;
        virtual void init_sink() = 0;
        bool initialized_;

        ReceiverBase(const ReceiverBase&);     //No Copy Constructor
        ReceiverBase& operator=(const ReceiverBase&);     //No Assignment Operator
};

#endif // _MEDIA_BASE_H_

