class TcpServer
{
    public:
        TcpServer(int port)
            : port_(port){}

        bool send(const std::string& msg){ return !msg.empty();}
        bool socket_bind_listen(){ return true;}
        bool accept(){ return true;}
        bool connected(){return connected_;}
        bool read(std::string& buff){
            if(!connected)
                return false;
            buff = "data";
            return true;
        }


    private:
        bool connected_;
}
