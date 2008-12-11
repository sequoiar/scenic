#include "util.h"

int mainTcp(int argc, char **argv);
int mainPof(int argc, char **argv);

int main(int argc, char **argv)
{
    std::string filename = argv[0];
    LOG_DEBUG(filename);
    try
    {
        if(filename.find("mainTester\0") != std::string::npos)
            return mainTcp(argc,argv);
        else if (filename.find("pof\0") != std::string::npos)
            return mainPof(argc, argv);
        else
        {
            THROW_ERROR("The file or symbolic link named "<< filename << " has no main candidate! try renaming to pof or mainTester");
            return 1;
        }
   }
   catch(Except)
   {
   }
}

