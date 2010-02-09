#include "./capsParser.h"

#include "util.h"
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>

#include "caps.h"

int maxNumChannelsForCodec(const std::string &codec)
{
    int result;
    if (codec == "mp3")
        result = 2;
    else if (codec == "raw")
        result = 8;
    else if (codec == "vorbis")
        result = 24;
    else
        THROW_CRITICAL("Invalid codec " << codec);
    return result;
}

/// Video profile is in format <codec>_<width>_<height>_<pictureAspectRatio>
std::string CapsParser::getVideoCaps(const std::string &codec, int width, int height, const std::string &pictureAspectRatio)
{
    using boost::lexical_cast;

    const std::string profile = codec + "_" + 
        lexical_cast<std::string>(width) + "_" + 
        lexical_cast<std::string>(height) + "_" +
        pictureAspectRatio;
    return caps::getCaps(profile);
}


/// Audio profile is in format <codec>_<channels>_<samplerate>
std::string CapsParser::getAudioCaps(const std::string &codec, int numChannels, int sampleRate)
{
    using boost::lexical_cast;
    if (maxNumChannelsForCodec(codec) < numChannels or numChannels < 1)
        THROW_CRITICAL("Invalid channel number " << numChannels << " for codec " << codec); 

    const std::string profile = codec + "_" + 
        lexical_cast<std::string>(numChannels) + "_" + 
        lexical_cast<std::string>(sampleRate);
    return caps::getCaps(profile);
}

