#ifndef __MESSAGE__
#define __MESSAGE__
namespace message
{
enum type
{
    undefined, err, ok, ack, open, close, start, stop, pause, quit, info
};
const char *str[] = {
    "undefined", "err", "ok", "ack", "open", "close", "start", "stop", "pause", "quit", "info"
};
}

struct Message
{
    Message(int m) : type((message::type)(m))
    {
    }
    Message(message::type m) : type(m)
    {
    }
    Message() : type(message::undefined)
    {
    }
    message::type type;
    int getInt()
    {
        return (int) type;
    }
};

#endif
