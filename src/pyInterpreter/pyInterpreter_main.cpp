#include "pyInterpreter.h"

#include <iostream>

int GROUND_LOOP(int result,int argc, char* argv[]);

int main(int argc, char* argv[])
{
    static int count;
    std::string result;
    pyInterpreter py;

    py.init(argc,argv);

    std::cout << std::endl << "Welcome to the Console" << std::endl;
    std::cout << "Brought to you by The Military Industial Complex." << std::endl;
    while(1)
    {
        std::cout << "GL: ";
        if(GROUND_LOOP(count,argc,argv))
        {
            if(!py.run_input().empty())
                count = -1;
        }
        else
            break;
    }

    std::cout << std::endl << "going down - CLEANUP ----" << std::endl;
}
