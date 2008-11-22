/* tcpThread.h
 * Copyright 2008 Koya Charles & Tristan Matthews 
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "msgThread.h"
#include "tcpServer.h"

/// tcp server in a thread - also provides log message dispatching
class TcpThread
    : public MsgThread
{
    public:
        TcpThread(int inport, bool logF=false)
            : serv_(inport), logFlag_(logF){}
        ~TcpThread(){}
        bool send(MapMsg& msg);

        bool socket_connect_send(const std::string& addr, MapMsg& msg);

    private:
        int main();
        bool gotQuit();

        TcpServer serv_;
        bool logFlag_;

        TcpThread(const TcpThread&);            //No Copy Constructor
        TcpThread& operator=(const TcpThread&); //No Assignment Operator
};

