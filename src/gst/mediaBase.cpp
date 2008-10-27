
// mediaBase.cpp
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

#include <cassert>
#include "mediaBase.h"

LocalBase::~LocalBase()
{
    stop();
}


void LocalBase::init()  // template method
{
    // these methods are defined in subclasses
    init_source();
    init_sink();
}

SenderBase::~SenderBase()
{
    stop();
}


void SenderBase::init()  // template method
{
    // these methods are defined in subclasses
    init_source();
    init_codec();
    init_payloader();
}

ReceiverBase::~ReceiverBase()
{
    stop();
}


void ReceiverBase::init()  // template method
{
    // these methods are defined in subclasses
    init_codec();
    init_depayloader();
    init_sink();
}

