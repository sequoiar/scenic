/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <error.h>

#include <string>

#include "tcpServer.h"

#ifdef ALLOW_ANY_ADDR
#define INADDR   INADDR_ANY
#else
#define INADDR   inet_addr("10.12.110.57")
#endif



void error(const char *msg)
{
    perror(msg);
    exit(1);
}

bool TcpServer::socket_bind_listen()
{
    struct sockaddr_in serv_addr;
    int optval = 1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR;
    serv_addr.sin_port = htons(port_);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    listen(sockfd,5);

    return true;
}
    
bool TcpServer::accept()
{ 
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);
    newsockfd = ::accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
        error("ERROR on accept");
    return true;
}

bool TcpServer::recv(std::string& out)
{
    int n=0;
        out.clear();
        bzero(buffer_,BUFFSIZE);
        n = ::read(newsockfd,buffer_,BUFFSIZE);

        if (n < 0 || buffer_[0] == 0) return false; //error("ERROR reading from socket");
        printf("Here is the message: %s\n",buffer_);
    return true;
}

bool TcpServer::send(const std::string& in)
{
    int n=0;
        n = ::write(newsockfd,"I got your message\n",19);
        if (n < 0) return false; //error("ERROR writing to socket");
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




