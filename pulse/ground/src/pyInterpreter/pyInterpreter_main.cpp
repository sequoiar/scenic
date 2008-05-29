#include "pyInterpreter.h"

#include <iostream>

int main(int argc, char* argv[])
{
    pyInterpreter py;

    py.init(argc,argv);

    py.interact();

    std::string x = py.run_str("x.greet()");

    std::cout << "RET:" << x << std::endl;
}
