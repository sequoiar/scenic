#include "./capsParser.h"

#include <fstream>
#include <iostream>
#include <string>

// expand macro and stringify it
#define XSTRINGIFY(x) STRINGIFY(x)
#define STRINGIFY(x) #x


std::string CapsParser::getVideoCaps(const std::string &profile)
{
    return getCapsFromFile(XSTRINGIFY(__DATA_DIR__) "/caps.txt", profile);
}


std::string CapsParser::getAudioCaps(const std::string &profile)
{
    return getCapsFromFile(XSTRINGIFY(__DATA_DIR__) "/caps.txt", profile);
}


#undef STRINGIFY
#undef XSTRINGIFY


std::string CapsParser::getCapsFromFile(const char *filename, const std::string &profile)
{
    bool foundCaps = false;
    std::string line;
    std::ifstream capsFile(filename);

    if (capsFile.is_open())
    {
        while (!capsFile.eof() and !foundCaps)
        {
            getline(capsFile, line);
            if (line == profile)
            {
                getline(capsFile, line); // get caps
                foundCaps = true;
            }
        }
        capsFile.close();
    }
    else 
        std::cerr << "Unable to open file " << filename << std::endl;

    if (not foundCaps)
        return "";
    
    return line;
}

