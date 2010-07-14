/* textOverlay.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gst/gst.h>
#include "textOverlay.h"
#include "pipeline.h"

/** Constructor sets text */
TextOverlay::TextOverlay(const Pipeline &pipeline, const std::string &text) : 
    pipeline_(pipeline),
    textoverlay_(pipeline_.makeElement("textoverlay", NULL))
{
    g_object_set(textoverlay_, "text", text.c_str(), "font-desc", "normal 50",
            NULL);
}

/// Destructor 
TextOverlay::~TextOverlay()
{
    pipeline_.remove(&textoverlay_);
}

