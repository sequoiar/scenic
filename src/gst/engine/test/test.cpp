#include <cppunit/TestCase.h>
class ComplexNumberTest : public CppUnit::TestCase { 
public: 
  ComplexNumberTest( std::string name ) : CppUnit::TestCase( name ) {}
  
  void runTest() {
    CPPUNIT_ASSERT( 1 );
    CPPUNIT_ASSERT( !0 );
  }
};

int main(int, char**)
{
    ComplexNumberTest test("Hello");
    test.runTest();
}
