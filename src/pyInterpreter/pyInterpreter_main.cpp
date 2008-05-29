#include "pyInterpreter.h"



int main(int argc, char* argv[])
{
    pyInterpreter py;

    py.init(argc,argv);

    py.interact();

}
