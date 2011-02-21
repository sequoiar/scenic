/* videoFactory.h
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

#ifndef _VIDEO_FACTORY_H_
#define _VIDEO_FACTORY_H_

#include <tr1/memory>

namespace boost {
    namespace program_options {
        class variables_map;
    }
}

class Pipeline;
class VideoSender;
class VideoReceiver;
class LocalVideo;

namespace videofactory
{
    std::tr1::shared_ptr<VideoReceiver> buildVideoReceiver(Pipeline &pipeline, const boost::program_options::variables_map &options);
    std::tr1::shared_ptr<VideoSender> buildVideoSender(Pipeline &pipeline, const boost::program_options::variables_map &options);
    std::tr1::shared_ptr<LocalVideo> buildLocalVideo(Pipeline &pipeline, const boost::program_options::variables_map &options);
}

#endif // _VIDEO_FACTORY_H_

