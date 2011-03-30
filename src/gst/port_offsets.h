
/* factories.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
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

#ifndef _PORTS_H_
#define _PORTS_H_

namespace ports {
    // If using audio and video, we need a range of [0-5]. If one of the two is
    // disabled, then we just need a range of [0-3]
    // RTP=n, RTCP1=n+1, RTCP2=n+5
    static const int RTCP_FIRST_OFFSET = 1;
    static const int RTCP_SECOND_OFFSET = 5;
    static const int MINIMUM_RANGE_FOR_ONE_STREAM = 3;
    static const int MINIMUM_RANGE_FOR_TWO_STREAMS = 5;
    static const int CAPS_OFFSET = 9;
}

#endif // _PORTS_H_

