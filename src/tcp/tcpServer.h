#ifndef TCP_SERVER
#define TCP_SERVER

class TcpServer
{
    public:
        TcpServer(int port)
            : port_(port),connected_(false){}

        bool send(const std::string& msg){ return !msg.empty();}
        bool socket_bind_listen(){ return true;}
        bool accept(){ return true;}
        bool connected(){return connected_;}
        bool read(std::string& buff){
            if(!connected_)
                return false;
            buff = "data";
            return true;
        }


    private:
		int port_;
        bool connected_;
};

#endif
