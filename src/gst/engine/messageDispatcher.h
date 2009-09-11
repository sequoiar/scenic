
// messageDispatcher.h
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

#ifndef _MESSAGE_DISPATCHER_H_
#define _MESSAGE_DISPATCHER_H_

#include <vector>

class MessageHandler;

class MessageDispatcher 
{
    public:
        /// This is the single point of access to the singleton instance of this messagedispatcher
        static MessageDispatcher * getInstance();
        void subscribe(MessageHandler *obj);
        void unsubscribe(MessageHandler *obj);
        void sendMessage(const std::string &message);    // could also overload with a version that has an argument map

    private:
        void updateHandlers(const std::string &message);
        std::vector<MessageHandler*> handlers_;
        
        MessageDispatcher() : handlers_() {}

        ~MessageDispatcher(){};
        static MessageDispatcher *instance_;
};

#endif // _MESSAGE_DISPATCHER_H_ 
