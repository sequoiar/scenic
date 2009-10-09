
// messageHandler.cpp
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
// Abstract interface that for handling messages from the gst bus

#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_

/** 
* Abstract interface which requires its implementors to provide 
* functionality to handle messages sent to it
*/

#include <string>

class MessageHandler
{
    public:
        MessageHandler();
        virtual ~MessageHandler();
        virtual bool handleMessage(const std::string &path, 
                const std::string &arguments) = 0;
};

#endif // _MESSAGE_HANDLER_H_ 

