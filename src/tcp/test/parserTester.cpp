#include <glib.h>
#include <stdio.h>
#include <map>
#include <string>
#include "tcp/parser.h"
#include "logWriter.h"
#include "mapMsg.h"

#include <cpptest.h>

class ParserTester : public Test::Suite
{
    public:
        ParserTester()
        {
            TEST_ADD(ParserTester::tokenize_test)
            TEST_ADD(ParserTester::stringify_test)
            TEST_ADD(ParserTester::roundtrip_test)

        }

private:
        void tokenize_test();
        void stringify_test();
        void roundtrip_test();

};

void ParserTester::tokenize_test()
{ 
    TEST_ASSERT(1);
    LOG_ERROR("Here");
    TEST_ASSERT(1);
}

void ParserTester::stringify_test()
{

}

void ParserTester::roundtrip_test()
{
    std::string str = "init: it=103945 mystring=\"\\\"bar you\\\"\" Hellotab=1.1";
    std::map<std::string, StrIntFloat> mymap;
    if(tokenize(str, mymap))
        LOG_DEBUG("Success");
        else
    LOG_DEBUG("Fail");

            std::string out;
    if(stringify(mymap, out))
        LOG_DEBUG(out);
    else
        LOG_DEBUG("Fail stringify");


}

int main(int, char**)
{
    ParserTester pt;
    Test::TextOutput output(Test::TextOutput::Verbose);
    return pt.run(output,false);
}

