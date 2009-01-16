/* singleBuffer.h
 * Copyright 2008 Koya Charles & Tristan Matthews
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifndef _SINGLE_BUFFER_H_
#define _SINGLE_BUFFER_H_

#include "tcp/tcpThread.h"
#include "tcp/parser.h"

static std::string tcpGetBuffer(int port, int &id)
{
    TcpThread tcp(port);
    tcp.run();
    QueuePair& queue = tcp.getQueue();
    for(;;)
    {
        MapMsg f = queue.timed_pop(100000);
        if(f["command"].empty())
            continue;
        try
        {
            if(std::string(f["command"]) != "buffer")
            {
                LOG_INFO("Unknown msg.");
                continue;
            }
            id = f["id"];
            return f["str"];
        }
        catch(ErrorExcept)
        {
        }
    }
    return "";
}

#include <errno.h>

static bool tcpSendBuffer(const char *ip, int port, int id, const std::string &caps)
{
    MapMsg msg("buffer");

    TcpThread tcp(port);

    msg["str"] = caps;
    msg["id"] = id;

    const int MAX_TRIES = 100;

    for(int i = 0; i < MAX_TRIES; ++i)
    {
        try
        {
            bool ret = tcp.socket_connect_send(ip, msg);
            if(ret)
                return true;
        }
        catch(ErrorExcept e)
        {
           if(e.errno_ == ECONNREFUSED ) 
               LOG_DEBUG("GOT ECONNREFUSED");
           else
               return false;
        }
        usleep(100000);
    }
    return false;
}

#endif // _SINGLE_BUFFER_H_
