#include "builder.h"
#include "tcp/tcpThread.h"
#include "gstSenderThread.h"
#include "gstReceiverThread.h"

MsgThread* Builder::TcpBuilder(int port,bool log)
{
    return ( new TcpThread(port,log) );
}

MsgThread* Builder::GstBuilder(bool send)
{
    if(send)
        return new GstSenderThread();
    else
        return new GstReceiverThread();
}

