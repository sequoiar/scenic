/* capsParser.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _CAPS_PARSER_H_
#define _CAPS_PARSER_H_

#include <string>

class CapsParser {
    private:
        static std::string getCapsFromFile(const char *filename, const std::string &codec);
    public:
        /// video caps will vary depending on resolution
        static std::string getVideoCaps(const std::string &codec, 
                int captureWidth, 
                int captureHeight, 
                const std::string &pictureAspectRatio);
        static std::string getAudioCaps(const std::string &codec, int numChannels, int sampleRate);
};

#endif // _CAPS_PARSER_H_
