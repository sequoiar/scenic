
// messageDispatcher.cpp
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

#include "util.h"
#include "messageDispatcher.h"
#include "messageHandler.h"
#include <algorithm>

MessageDispatcher *MessageDispatcher::instance_ = 0;


MessageDispatcher * MessageDispatcher::getInstance()
{
    if (instance_ == 0) {
        instance_ = new MessageDispatcher();
    }
    return instance_;
}


void MessageDispatcher::sendMessage(const std::string &message)
{
    updateHandlers(message);
}


void MessageDispatcher::updateHandlers(const std::string &message)
{
    std::vector<MessageHandler*>::iterator iter;
    for (iter = handlers_.begin(); iter != handlers_.end(); ++iter)
        if ((*iter)->handleMessage(message))    // one replied true
            break;
    if (iter == handlers_.end())
        LOG_WARNING("Message " << message << " was not handled");
}


void MessageDispatcher::subscribe(MessageHandler *obj)
{
    handlers_.push_back(obj);
}


void MessageDispatcher::unsubscribe(MessageHandler *obj)
{
    // remove the busmsghandler from the list

    // find the busmsghandler in the list
    std::vector<MessageHandler*>::iterator iter;
    iter = std::find(handlers_.begin(), handlers_.end(), obj);

    // assert that we were able to find the handler 
    assert(iter != handlers_.end() );

    // remove it
    handlers_.erase(iter);
}

