#include <cassert>
#include <iostream>
#include <stdlib.h>
#include "tcp/tcpThread.h"
#include "tcp/parser.h"
#include "logWriter.h"

int main(int argc, char** argv)
{
    if(argc < 2)
        return 0;
    int port = atoi(argv[1]);
    TcpThread tcp(port);
    TcpQueue& queue = tcp.getQueue();
    LOG("helo",DEBUG);
    tcp.run();

    while(1)
    {
        MapMsg f = queue.timed_pop(1000000);
        std::string command;
        std::cout << f["command"].type() << std::endl;
        if(f["command"].get(command))
        {
            std::cout << command << std::endl;
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
}


