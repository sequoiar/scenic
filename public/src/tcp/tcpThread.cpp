/* GTHREAD-QUEUE-PAIR - Library of TcpThread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews
 *
 * This library is free software; you can redisttribute it and/or
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
#include "tcpThread.h"
#include "logWriter.h"
#include "parser.h"

int TcpThread::main()
{
    bool quit = false;
    std::string msg;

    if(!serv_.socket_bind_listen())
        return -1;
    while(!quit)
    {
        while(!quit)
        {
            usleep(10000);
            if(!serv_.accept() || ( quit = gotQuit() ) ){
                continue;
            }
            LOG_INFO("Got Connection.");
            while(serv_.connected())
            {
                if((quit = gotQuit()))
                {
                    break;
                }
                if(serv_.recv(msg))
                {
                    MapMsg mapMsg;
                    if(tokenize(msg, mapMsg))
                        queue_.push(mapMsg);
                    else
                        LOG_ERROR("Bad Msg Received.");
                }
                else
                    usleep(10000);
            }
            LOG_WARNING("Disconnected from Core.")
        }
        serv_.close();
    }
    return 0;
}


bool TcpThread::gotQuit()
{
    MapMsg f = queue_.timed_pop(1);
    std::string command;
    if(f["command"].get(command)&& !command.compare("quit"))
    {
        queue_.push(f);
        return true;
    }
    return false;
}


bool TcpThread::send(MapMsg& msg)
{
    std::string msg_str;
    stringify(msg, msg_str);

    return serv_.send(msg_str);
}


