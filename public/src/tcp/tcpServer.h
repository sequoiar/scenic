#ifndef TCP_SERVER
#define TCP_SERVER

#define BUFFSIZE     256

class TcpServer
{
    public:
        TcpServer(int port)
            : sockfd(0), newsockfd(0), buffer_(), port_(port), connected_(false){}
        bool recv(std::string& out);
        bool send(const std::string& in);
        bool socket_bind_listen();
        bool accept();
        bool close();

        bool connected(){return connected_;}
    private:
        int sockfd, newsockfd;

        char buffer_[BUFFSIZE];
        int port_;
        bool connected_;
};


#endif
