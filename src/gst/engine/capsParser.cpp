#include "./capsParser.h"

#include "util.h"
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>

#include "caps.h"

/// Video profile is in format <codec>
std::string CapsParser::getVideoCaps(const std::string &codec, int width, int height)
{
    using boost::lexical_cast;

    const std::string profile = codec + "_" + 
        lexical_cast<std::string>(width) + "_" + 
        lexical_cast<std::string>(height);
    return caps::getCaps(profile);
}


/// Audio profile is in format <codec>_<channels>_<samplerate>
std::string CapsParser::getAudioCaps(const std::string &codec, int numChannels, int sampleRate)
{
    using boost::lexical_cast;

    const std::string profile = codec + "_" + 
        lexical_cast<std::string>(numChannels) + "_" + 
        lexical_cast<std::string>(sampleRate);
    return caps::getCaps(profile);
}

