#include <Python.h>
#include <boost/python.hpp>
#include "msgThread.h"

class WrapMsgThread
{
MsgThread* msgThread;
public:
    WrapMsgThread(MsgThread* t): msgThread(t){msgThread->run();}
   
    bool send(boost::python::tuple /*tu*/)
    {
        return true;
    }
 
    boost::python::tuple
    getMsg(int ms)
    {
        return boost::python::make_tuple(ms,__TIME__);
    }

};




