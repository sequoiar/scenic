#include <string>
#include <iostream>

int mainSyncTestSuite(int argc, char **argv);
int mainRtpSyncTestSuite(int argc, char **argv);

int mainVideoTestSuite(int argc, char **argv);
int mainRtpVideoTestSuite(int argc, char **argv);

int mainRtpAudioTestSuite(int argc, char **argv);
int mainAudioTestSuite(int argc, char **argv);

int mainPof(int argc, char **argv);

int main(int argc, char **argv)
{
   std::string filename = argv[0];
   
   if(filename.find("syncTester") != std::string::npos)
       return mainSyncTestSuite(argc,argv);
   else if(filename.find("videoTester") != std::string::npos)
       return mainVideoTestSuite(argc,argv);
   else if(filename.find("rtpVideoTester") != std::string::npos)
       return mainRtpVideoTestSuite(argc,argv);
   else if(filename.find("rtpAudioTester") != std::string::npos)
       return mainRtpAudioTestSuite(argc,argv);
   else if(filename.find("audioTester") != std::string::npos)
       return mainAudioTestSuite(argc,argv);
   else if(filename.find("rtpSyncTester") != std::string::npos)
       return mainRtpSyncTestSuite(argc,argv);
   else if (filename.find("pof") != std::string::npos)
       return mainPof(argc, argv);
   else
   {
       std::cout << "Error: Symbolic link name doesn't match exectuable!" << std::endl;
        return 1;
   }
}

