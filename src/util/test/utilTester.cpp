#include "util.h"
#include "mapMsg.h"

class TestSubscriber : public MapMsg::Subscriber
{
    public:
        void operator()(MapMsg& msg)
        {
            LOG_DEBUG(msg);
        }
};

class Logger
    : public Log::Subscriber
{
    public:
        void operator()(LogLevel& l, std::string& msg){ std::cout << l << " " << msg; }

};

int main(int, char**)
{
    TestSubscriber f;
    Logger l;
    std::vector<double> v(10),v2;
    v[1] = 1.12;
    v[2] = 21.12;
    v[3] = 31.12;
    v[4] = 41.12;
    MapMsg msg("try");
    msg["vector"] = v;
    v2 = msg["vector"];
    tassert(v == v2);
    MapMsg("xtry",true)["value"] = "hello"; // how bout that foo?
    std::cout << "bye" << std::endl;
    return 0;
}

