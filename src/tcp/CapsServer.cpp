
#include "util/logWriter.h"
#include "CapsServer.h"
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
typedef boost::shared_ptr<TcpConnection> connection_ptr;
        
connection_ptr TcpConnection::create(boost::asio::io_service& io_service, const std::string &message)
{
    return connection_ptr(new TcpConnection(io_service, message));
}

void TcpConnection::start()
{
    boost::asio::async_write(socket_, boost::asio::buffer(message_),
            boost::bind(&TcpConnection::handle_write, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

TcpConnection::TcpConnection(boost::asio::io_service& io_service, const std::string &message) : 
    socket_(io_service), message_(message)
{}

void TcpConnection::handle_write(const boost::system::error_code& /*error*/,
        size_t bytes_transferred)
{
    LOG_DEBUG(bytes_transferred << " bytes transferred");
}

// an async tcp server that serves caps
CapsServer::CapsServer(unsigned int port, const std::string &caps) : 
    caps_(caps),
    io_service_(),
    acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)),
    dummy_(start_accept()),
    serverThread_(boost::bind(&boost::asio::io_service::run, &io_service_)) {}

CapsServer::~CapsServer() 
{
    io_service_.stop();
    serverThread_.join();
}

// gets called by constructor and also when we have new connections
int CapsServer::start_accept()
{
    TcpConnection::connection_ptr new_connection =
        TcpConnection::create(acceptor_.io_service(), caps_);

    acceptor_.async_accept(new_connection->socket(),
            boost::bind(&CapsServer::handle_accept, 
                this, 
                new_connection,
                boost::asio::placeholders::error));
    // hack so that we can call this from an initalizer list
    return 0;
}

void CapsServer::handle_accept(TcpConnection::connection_ptr new_connection, 
        const boost::system::error_code& error)
{
    if (not error)
    {
        new_connection->start();
        start_accept();
    }
}

