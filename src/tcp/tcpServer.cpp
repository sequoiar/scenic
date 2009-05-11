// tcpServer.cpp
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

#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <error.h>
#include <errno.h>

#include "tcpServer.h"

#define ALLOW_ANY_ADDR
#ifdef ALLOW_ANY_ADDR
#define INADDR   INADDR_ANY
#else
#define INADDR   inet_addr("127.0.0.1")
#endif


bool TcpServer::set_non_blocking(int sockfd_param)
{
    long arg;

    // Set non-blocking
    if( (arg = fcntl(sockfd_param, F_GETFL, NULL)) < 0)
    {
        return false;
    }
    arg |= O_NONBLOCK;
    if( fcntl(sockfd_param, F_SETFL, arg) < 0)
    {
        return false;
    }
    return true;
}


bool TcpServer::socket_connect_send(const std::string& addr, const std::string& msg) const
{
    struct sockaddr_in serv_addr;
    //int optval = 1;
    int ssockfd = 0;
    ssockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ssockfd <= 0)
        THROW_ERROR("Error opening socket: errno msg: " << strerror(errno));
    bzero((char *) &serv_addr, sizeof(serv_addr));


    if(!inet_aton(addr.c_str(), &(serv_addr.sin_addr)))
    {
        ::close(ssockfd);
        THROW_ERROR("Address bad." << addr);
    }
    serv_addr.sin_family = AF_INET;
    // serv_addr.sin_addr.s_addr = INADDR;
    serv_addr.sin_port = htons(port_);


    if (connect(ssockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        ::close(ssockfd);
        return false;
        //THROW_ERRNO("Cannot Connect to peer. " <<  strerror(errno), errno);
    }
    int n=0;
    n = ::write(ssockfd, msg.c_str(), msg.size());
    n = ::write(newsockfd, "\r\n", 2);                       //Telnet standard line end
    if (n <= 0){
        ::close(ssockfd);
        THROW_ERROR("Writing to socket failed.");
    }
    usleep(1000000);
    ::close(ssockfd);
    return true;
}


bool TcpServer::socket_bind_listen()
{
    struct sockaddr_in serv_addr;
    int optval = 1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0)
        THROW_ERRNO("Error opening socket: errno msg: " << strerror(errno), errno);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    set_non_blocking(sockfd);

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR;
    serv_addr.sin_port = htons(port_);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        THROW_CRITICAL("Error at bind: errno msg: " << strerror(errno));
    if(!listen(sockfd, 5) <= 0)
        THROW_ERRNO("Error on listen: errno msg: " << strerror(errno), errno);
    LOG_INFO("Listening for connection on port: " << port_);
    puts("READY");
    return true;
}


bool TcpServer::accept()
{
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);
    newsockfd = ::accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd <= 0){
        if(newsockfd == -1 && errno == EWOULDBLOCK)
            return false;
        else
            THROW_ERROR("Error on listen: errno msg: " << strerror(errno));
    }
    connected_ = true;
    set_non_blocking(newsockfd);

    ::close(sockfd);
    sockfd = 0;

    return true;
}


bool TcpServer::recv(std::string& out)
{
    int n=0;
    out.clear();

    //while more data to receive
    do
    {
        bzero(buffer_, BUFFSIZE);
        n = ::recv(newsockfd, buffer_, BUFFSIZE, 0);
        if (n <= 0 || buffer_[0] == 0)
        {
            if (n != 0 && errno == EWOULDBLOCK)
                break;
            connected_ = false;
            return false;
        }
        out.append(buffer_, n);
    } while(1);

    if(out.empty())
        return false;
    return true;
}


bool TcpServer::send(const std::string& in)
{
    if(!connected())
        return false;
    int n=0;
    n = ::write(newsockfd, in.c_str(), in.size());
    n = ::write(newsockfd, "\r\n", 2);                       //Telnet standard line end
    if (n <= 0)
        THROW_ERRNO("Writing to socket failed.", errno);
    return true;
}


bool TcpServer::close()
{
    if(sockfd)
        if(!::close(sockfd))
            return false;
    if(newsockfd)
        if(!::close(newsockfd))
            return false;
    return true;
}


