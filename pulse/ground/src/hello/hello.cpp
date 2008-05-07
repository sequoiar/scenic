#include "hello.h"

const char * hello::greet()
{
    static std::string ts;
    ts = "hello " + s;
    
    return ts.c_str();
}

void hello::set_name(char const* n)
{
    s = n;
}


