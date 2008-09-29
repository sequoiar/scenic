#include "msgThread.h"

namespace Builder
{
    MsgThread* TcpBuilder(int port,bool log);
    MsgThread* GstBuilder(bool send);
}
