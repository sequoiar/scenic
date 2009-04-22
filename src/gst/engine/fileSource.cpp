/* fileSource.cpp
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

this class manages a collection of filesrcs, which video and audiosources try to link to, not unlike dv1394src

if a location already exists, try to plug into that one.
else create new filesrc and plug into it */

#include "util.h"
#include "pipeline.h"
#include "gstLinkable.h"
#include "fileSource.h"

// class holds a static map of all the existing instances of FileSources 
std::map<std::string, FileSource*> FileSource::fileSources_;

FileSource::FileSource() : filesrc_(0), decodebin_(0), videoQueue_(0), audioQueue_(0)
{}

FileSource::~FileSource() 
{
    if (isLinked())
        LOG_WARNING("Deleting FileSource that is still linked");
    Pipeline::Instance()->remove(&decodebin_);
    Pipeline::Instance()->remove(&filesrc_);
}


void FileSource::init(const std::string & location)
{
    filesrc_ = Pipeline::Instance()->makeElement("filesrc", NULL); 
    g_object_set(G_OBJECT(filesrc_), "location", location.c_str(), NULL);   // set location
    decodebin_ = Pipeline::Instance()->makeElement("decodebin", NULL);
    // bind callback
    g_signal_connect(decodebin_, "new-decoded-pad",
        G_CALLBACK(FileSource::cb_new_src_pad),
        static_cast<void *>(this));

    // link them
    gstlinkable::link(filesrc_, decodebin_);
}

bool FileSource::instanceExists(const std::string &location)
{
    return fileSources_.find(location) != fileSources_.end();
}   

/// checks to see if an instance is already available, or creates a new one if needed.
// FIXME: maybe this should just return the appropriate queue instead of the class?
GstElement * FileSource::acquire(const std::string &location, MEDIA_TYPE mediaType)
{
    GstElement *queue = 0;

    if (not instanceExists(location))  // make new FileSource if needed
    {
        fileSources_[location] = new FileSource();
        fileSources_[location]->init(location);
    }
    
    if (fileSources_[location]->videoQueue_ == 0)
        fileSources_[location]->videoQueue_ = Pipeline::Instance()->makeElement("queue", NULL);

    if (fileSources_[location]->audioQueue_ == 0)
        fileSources_[location]->audioQueue_ = Pipeline::Instance()->makeElement("queue", NULL);

    switch (mediaType)
    {
        case VIDEO:
            {
                queue = fileSources_[location]->videoQueue_;
                break;
            }

        case AUDIO:
            {   
                queue = fileSources_[location]->audioQueue_;
                break;
            }
    }

    return queue;
}
 

// called by client
void FileSource::release(const std::string &location, MEDIA_TYPE mediaType)
{
    if (not instanceExists(location))
    {
        LOG_WARNING("Trying to call release on non existent FileSource object");
        return;
    }
    
    switch (mediaType) 
    {
        case VIDEO:
        fileSources_[location]->removeVideo();
        break;

        case AUDIO:
        fileSources_[location]->removeAudio();
        break;
    }

    if (not fileSources_[location]->isLinked()) // no more objects using the filesource
        fileSources_.erase(location);
}

void FileSource::removeVideo()
{
    Pipeline::Instance()->remove(&videoQueue_);
}


void FileSource::removeAudio()
{
    Pipeline::Instance()->remove(&audioQueue_);
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

    assert(sinkElement);
    sinkPad = gst_element_get_static_pad(sinkElement, "sink");

    LOG_DEBUG("linking new srcpad and sinkpad.");
    assert(gstlinkable::link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}

