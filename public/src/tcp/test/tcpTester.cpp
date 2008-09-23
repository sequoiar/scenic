#include <cassert>
#include <iostream>
#include <stdlib.h>
#include "tcp/tcpThread.h"
#include "tcp/parser.h"
#include "logWriter.h"

int main(int argc, char** argv)
{
    try
    {
        if(argc < 2)
            LOG_CRITICAL("2 or 3 args: port [addr]");
        int port = atoi(argv[1]);
        TcpThread tcp(port);


        QueuePair& queue = tcp.getQueue();
        tcp.run();

        while(1)
        {
            try
            {
                MapMsg f = queue.timed_pop(1000000);
                std::string command;
                if(f["command"].get(command))
                {
                    if(command == "quit")
                    {
                        queue.push(f);
                        break;
                    }
                    else
                    {
                        if(command == "test")
                            LOG_DEBUG(tcp.socket_connect_send("127.0.0.1",f));
                        else
                            tcp.send(f);
                    }
                }
            }
            catch(ErrorExcept) { }
        }
    }
    catch(Except e)
    {
            std::cerr << e.msg_;
    }
}


