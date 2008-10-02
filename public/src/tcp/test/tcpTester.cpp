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
            THROW_CRITICAL("2 or 3 args: port [addr]");

        int port = atoi(argv[1]);
        TcpThread tcp(port,true);


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
                        break;
                        
                    if (command == "exception")
                        throw(f["exception"].except());
    
                    
                    if(command == "test")
                        LOG_DEBUG(tcp.socket_connect_send("127.0.0.1",f));
                    else
                        tcp.send(f);
                    
                }
            }
            catch(ErrorExcept e) 
            { 
               LOG_DEBUG("In while Except. " << e.msg_); 
            }
        }
    }
    catch(Except e)
    {

    }
    return -1;
}


