
#include "tcp/tcpThread.h"
#include "tcp/parser.h"

static std::string tcpGetBuffer(int port)
{
    TcpThread tcp(port);
    tcp.run();
    QueuePair& queue = tcp.getQueue();
    for(;;)
    {
        MapMsg f = queue.timed_pop(100000);
        if(f["command"].empty())
            continue;
        try
        {
            if(std::string(f["command"]) != "buffer")
            {
                LOG_INFO("Unknown msg.");
                continue;
            }
            return f["str"];
        }
        catch(ErrorExcept)
        {
        }
    }
    return "";
}

#include <errno.h>

static bool tcpSendBuffer(const char *ip, int port, const std::string &caps)
{
    MapMsg msg("buffer");

    TcpThread tcp(port);

    msg["str"] = caps;

    const int MAX_TRIES = 100;

    for(int i = 0; i < MAX_TRIES; ++i)
    {
        try
        {
            bool ret = tcp.socket_connect_send(ip, msg);
            if(ret)
                return true;
        }
        catch(ErrorExcept e)
        {
           if(e.errno_ == ECONNREFUSED ) 
               LOG_DEBUG("GOT ECONNREFUSED");
           else
               return false;
        }
        usleep(100000);
    }
    return false;
}



