//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class tcp_session
{
    public:
        tcp_session(boost::asio::io_service& io_service)
            : socket_(io_service)
        {
        }

        tcp::socket& socket()
        {
            return socket_;
        }

        void start()
        {
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                    boost::bind(&tcp_session::handle_read, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }

        void handle_read(const boost::system::error_code& error,
                size_t bytes_transferred)
        {
            if (!error)
            {
                boost::asio::async_write(socket_,
                        boost::asio::buffer(data_, bytes_transferred),
                        boost::bind(&tcp_session::handle_write, this,
                            boost::asio::placeholders::error));
            }
            else
            {
                delete this;
            }
        }

        void handle_write(const boost::system::error_code& error)
        {
            if (!error)
            {
                socket_.async_read_some(boost::asio::buffer(data_, max_length),
                        boost::bind(&tcp_session::handle_read, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
            }
            else
            {
                delete this;
            }
        }

    private:
        tcp::socket socket_;
        enum { max_length = 1024 };
        char data_[max_length];
};

class tcp_server
{
    public:
        tcp_server(boost::asio::io_service& io_service, short port)
            : io_service_(io_service),
            acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
        {
            tcp_session* new_tcp_session = new tcp_session(io_service_);
            acceptor_.async_accept(new_tcp_session->socket(),
                    boost::bind(&tcp_server::handle_accept, this, new_tcp_session,
                        boost::asio::placeholders::error));
        }

        void handle_accept(tcp_session* new_tcp_session,
                const boost::system::error_code& error)
        {
            if (!error)
            {
                new_tcp_session->start();
                new_tcp_session = new tcp_session(io_service_);
                acceptor_.async_accept(new_tcp_session->socket(),
                        boost::bind(&tcp_server::handle_accept, this, new_tcp_session,
                            boost::asio::placeholders::error));
            }
            else
            {
                delete new_tcp_session;
            }
        }

    private:
        boost::asio::io_service& io_service_;
        tcp::acceptor acceptor_;
};


class udp_server
{
    public:
        udp_server(boost::asio::io_service& io_service, short port)
            : io_service_(io_service),
            socket_(io_service, udp::endpoint(udp::v4(), port))
        {
            socket_.async_receive_from(
                    boost::asio::buffer(data_, max_length), sender_endpoint_,
                    boost::bind(&udp_server::handle_receive_from, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }

        void handle_receive_from(const boost::system::error_code& error,
                size_t bytes_recvd)
        {
            if (!error && bytes_recvd > 0)
            {
                socket_.async_send_to(
                        boost::asio::buffer(data_, bytes_recvd), sender_endpoint_,
                        boost::bind(&udp_server::handle_send_to, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
            }
            else
            {
                socket_.async_receive_from(
                        boost::asio::buffer(data_, max_length), sender_endpoint_,
                        boost::bind(&udp_server::handle_receive_from, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
            }
        }

        void handle_send_to(const boost::system::error_code& error, size_t bytes_sent)
        {
            socket_.async_receive_from(
                    boost::asio::buffer(data_, max_length), sender_endpoint_,
                    boost::bind(&udp_server::handle_receive_from, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }

    private:
        boost::asio::io_service& io_service_;
        udp::socket socket_;
        udp::endpoint sender_endpoint_;
        enum { max_length = 1024 };
        char data_[max_length];
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_udp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        using namespace std; // For atoi.
        tcp_server s(io_service, atoi(argv[1]));
        udp_server us(io_service, atoi(argv[1]));
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

