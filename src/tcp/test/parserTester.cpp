#include "util.h"
#include <glib.h>
#include <stdio.h>
#include <map>
#include <string>
#include "tcp/parser.h"
#include "mapMsg.h"

#include <cpptest.h>

class ParserTester
    : public Test::Suite
{
    public:
        ParserTester(char** argv)
            : arg(argv[1])
        {
            TEST_ADD(ParserTester::tokenize_test)
//            TEST_ADD(ParserTester::stringify_test)
//            TEST_ADD(ParserTester::roundtrip_test)
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
    TEST_ASSERT(Parser::tokenize(str, mymap));
}


void ParserTester::stringify_test()
{}


void ParserTester::roundtrip_test()
{
//    std::string str = "init: it=103945 mystring=\"\\\"bar you\\\"\" Hellotab=1.1";

    MapMsg mymap;
    TEST_ASSERT(Parser::tokenize(arg, mymap));

    std::string out;
    TEST_ASSERT(Parser::stringify(mymap, out));
    LOG_DEBUG(out);
}


int main(int argc, char** argv)
{
    if (argc < 2)
        return -1;
    ParserTester pt(argv);
    Test::TextOutput output(Test::TextOutput::Verbose);
    return pt.run(output, false);
}


