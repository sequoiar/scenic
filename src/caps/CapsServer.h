#ifndef _CAPS_SERVER_H_
#define _CAPS_SERVER_H_

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <string>

#pragma GCC diagnostic ignored "-Weffc++"
// encapsulates instance of new connection
class TcpConnection : public boost::enable_shared_from_this<TcpConnection>
{
    public:
        typedef boost::shared_ptr<TcpConnection> connection_ptr;

        static connection_ptr create(boost::asio::io_service& io_service, const std::string &message);
        
        boost::asio::ip::tcp::socket& socket()
        {
            return socket_;
        }

        void start();

    private:
        TcpConnection(boost::asio::io_service& io_service, const std::string &message);

        void handle_write(const boost::system::error_code& error,
                size_t bytes_transferred);

        boost::asio::ip::tcp::socket socket_;
        std::string message_;
};

// empty base class so we can switch implementations
class CapsServer {
};

// an async tcp server that serves caps
class TcpCapsServer : public CapsServer {
    public:
        TcpCapsServer(unsigned int port, const std::string &caps);
        ~TcpCapsServer();

    private:
        int start_accept();

        void handle_accept(TcpConnection::connection_ptr new_connection,
                const boost::system::error_code& error);

        std::string caps_;
        boost::asio::io_service io_service_;
        boost::asio::ip::tcp::acceptor acceptor_; 
        int dummy_;
        boost::thread serverThread_;
};

#endif // _CAPS_SERVER_H_
