#include <Python.h>
#include <boost/python.hpp>
#include "msgThread.h"


class MsgThreadWrap
{
TcpThread thread_;
QueuePair &q_;
public:
    MsgThreadWrap(int port): 
        thread_(port,0),q_(thread_.getQueue())
    {thread_.run();}
   
    bool send(boost::python::tuple /*tu*/)
    { return true; }
 
    boost::python::dict
    getMsg(int ms)
    { 
        MapMsg m = q_.timed_pop(ms*1000);
        boost::python::dict d;
        const std::pair<const std::string, StrIntFloat>* it;
        for(it = m.begin(); it != 0; it = m.next())
        {
            LOG_DEBUG("");
            switch(it->second.get_type())
            {
                case 's':
                    d.setdefault(it->first.c_str(),std::string(it->second));
                    break;
                case 'i':
                    d.setdefault(it->first.c_str(),int(it->second));
                    break;
                case 'f':
                    d.setdefault(it->first.c_str(),double(it->second));
                    break;
                case 'F':
                    break;
                case 'e':
                    break;
//                default:
//                    THROW_ERROR("Command " << it->first
  //                                         << " has unknown type " << it->second.get_type());
            }
        }
        return d;
    }
};




