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
                    if(!command.compare("quit"))
                    {
                        queue.push(f);
                        break;
                    }
                    else{
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


