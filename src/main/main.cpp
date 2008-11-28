#include <string>
#include <iostream>


int mainTcp(int argc, char **argv);
int mainPof(int argc, char **argv);

int main(int argc, char **argv)
{
   std::string filename = argv[0];
   
   if(filename.find("mainTester\0") != std::string::npos)
       return mainTcp(argc,argv);
    else if (filename.find("pof\0") != std::string::npos)
       return mainPof(argc, argv);
   else
   {
       std::cout << "Error: Symbolic link name doesn't match exectuable!" << std::endl;
        return 1;
   }
}

