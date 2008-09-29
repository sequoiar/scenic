/* GTHREAD-QUEUE-PAIR - Library of TcpThread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews
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

#include "msgThread.h"
#include "tcpServer.h"

class TcpThread
    : public MsgThread
{
    public:
        TcpThread(int inport,bool logF=false)
            : serv_(inport),logFlag_(logF){}
        ~TcpThread(){}
        bool send(MapMsg& msg);

        bool socket_connect_send(const std::string& addr, MapMsg& msg);

    private:
        int main();
        bool gotQuit();

        TcpServer serv_;
        bool logFlag_;

        TcpThread(const TcpThread&); //No Copy Constructor
        TcpThread& operator=(const TcpThread&); //No Assignment Operator
};
