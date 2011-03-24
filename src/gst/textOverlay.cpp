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


// callback to change the text position
gboolean textPositionCallback(gpointer data)
{
    GstElement *textoverlay = static_cast<GstElement*>(data);
    static int deltay = 0;
    static int direction = 5;
    deltay += direction;
    g_object_set(textoverlay, "deltay", deltay, NULL);
    if (abs(deltay) > 20)
        direction = -direction;
    return TRUE;
}

/** Constructor sets text */
TextOverlay::TextOverlay(const Pipeline &pipeline, const std::string &text)
{
    if (not text.empty())
    {
        textoverlay_ = pipeline.makeElement("textoverlay", NULL);
        g_object_set(textoverlay_, "text", text.c_str(), "font-desc", "sans 50",
                NULL);
        g_timeout_add(50 /* ms */, 
                static_cast<GSourceFunc>(textPositionCallback),
                textoverlay_);
    }
    else // just a passthrough element
    {
        textoverlay_ = pipeline.makeElement("identity", NULL);
        g_object_set(textoverlay_, "silent", TRUE, NULL);
    }
}
