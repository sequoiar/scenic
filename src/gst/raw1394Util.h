
// raw1394util.h
// Based on raw1394util.h, part of dvgrab available at <http://www.kinodv.org/>
// Copyright 2008 Koya Charles & Tristan Matthews //
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

#ifndef _RAW1394UTIL_H_
#define _RAW1394UTIL_H_ 

#include <libraw1394/raw1394.h>

namespace Raw1394 
{

	int raw1394_get_num_ports();
	raw1394handle_t raw1394_open( int port );
	void raw1394_close( raw1394handle_t handle );
	int discoverAVC( int * port, octlet_t* guid );
    bool cameraIsReady();
}

#endif // RAW1394UTIL_H 
