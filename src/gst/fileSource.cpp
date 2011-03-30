/* fileSource.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *

this class manages a collection of filesrcs, which video and audiosources try to link to, not unlike dv1394src

if a location already exists, try to plug into that one.
else create new filesrc and plug into it */

#include <gst/gst.h>
#include <cassert>
#include "pipeline.h"
#include "gstLinkable.h"
#include "fileSource.h"

// class holds a static map of all the existing instances of FileSources 
std::map<std::string, FileSource*> FileSource::fileSources_;

FileSource::FileSource(const Pipeline &pipeline, const std::string & location) :
    pipeline_(pipeline),
    filesrc_(pipeline_.makeElement("filesrc", NULL)),
    decodebin_(pipeline_.makeElement("decodebin2", NULL)),
    videoQueue_(0), 
    audioQueue_(0)
{
    LOG_DEBUG("Init on filesource for location " << location);
    g_object_set(G_OBJECT(filesrc_), "location", location.c_str(), NULL);   // set location
    // bind callback
    g_signal_connect(decodebin_, "new-decoded-pad",
            G_CALLBACK(FileSource::cb_new_src_pad),
            static_cast<void *>(this));

    // link them
    gstlinkable::link(filesrc_, decodebin_);
}


FileSource::~FileSource() 
{
    if (isLinked())
        LOG_WARNING("Deleting FileSource that is still linked");
}


bool FileSource::instanceExists(const std::string &location)
{
    return fileSources_.find(location) != fileSources_.end();
}   

/// checks to see if an instance is already available, or creates a new one if needed.
GstElement * FileSource::acquireAudio(const Pipeline &pipeline, const std::string &location)
{
    if (not instanceExists(location))  // make new FileSource if needed
        fileSources_[location] = new FileSource(pipeline, location);

    if (fileSources_[location]->audioQueue_ == 0)
        fileSources_[location]->audioQueue_ = pipeline.makeElement("queue", NULL);

    return fileSources_[location]->audioQueue_;
}


/// checks to see if an instance is already available, or creates a new one if needed.
GstElement * FileSource::acquireVideo(const Pipeline &pipeline, const std::string &location)
{
    if (not instanceExists(location))  // make new FileSource if needed
        fileSources_[location] = new FileSource(pipeline, location);

    if (fileSources_[location]->videoQueue_ == 0)
        fileSources_[location]->videoQueue_ = pipeline.makeElement("queue", NULL);

    return fileSources_[location]->videoQueue_;
}


// called by client
void FileSource::releaseAudio(const std::string &location)
{
    if (not instanceExists(location))
    {
        LOG_WARNING("Trying to call release on non existent FileSource object");
        return;
    }

    if (not fileSources_[location]->isLinked()) // no more objects using the filesource
        fileSources_.erase(location);
}


// called by client
void FileSource::releaseVideo(const std::string &location)
{
    if (not instanceExists(location))
    {
        LOG_WARNING("Trying to call release on non existent FileSource object");
        return;
    }

    if (not fileSources_[location]->isLinked()) // no more objects using the filesource
        fileSources_.erase(location);
}

bool FileSource::isLinked()
{
    return videoQueue_ != 0 and audioQueue_ != 0;
}

void FileSource::cb_new_src_pad(GstElement *  /*srcElement*/, GstPad * srcPad, gboolean /*last*/, void * data)
{
    GstStructure *str;
    GstPad *sinkPad = 0;
    GstCaps *caps;
    // now we can link our queue to our new decodebin element
    FileSource *context = static_cast<FileSource*>(data); // data is the FileSource we want

    GstElement *sinkElement = NULL;

    /* check media type of new srcPad, make sure we link the right one */
    caps = gst_pad_get_caps(srcPad);
    str = gst_caps_get_structure(caps, 0);

    if (g_strrstr(gst_structure_get_name(str), "video"))
    {
        LOG_DEBUG("Got video pad");
        sinkElement = context->videoQueue_;
    }
    else if (g_strrstr(gst_structure_get_name(str), "audio"))
    {
        LOG_DEBUG("Got audio pad");
        sinkElement = context->audioQueue_;
    }
    else 
    {
        gst_caps_unref(caps);
        gst_object_unref(sinkPad);
        THROW_ERROR("Trying to link pad that is neither audio nor video");
    }
    gst_caps_unref(caps);

    if (sinkElement == 0)
    {
        LOG_WARNING("Not expecting this decoded stream, ignoring");
        return;
    }

    sinkPad = gst_element_get_static_pad(sinkElement, "sink");

    LOG_DEBUG("linking new srcpad and sinkpad.");
    bool linked = gstlinkable::link_pads(srcPad, sinkPad);
    assert(linked);
    gst_object_unref(sinkPad);
}

