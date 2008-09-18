/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <error.h>
#include <errno.h>

#include <string>
#include "logWriter.h"
#include "tcpServer.h"
#include <sstream>

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


bool TcpServer::socket_bind_listen()
{
    struct sockaddr_in serv_addr;
    int optval = 1;
    sockfd = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0)
        LOG_ERROR("Error opening socket: errno msg: " << sys_errlist[errno]);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    set_non_blocking(sockfd);

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR;
    serv_addr.sin_port = htons(port_);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        LOG_ERROR("Error at bind: errno msg: " << sys_errlist[errno]);

    if(listen(sockfd, 5) <= 0)
        LOG_ERROR("Error on listen: errno msg: " << sys_errlist[errno]); 

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
            LOG_ERROR("Error on listen: errno msg: " << sys_errlist[errno]);
    }
    connected_ = true;
    set_non_blocking(newsockfd);


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

            LOG_ERROR("Error reading from socket.");
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
    int n=0;
    n = ::write(newsockfd, in.c_str(), in.size());
    if (n <= 0){
        LOG_ERROR("Writing to socket failed.");
        return false; 
    }
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


