
// syncTester.h
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

#ifndef _SYNC_TESTER_H_
#define _SYNC_TESTER_H_

#include "videoSender.h"
#include "audioSender.h"
#include "videoReceiver.h"
#include "audioReceiver.h"
#include "videoConfig.h"
#include "audioConfig.h"

class SyncTester 
{
public:
    virtual void run() = 0;
    virtual ~SyncTester(){}
protected:
    SyncTester() {};
    static const int V_PORT;
    static const int A_PORT;
    static const int NUM_CHANNELS;
};

class SyncTesterSend : public SyncTester
{
    public:
        virtual void run();
};

class SyncTesterReceive : public SyncTester
{
    public:
        virtual void run();
};

#endif // _SYNC_TESTER_H_
