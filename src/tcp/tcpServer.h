// tcpServer.h
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

/** \file 
 *      TcpServer class  
 *
 */


#ifndef TCP_SERVER
#define TCP_SERVER

#include <string>

#define BUFFSIZE     16

class TcpServer
{
    public:
        TcpServer(int port)
            : sockfd(0), newsockfd(0), buffer_(), port_(port), connected_(false){}
        bool recv(std::string& out);
        bool send(const std::string& in);
        bool socket_bind_listen();
        bool socket_connect_send(const std::string& addr,const std::string& in);
        bool accept();
        bool close();

        bool set_non_blocking(int sockfd);

        bool connected(){return connected_;}
    private:
        int sockfd, newsockfd;

        char buffer_[BUFFSIZE];
        int port_;
        bool connected_;
     
};


#endif
