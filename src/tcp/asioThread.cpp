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

#include "asioThread.h"

#ifdef HAVE_BOOST_ASIO
#include <boost/bind.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::asio::io_service;
using boost::system::error_code;
using boost::asio::async_write;
using boost::asio::buffer;
using boost::asio::placeholders::error;
using boost::asio::placeholders::bytes_transferred;

class tcp_session
{
    public:
        tcp_session(io_service& io_service)
            : socket_(io_service), welcome_()
        {
        }

        tcp::socket& socket()
        {
            return socket_;
        }

        void start()
        {
            welcome_ = "welcome.\nREADY:\n"; 
                async_write(socket_,
                        buffer(welcome_),
                        boost::bind(&tcp_session::write_cb, this,
                            error));
        }

        void read_cb(const error_code& error,
                size_t bytes_transferred)
        {
            if (!error)
            {
                async_write(socket_,
                        buffer(data_, bytes_transferred),
                        boost::bind(&tcp_session::write_cb, this,
                            error));
            }
            else
            {
                delete this;
            }
        }

        void write_cb(const error_code& error)
        {
            if (!error)
            {
                socket_.async_read_some(buffer(data_, max_length),
                        boost::bind(&tcp_session::read_cb, this,
                            error,
                            bytes_transferred));
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
        std::string welcome_;
};

class tcp_server
{
    public:
        tcp_server(io_service& io_service, short port)
            : io_service_(io_service),
            acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
        {
            tcp_session* new_tcp_session = new tcp_session(io_service_);
            acceptor_.async_accept(new_tcp_session->socket(),
                    boost::bind(&tcp_server::handle_accept, this, new_tcp_session,
                        error));
        }

        void handle_accept(tcp_session* new_tcp_session,
                const error_code& error)
        {
            if (!error)
            {
                new_tcp_session->start();
                new_tcp_session = new tcp_session(io_service_);
                acceptor_.async_accept(new_tcp_session->socket(),
                        boost::bind(&tcp_server::handle_accept, this, new_tcp_session,
                            error));
            }
            else
            {
                delete new_tcp_session;
            }
        }

    private:
        io_service& io_service_;
        tcp::acceptor acceptor_;
};


class udp_server
{
    public:
        udp_server(io_service& io_service, short port)
            : io_service_(io_service),
            socket_(io_service, udp::endpoint(udp::v4(), port)), sender_endpoint_()
        {
            socket_.async_receive_from(
                    buffer(data_, max_length), sender_endpoint_,
                    boost::bind(&udp_server::handle_receive_from, this,
                        error,
                        bytes_transferred));
        }

        void handle_receive_from(const error_code& error,
                size_t bytes_recvd)
        {
            if (!error && bytes_recvd > 0)
            {
                socket_.async_send_to(
                        buffer(data_, bytes_recvd), sender_endpoint_,
                        boost::bind(&udp_server::handle_send_to, this,
                            error,
                            bytes_transferred));
            }
            else
            {
                socket_.async_receive_from(
                        buffer(data_, max_length), sender_endpoint_,
                        boost::bind(&udp_server::handle_receive_from, this,
                            error,
                            bytes_transferred));
            }
        }

        void handle_send_to(const error_code& error, size_t )//bytes_sent)
        {
            socket_.async_receive_from(
                    buffer(data_, max_length), sender_endpoint_,
                    boost::bind(&udp_server::handle_receive_from, this,
                        error,
                        bytes_transferred));
        }

    private:
        io_service& io_service_;
        udp::socket socket_;
        udp::endpoint sender_endpoint_;
        enum { max_length = 1024 };
        char data_[max_length];
};
#endif



int asio_thread::main()
{


    try
    {
#ifdef HAVE_BOOST_ASIO
        io_service io_service;

        using namespace std; // For atoi.
        tcp_server s(io_service, 1111);
        udp_server us(io_service, 1111);

        MapMsg msg;
        for(;;)
        {
            msg = queue_.timed_pop(2000);
            if(msg.cmd().empty())
                io_service.poll();
            else
                if(msg.cmd() == "quit")
                    break;
        }
#endif
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

