#ifndef __OSC_THREAD_H__
#define __OSC_THREAD_H__

#include "thread/baseThread.h"
#include "lo/lo.h"
#include <string>
class OscMessage
{
public:
    OscMessage(const char*p,const char *t, lo_arg **v, int c,void* d)
        :path(p),types(t),argv(v),argc(c),data(d) {}
    OscMessage(){}
    std::string path,types;
    lo_arg** argv;
    int argc;
    void* data;

};

typedef QueuePair_<OscMessage> QueuePairOfOscMessage;

class OscThread : public BaseThread<OscMessage>
{
    int main();
    static int generic_handler_static(const char *path, const char *types, lo_arg **argv, int argc,
                void *data, void *user_data);
 
    int generic_handler(const char *path, const char *types, lo_arg **argv, int argc,
                void *data);
    static void liblo_error(int num, const char *msg, const char *path){}
};

#endif
