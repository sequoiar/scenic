#include "pyInterpreter.h"

#include <iostream>

int _ground_init(int argc, char* argv[]);
int _ground_loop(int result);

int main(int argc, char* argv[])
{
    static int count;
    std::string result;
    pyInterpreter py;

    py.init(argc,argv);

    _ground_init(argc,argv);

    std::cout << std::endl << "Welcome to the Console" << std::endl;
    std::cout << "Brought to you by The Military Industrial Complex." << std::endl;
    while(1)
    {
        std::cout << "GL: ";
        if(_ground_loop(count))
        {
            if(!py.run_input().empty())
                count = -1;
        }
        else
            break;
    }

    std::cout << std::endl << "going down - CLEANUP ----" << std::endl;
}
