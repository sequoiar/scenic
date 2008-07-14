#include "gstThread.h"
#include "osc/osc.h"
#include "gutil/optionArgs.h"

int main (int argc, char** argv)
{
    GstThread gst;
    OscThread o;
    OptionArgs opts;

    opts.add(gst.get_args());
    opts.add(o.get_args());

    if(!opts.parse(argc,argv))
        return 1;

    QueuePair gst_queue = gst.getQueue("");
    QueuePairOfOscMessage osc_queue = o.getQueue("");
    if(!gst.run())
        return -1;

    if(!o.run())
        return -1;

    while(1)
    {
        OscMessage m = osc_queue.copy_timed_pop(10000);
        if(m.path.empty() || m.path.compare("/gst"))
            continue;

        LOG(m.args[0].s);
        if(!m.args[0].s.compare("init")){
            BaseMessage in(BaseMessage::init);
            gst_queue.push(in);
        }

        if(!m.args[0].s.compare("start")){
            BaseMessage start(BaseMessage::start);
            gst_queue.push(start);
        }
        if(!m.args[0].s.compare("stop")){
            BaseMessage stop(BaseMessage::stop);
            gst_queue.push(stop);
        }
/*
        BaseMessage f = queue.copy_timed_pop(1);

        if(m.get_type() == BaseMessage::quit) {
            break;
        }
*/


    }

    std::cout << "Done!" << std::endl;

    return 0;
}

//./mainTester -s videotestsrc --oscLocal=7770 --oscRemote=7771 --oscRemoteHost=127.0.0.1
