#include "./capsParser.h"

#include "util.h"
#include <fstream>
#include <iostream>
#include <string>

// expand macro and stringify it
#define XSTRINGIFY(x) STRINGIFY(x)
#define STRINGIFY(x) #x


/// Video profile is in format <codec>
std::string CapsParser::getVideoCaps(const std::string &profile)
{
    return getCapsFromFile(XSTRINGIFY(__DATA_DIR__) "/caps.txt", profile);
}


/// Audio profile is in format <codec>_<channels>_<samplerate>
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
        LOG_ERROR("Unable to open file " << filename);

    if (not foundCaps)
    {
        LOG_WARNING("Could not find caps for profile " << profile);
        return "";
    }
    
    return line;
}

