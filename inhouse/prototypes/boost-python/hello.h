#ifndef _HELLO_H_
#define _HELLO_H_

#include <string>

class hello
{
    std::string s;
public:
    const char * greet();
    void set_name(char const* n);
    void printTest1();
    void printTest2();
};

#endif //_HELLO_H_
