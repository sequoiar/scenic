#include <cppunit/TestCase.h>
#include "../remoteConfig.h"

class ReceiverConfigTest : public CppUnit::TestCase 
{ 
public: 
  ReceiverConfigTest(std::string name) : CppUnit::TestCase(name) {}
  
  void runTest() 
  {
    CPPUNIT_ASSERT(ReceiverConfig::isSupportedCodec("mpeg4"));
    CPPUNIT_ASSERT(ReceiverConfig::isSupportedCodec("theora"));
    CPPUNIT_ASSERT(ReceiverConfig::isSupportedCodec("h264"));
    CPPUNIT_ASSERT(ReceiverConfig::isSupportedCodec("h263"));
    CPPUNIT_ASSERT(ReceiverConfig::isSupportedCodec("vorbis"));
    CPPUNIT_ASSERT(ReceiverConfig::isSupportedCodec("raw"));
    CPPUNIT_ASSERT(ReceiverConfig::isSupportedCodec("mp3"));
    CPPUNIT_ASSERT(ReceiverConfig::isSupportedCodec("poop"));
  }
};

int main(int, char**)
{
    ReceiverConfigTest test("ReceiverConfigTest");
    test.runTest();
}

