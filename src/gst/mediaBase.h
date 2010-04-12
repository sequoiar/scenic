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


#include <boost/shared_ptr.hpp>

class Pipeline;
class _GstMessage;
class SenderConfig;

class SenderBase 
{
    public: 
        SenderBase(boost::shared_ptr<SenderConfig> rConfig);
        virtual ~SenderBase();
        bool capsAreCached() { return checkCaps(); }

    protected:
        boost::shared_ptr<SenderConfig> remoteConfig_;
        void createPipeline(Pipeline &pipeline);

    private:
        virtual bool checkCaps() const = 0;
        virtual void createSource(Pipeline &pipeline) = 0;
        virtual void createCodec(Pipeline &pipeline) = 0;
        virtual void createPayloader() = 0;
};

class ReceiverBase 
{
    public: 
        ReceiverBase(){};
        virtual ~ReceiverBase(){};
    protected:
        void createPipeline(Pipeline &pipeline);

    private:
        virtual void createCodec(Pipeline &pipeline) = 0;
        virtual void createDepayloader() = 0;
        virtual void createSink(Pipeline &pipeline) = 0;
};

#endif // _MEDIA_BASE_H_

