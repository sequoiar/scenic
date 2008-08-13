#include <cassert>
#include <iostream>
#include <stdlib.h>
#include "tcp/tcpThread.h"
int main(int argc, char** argv)
{
    if(argc < 2)
        return 0;
    int port = atoi(argv[1]);
    TcpThread tcp(port);
    TcpQueue& queue = tcp.getQueue();

    tcp.run();

    while(1)
    {
        StdMsg f = queue.timed_pop(1000000);
        if(f.get_type() == StdMsg::STD)
        {
            std::cout << f.getMsg() << std::endl;
            if(!f.getMsg().compare(0, 4, "quit"))
            {
                StdMsg q(StdMsg::QUIT);
                queue.push(q);
                break;
            }
            else{
                tcp.send(f.getMsg());
            }
        }
    }
}


