#include <Python.h>
#include <boost/python.hpp>
#include "msgThread.h"

class MsgWrapConfig
{
    public:
        virtual MsgThread* GetMsgThread(){ return NULL;} ;
        virtual ~MsgWrapConfig(){}
};

class TcpWrapConfig : public MsgWrapConfig
{
    int port_;
    int log_;

    public:
        TcpWrapConfig(int port,bool log):port_(port),log_(log){}
        MsgThread* GetMsgThread(){ return new TcpThread(port_,log_);}
};

class ThreadWrap
{
MsgThread& thread_;
QueuePair &q_;
public:
    ThreadWrap(MsgWrapConfig* conf): 
        thread_(*(conf->GetMsgThread())),q_(thread_.getQueue())
    {thread_.run();}
   
    bool send(boost::python::dict dt)
    {     
        boost::python::list l = dt.items();
        int n = boost::python::extract<int>(l.attr("__len__")());

            
        for ( int i = 0; i < n; i++ )
        {
            tuple val = (boost::python::extract<boost::python::tuple>(l[i]));
            LOG_DEBUG("%s:%s" % val);
        }
        return true; 
    }
 
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




