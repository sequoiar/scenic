// mediaConfig.cpp
// Copyright 2008 Koya Charles & Tristan Matthews
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

/** \file
 *      Base class for media parameter objects.
 *
 */

#include "mediaConfig.h"
#include "logWriter.h"


// FIXME: not every mediaconfig has a location
bool MediaConfig::fileExists() const
{
    if (location_.empty())
    {
        LOG("No file location given", ERROR);
        return false;
    }
    FILE *file;
    file = fopen(location(), "r");
    if (file != NULL)
    {
        fclose(file);
        return true;
    }
    else
    {
        LOG("File does not exist", ERROR);
        return false;
    }
}


const char* MediaConfig::location() const
{
    if (!location_.empty())
        return location_.c_str();
    else {
        LOG("No location specified", ERROR);
        return NULL;
    }
}


