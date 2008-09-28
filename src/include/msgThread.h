#include "baseThread.h"
#include "mapMsg.h"

typedef QueuePair_<MapMsg> QueuePair;
class MsgThread
    : public BaseThread<MapMsg>
{


};
