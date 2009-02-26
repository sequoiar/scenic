#include "util.h"
#include <glib.h>
#include <stdio.h>
#include <map>
#include <string>
#include "tcp/parser.h"
#include "mapMsg.h"


class ParserTester
{
    public:
        ParserTester(char** argv)
            : arg(argv[1])
        {
            tokenize_test();
        }


    private:
        void tokenize_test();
        void stringify_test();
        void roundtrip_test();

        std::string arg;
};

void ParserTester::tokenize_test()
{
    std::string str = "init: it=103945 mystring=\"\\\"bar you\\\"\" Hellotab=1.1 2.2 ";
    MapMsg mymap;
    assert(Parser::tokenize(str, mymap));
}


void ParserTester::stringify_test()
{}


void ParserTester::roundtrip_test()
{
//    std::string str = "init: it=103945 mystring=\"\\\"bar you\\\"\" Hellotab=1.1";

    MapMsg mymap;
    assert(Parser::tokenize(arg, mymap));

    std::string out;
    assert(Parser::stringify(mymap, out));
    LOG_DEBUG(out);
}


int main(int argc, char** argv)
{
    if (argc < 2)
        return -1;
    ParserTester pt(argv);
}


