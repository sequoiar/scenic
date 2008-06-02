#include "pyInterpreter.h"

#include <iostream>

int main(int argc, char* argv[])
{
    pyInterpreter py;

    py.init(argc,argv);

    py.interact();

}
