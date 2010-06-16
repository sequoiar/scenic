#include "CapsClient.h"
#include "util.h"
#include <iterator>
#include <boost/asio.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/thread.hpp>

#pragma GCC diagnostic ignored "-Weffc++"
using boost::asio::ip::tcp;

CapsClient::CapsClient(const std::string &host, const std::string &port) :
        host_(host),
        port_(port)
    {}

std::string CapsClient::getCaps()
{
    boost::asio::io_service io_service_;
    tcp::resolver resolver(io_service_);
    tcp::resolver::query query(host_.c_str(), port_.c_str());

    tcp::socket socket(io_service_);
    bool connected = false;
    boost::system::error_code error;
    while (not connected)
    {
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;
        error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end)
        {
            socket.close();
            socket.connect(*endpoint_iterator++, error);
        }
        if (error == boost::asio::error::connection_refused or
                error == boost::asio::error::host_not_found)
        {
            LOG_DEBUG("no host yet, sleep");
            boost::this_thread::sleep(boost::posix_time::seconds(1));
        }
        else if (error)
            throw boost::system::system_error(error);
        else
            connected = true;

        // client must check for this in case we've been interrupted
        // FIXME: this isn't very elegant but at least it gets us to the top level
        if (signal_handlers::signalFlag())
            THROW_ERROR("INTERRUPTED");
    }

    boost::asio::streambuf buf;

    bool done = false;
    while (not done)
    {
        size_t len = boost::asio::read_until(socket, buf, '\0', error);
        if (error == boost::asio::error::eof)
        {
            LOG_DEBUG("closed by peer\n");
            done = true;
        }
        else if (error)
            throw boost::system::system_error(error); // Some other error.
        else
            LOG_DEBUG("Received " << len << "bytes");
    }

    std::string retVal((std::istreambuf_iterator<char>(&buf)),
            std::istreambuf_iterator<char>());
    return retVal;
}

